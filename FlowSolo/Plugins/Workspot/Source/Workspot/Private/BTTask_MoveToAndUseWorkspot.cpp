// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_MoveToAndUseWorkspot.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "SmartObjectSubsystem.h"
#include "SmartObjectComponent.h"
#include "SmartObjectRequestTypes.h"
#include "SmartObjectTypes.h"
#include "GameplayBehaviorSmartObjectBehaviorDefinition.h"
#include "WorkspotHelpers.h"
#include "WorkspotSubsystem.h"
#include "WorkspotInstance.h"
#include "Workspot.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_MoveToAndUseWorkspot::UBTTask_MoveToAndUseWorkspot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Move To And Use Workspot";
	bNotifyTick = true;
	bNotifyTaskFinished = true;

	// Setup blackboard key filter
	SmartObjectActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MoveToAndUseWorkspot, SmartObjectActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_MoveToAndUseWorkspot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FTaskMemory* Memory = CastInstanceNodeMemory<FTaskMemory>(NodeMemory);
	Memory->ClaimHandle = FSmartObjectClaimHandle::InvalidHandle;
	Memory->WorkspotInstance.Reset();
	Memory->bMovementComplete = false;
	Memory->bWorkspotStarted = false;

	UE_LOG(LogWorkspot, Log, TEXT("BTTask_MoveToAndUseWorkspot - START"));

	// Step 1: Find and claim SmartObject
	if (!FindAndClaimSmartObject(OwnerComp, Memory))
	{
		UE_LOG(LogWorkspot, Error, TEXT("BTTask_MoveToAndUseWorkspot - Failed to find/claim SmartObject"));
		return EBTNodeResult::Failed;
	}

	// Step 2: Move to slot location
	if (!MoveToSlotLocation(OwnerComp, Memory))
	{
		UE_LOG(LogWorkspot, Error, TEXT("BTTask_MoveToAndUseWorkspot - Failed to move to slot"));
		ReleaseSlot(OwnerComp, Memory);
		return EBTNodeResult::Failed;
	}

	// Movement started, wait for completion in Tick
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_MoveToAndUseWorkspot::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FTaskMemory* Memory = CastInstanceNodeMemory<FTaskMemory>(NodeMemory);

	UE_LOG(LogWorkspot, Warning, TEXT("BTTask_MoveToAndUseWorkspot - ABORTED"));

	// Stop workspot if running
	if (Memory->WorkspotInstance.IsValid())
	{
		AAIController* AIController = OwnerComp.GetAIOwner();
		if (AIController && AIController->GetPawn())
		{
			UWorkspotHelpers::StopWorkspotOnActor(AIController->GetPawn(), true);
		}
	}

	// Release slot
	ReleaseSlot(OwnerComp, Memory);

	return EBTNodeResult::Aborted;
}

void UBTTask_MoveToAndUseWorkspot::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FTaskMemory* Memory = CastInstanceNodeMemory<FTaskMemory>(NodeMemory);
	AAIController* AIController = OwnerComp.GetAIOwner();

	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Wait for movement to complete
	if (!Memory->bMovementComplete)
	{
		EPathFollowingStatus::Type MoveStatus = AIController->GetMoveStatus();

		if (MoveStatus == EPathFollowingStatus::Moving)
		{
			// Still moving, wait
			return;
		}
		else if (MoveStatus == EPathFollowingStatus::Idle)
		{
			// Movement completed successfully
			Memory->bMovementComplete = true;
			UE_LOG(LogWorkspot, Log, TEXT("BTTask_MoveToAndUseWorkspot - Movement complete"));
		}
		else
		{
			// Movement failed
			UE_LOG(LogWorkspot, Error, TEXT("BTTask_MoveToAndUseWorkspot - Movement failed with status %d"), (int32)MoveStatus);
			ReleaseSlot(OwnerComp, Memory);
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}
	}

	// Start workspot if not started yet
	if (Memory->bMovementComplete && !Memory->bWorkspotStarted)
	{
		if (!StartWorkspotFromSlot(OwnerComp, Memory))
		{
			UE_LOG(LogWorkspot, Error, TEXT("BTTask_MoveToAndUseWorkspot - Failed to start workspot"));
			ReleaseSlot(OwnerComp, Memory);
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}

		Memory->bWorkspotStarted = true;
	}

	// Wait for workspot to complete
	if (Memory->bWorkspotStarted)
	{
		if (!Memory->WorkspotInstance.IsValid() || Memory->WorkspotInstance->IsFinished())
		{
			// Workspot completed
			UE_LOG(LogWorkspot, Log, TEXT("BTTask_MoveToAndUseWorkspot - Workspot completed"));

			if (bReleaseSlotOnComplete)
			{
				ReleaseSlot(OwnerComp, Memory);
			}

			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}
	}
}

bool UBTTask_MoveToAndUseWorkspot::FindAndClaimSmartObject(UBehaviorTreeComponent& OwnerComp, FTaskMemory* Memory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	if (!Pawn)
	{
		UE_LOG(LogWorkspot, Error, TEXT("No valid Pawn"));
		return false;
	}

	UWorld* World = OwnerComp.GetWorld();
	USmartObjectSubsystem* SmartObjectSubsystem = World->GetSubsystem<USmartObjectSubsystem>();
	if (!SmartObjectSubsystem)
	{
		UE_LOG(LogWorkspot, Error, TEXT("No SmartObjectSubsystem found"));
		return false;
	}

	// Try to get SmartObject from blackboard
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* SmartObjectActor = nullptr;

	if (Blackboard && SmartObjectActorKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		SmartObjectActor = Cast<AActor>(Blackboard->GetValueAsObject(SmartObjectActorKey.SelectedKeyName));
	}

	if (!SmartObjectActor)
	{
		UE_LOG(LogWorkspot, Error, TEXT("No SmartObject actor found in blackboard"));
		return false;
	}

	// Get SmartObjectComponent
	USmartObjectComponent* SOComponent = SmartObjectActor->FindComponentByClass<USmartObjectComponent>();
	if (!SOComponent)
	{
		UE_LOG(LogWorkspot, Error, TEXT("Actor '%s' has no SmartObjectComponent"), *SmartObjectActor->GetName());
		return false;
	}

	// Get SmartObject handle
	FSmartObjectHandle SmartObjectHandle = SOComponent->GetRegisteredHandle();
	if (!SmartObjectHandle.IsValid())
	{
		UE_LOG(LogWorkspot, Error, TEXT("SmartObject is not registered"));
		return false;
	}

	// Create request to find available slots
	FSmartObjectRequest Request;
	Request.QueryBox = FBox(Pawn->GetActorLocation(), Pawn->GetActorLocation()).ExpandBy(SearchRadius);
	Request.Filter.BehaviorDefinitionClasses = { UGameplayBehaviorSmartObjectBehaviorDefinition::StaticClass() };

	// Find available slots
	TArray<FSmartObjectRequestResult> Results;
	SmartObjectSubsystem->FindSmartObjects(Request, Results, FConstStructView()); // Add missing UserData parameter

	// Filter results to only include our target SmartObject
	FSmartObjectRequestResult* TargetResult = Results.FindByPredicate([SmartObjectHandle](const FSmartObjectRequestResult& Result)
	{
		return Result.SmartObjectHandle == SmartObjectHandle && Result.IsValid();
	});

	if (!TargetResult)
	{
		UE_LOG(LogWorkspot, Error, TEXT("No available slots found on SmartObject '%s'"), *SmartObjectActor->GetName());
		return false;
	}

	// Check if slot can be claimed
	if (!SmartObjectSubsystem->CanBeClaimed(TargetResult->SlotHandle))
	{
		UE_LOG(LogWorkspot, Error, TEXT("Slot cannot be claimed (already in use or disabled)"));
		return false;
	}

	// Claim the slot using MarkSlotAsClaimed
	// UserData can be used to associate user-specific data with the claim
	FSmartObjectClaimHandle ClaimHandle = SmartObjectSubsystem->MarkSlotAsClaimed(
		TargetResult->SlotHandle,
		ESmartObjectClaimPriority::Normal,
		FConstStructView() // No user data for now
	);

	if (!ClaimHandle.IsValid())
	{
		UE_LOG(LogWorkspot, Error, TEXT("Failed to claim SmartObject slot"));
		return false;
	}

	Memory->ClaimHandle = ClaimHandle;
	UE_LOG(LogWorkspot, Log, TEXT("✅ Claimed SmartObject slot"));

	return true;
}

bool UBTTask_MoveToAndUseWorkspot::MoveToSlotLocation(UBehaviorTreeComponent& OwnerComp, FTaskMemory* Memory)
{
	if (!Memory->ClaimHandle.IsValid())
	{
		return false;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return false;
	}

	UWorld* World = OwnerComp.GetWorld();
	USmartObjectSubsystem* SmartObjectSubsystem = World->GetSubsystem<USmartObjectSubsystem>();
	if (!SmartObjectSubsystem)
	{
		return false;
	}

	// Get slot transform
	TOptional<FTransform> SlotTransform = SmartObjectSubsystem->GetSlotTransform(Memory->ClaimHandle);
	if (!SlotTransform.IsSet())
	{
		UE_LOG(LogWorkspot, Error, TEXT("Failed to get slot transform"));
		return false;
	}

	FVector TargetLocation = SlotTransform.GetValue().GetLocation();

	// Start movement
	FAIMoveRequest MoveRequest(TargetLocation);
	MoveRequest.SetAcceptanceRadius(AcceptableRadius);
	MoveRequest.SetUsePathfinding(true);

	FPathFollowingRequestResult MoveResult = AIController->MoveTo(MoveRequest);

	if (MoveResult.Code == EPathFollowingRequestResult::RequestSuccessful)
	{
		UE_LOG(LogWorkspot, Log, TEXT("✅ Started movement to slot location"));
		return true;
	}
	else
	{
		UE_LOG(LogWorkspot, Error, TEXT("Failed to start movement: %d"), (int32)MoveResult.Code);
		return false;
	}
}

bool UBTTask_MoveToAndUseWorkspot::StartWorkspotFromSlot(UBehaviorTreeComponent& OwnerComp, FTaskMemory* Memory)
{
	if (!Memory->ClaimHandle.IsValid())
	{
		return false;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
	{
		return false;
	}

	APawn* Pawn = AIController->GetPawn();

	// Stop AI movement if requested
	if (bStopMovementOnUse)
	{
		AIController->StopMovement();
	}

	// Start workspot using helper function
	bool bSuccess = UWorkspotHelpers::StartWorkspotFromClaimHandle(Pawn, Memory->ClaimHandle);

	if (!bSuccess)
	{
		UE_LOG(LogWorkspot, Error, TEXT("Failed to start workspot from claim handle"));
		return false;
	}

	// Get workspot instance for tracking
	UWorkspotSubsystem* WorkspotSubsystem = Pawn->GetWorld()->GetSubsystem<UWorkspotSubsystem>();
	if (WorkspotSubsystem)
	{
		Memory->WorkspotInstance = WorkspotSubsystem->GetActiveWorkspot(Pawn);
	}

	UE_LOG(LogWorkspot, Log, TEXT("✅ Workspot started successfully"));
	return true;
}

void UBTTask_MoveToAndUseWorkspot::ReleaseSlot(UBehaviorTreeComponent& OwnerComp, FTaskMemory* Memory)
{
	if (!Memory->ClaimHandle.IsValid())
	{
		return;
	}

	UWorld* World = OwnerComp.GetWorld();
	USmartObjectSubsystem* SmartObjectSubsystem = World->GetSubsystem<USmartObjectSubsystem>();
	if (SmartObjectSubsystem)
	{
		// Use MarkSlotAsFree instead of deprecated Release()
		bool bReleased = SmartObjectSubsystem->MarkSlotAsFree(Memory->ClaimHandle);
		if (bReleased)
		{
			UE_LOG(LogWorkspot, Log, TEXT("✅ Released SmartObject slot"));
		}
		else
		{
			UE_LOG(LogWorkspot, Warning, TEXT("⚠️ Failed to release SmartObject slot (might already be free)"));
		}
	}

	Memory->ClaimHandle = FSmartObjectClaimHandle::InvalidHandle;
}
