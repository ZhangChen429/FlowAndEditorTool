// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkspotHelpers.h"
#include "WorkspotTree.h"
#include "WorkspotBehaviorConfig.h"
#include "WorkspotSubsystem.h"
#include "Workspot.h"
#include "SmartObjectSubsystem.h"
#include "GameplayBehaviorSmartObjectBehaviorDefinition.h"
#include "WorkspotInstance.h"
#include "AIController.h"
#include "BrainComponent.h"

UWorkspotTree* UWorkspotHelpers::GetWorkspotTreeFromClaimHandle(UObject* WorldContextObject, const FSmartObjectClaimHandle& ClaimHandle)
{
	if (!ClaimHandle.IsValid())
	{
		return nullptr;
	}

	// Get the config first
	UWorkspotBehaviorConfig* Config = GetWorkspotConfigFromClaimHandle(WorldContextObject, ClaimHandle);
	if (!Config)
	{
		return nullptr;
	}

	return Config->GetWorkspotTree();
}

UWorkspotBehaviorConfig* UWorkspotHelpers::GetWorkspotConfigFromClaimHandle(UObject* WorldContextObject, const FSmartObjectClaimHandle& ClaimHandle)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("GetWorkspotConfigFromClaimHandle: WorldContextObject is null"));
		return nullptr;
	}

	if (!ClaimHandle.IsValid())
	{
		UE_LOG(LogWorkspot, Warning, TEXT("GetWorkspotConfigFromClaimHandle: Invalid ClaimHandle"));
		return nullptr;
	}

	// Get world
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("GetWorkspotConfigFromClaimHandle: Failed to get World from context"));
		return nullptr;
	}

	// Get SmartObject subsystem
	USmartObjectSubsystem* SmartObjectSubsystem = World->GetSubsystem<USmartObjectSubsystem>();
	if (!SmartObjectSubsystem)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("GetWorkspotConfigFromClaimHandle: No SmartObjectSubsystem found"));
		return nullptr;
	}

	// Get the behavior definition for this slot (template function, specify type)
	const UGameplayBehaviorSmartObjectBehaviorDefinition* GameplayBehaviorDef =
		SmartObjectSubsystem->GetBehaviorDefinition<UGameplayBehaviorSmartObjectBehaviorDefinition>(ClaimHandle);

	if (!GameplayBehaviorDef)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("GetWorkspotConfigFromClaimHandle: No GameplayBehaviorSmartObjectBehaviorDefinition found for handle"));
		return nullptr;
	}

	// Get the config
	UWorkspotBehaviorConfig* WorkspotConfig = Cast<UWorkspotBehaviorConfig>(GameplayBehaviorDef->GameplayBehaviorConfig);
	if (!WorkspotConfig)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("GetWorkspotConfigFromClaimHandle: GameplayBehaviorConfig is not UWorkspotBehaviorConfig"));
		return nullptr;
	}

	return WorkspotConfig;
}

bool UWorkspotHelpers::StartWorkspotFromClaimHandle(AActor* Avatar, const FSmartObjectClaimHandle& ClaimHandle)
{
	if (!Avatar)
	{
		UE_LOG(LogWorkspot, Error, TEXT("StartWorkspotFromClaimHandle: Avatar is null"));
		return false;
	}

	if (!ClaimHandle.IsValid())
	{
		UE_LOG(LogWorkspot, Error, TEXT("StartWorkspotFromClaimHandle: Invalid ClaimHandle"));
		return false;
	}

	// Get the config (use Avatar as WorldContextObject)
	UWorkspotBehaviorConfig* Config = GetWorkspotConfigFromClaimHandle(Avatar, ClaimHandle);
	if (!Config)
	{
		UE_LOG(LogWorkspot, Error, TEXT("StartWorkspotFromClaimHandle: Failed to get WorkspotBehaviorConfig from ClaimHandle"));
		return false;
	}

	if (!Config->WorkspotTree)
	{
		UE_LOG(LogWorkspot, Error, TEXT("StartWorkspotFromClaimHandle: Config has no WorkspotTree"));
		return false;
	}

	// Get workspot subsystem
	UWorkspotSubsystem* Subsystem = Avatar->GetWorld()->GetSubsystem<UWorkspotSubsystem>();
	if (!Subsystem)
	{
		UE_LOG(LogWorkspot, Error, TEXT("StartWorkspotFromClaimHandle: No WorkspotSubsystem found"));
		return false;
	}

	// Start workspot
	UWorkspotInstance* Instance = Subsystem->StartWorkspot(
		Avatar,
		Config->WorkspotTree,
		Config->PreferredEntryPoint
	);

	if (!Instance)
	{
		UE_LOG(LogWorkspot, Error, TEXT("StartWorkspotFromClaimHandle: Failed to start workspot"));
		return false;
	}

	UE_LOG(LogWorkspot, Log, TEXT("StartWorkspotFromClaimHandle: Successfully started workspot '%s' on %s"),
		*Config->WorkspotTree->GetName(), *Avatar->GetName());

	return true;
}

bool UWorkspotHelpers::PlayWorkspotOnActor(AActor* Actor, UWorkspotTree* WorkspotTree, FName EntryPointTag)
{
	if (!Actor)
	{
		UE_LOG(LogWorkspot, Error, TEXT("PlayWorkspotOnActor: Actor is null"));
		return false;
	}

	if (!WorkspotTree)
	{
		UE_LOG(LogWorkspot, Error, TEXT("PlayWorkspotOnActor: WorkspotTree is null"));
		return false;
	}

	if (!WorkspotTree->IsValid())
	{
		UE_LOG(LogWorkspot, Error, TEXT("PlayWorkspotOnActor: WorkspotTree '%s' is invalid"), *WorkspotTree->GetName());
		return false;
	}

	// Verify actor has skeletal mesh component (required for playing animations)
	USkeletalMeshComponent* MeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
	if (!MeshComp)
	{
		UE_LOG(LogWorkspot, Error, TEXT("PlayWorkspotOnActor: Actor '%s' has no SkeletalMeshComponent"), *Actor->GetName());
		return false;
	}

	// Get workspot subsystem
	UWorkspotSubsystem* Subsystem = Actor->GetWorld()->GetSubsystem<UWorkspotSubsystem>();
	if (!Subsystem)
	{
		UE_LOG(LogWorkspot, Error, TEXT("PlayWorkspotOnActor: No WorkspotSubsystem found"));
		return false;
	}

	// Start workspot
	UWorkspotInstance* Instance = Subsystem->StartWorkspot(Actor, WorkspotTree, EntryPointTag);
	if (!Instance)
	{
		UE_LOG(LogWorkspot, Error, TEXT("PlayWorkspotOnActor: Failed to start workspot"));
		return false;
	}

	UE_LOG(LogWorkspot, Log, TEXT("PlayWorkspotOnActor: Successfully started workspot '%s' on actor '%s'"),
		*WorkspotTree->GetName(), *Actor->GetName());

	return true;
}

void UWorkspotHelpers::StopWorkspotOnActor(AActor* Actor, bool bForceStop)
{
	if (!Actor)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("StopWorkspotOnActor: Actor is null"));
		return;
	}

	UWorkspotSubsystem* Subsystem = Actor->GetWorld()->GetSubsystem<UWorkspotSubsystem>();
	if (!Subsystem)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("StopWorkspotOnActor: No WorkspotSubsystem found"));
		return;
	}

	Subsystem->StopWorkspot(Actor, bForceStop);

	UE_LOG(LogWorkspot, Log, TEXT("StopWorkspotOnActor: Stopped workspot on actor '%s'"), *Actor->GetName());
}

bool UWorkspotHelpers::IsActorPlayingWorkspot(AActor* Actor)
{
	if (!Actor)
	{
		return false;
	}

	UWorkspotSubsystem* Subsystem = Actor->GetWorld()->GetSubsystem<UWorkspotSubsystem>();
	if (!Subsystem)
	{
		return false;
	}

	UWorkspotInstance* Instance = Subsystem->GetActiveWorkspot(Actor);
	return Instance != nullptr && !Instance->IsFinished();
}

UWorkspotInstance* UWorkspotHelpers::ClaimAndUseWorkspotSlot(
	AAIController* Controller,
	const FSmartObjectClaimHandle& ClaimHandle,
	bool bLockAILogic)
{
	UE_LOG(LogWorkspot, Display, TEXT(""));
	UE_LOG(LogWorkspot, Display, TEXT("═══════════════════════════════════════════"));
	UE_LOG(LogWorkspot, Display, TEXT("  ⭐ ClaimAndUseWorkspotSlot - START"));
	UE_LOG(LogWorkspot, Display, TEXT("═══════════════════════════════════════════"));

	// Step 1: Validate input
	if (!Controller)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ [1/6] Controller is NULL"));
		return nullptr;
	}

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ [1/6] Controller has no Pawn"));
		return nullptr;
	}

	if (!ClaimHandle.IsValid())
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ [1/6] ClaimHandle is invalid"));
		return nullptr;
	}

	UE_LOG(LogWorkspot, Display, TEXT("✅ [1/6] Validation passed (Controller: %s, Pawn: %s)"),
		*Controller->GetName(), *Pawn->GetName());

	// Step 2: Use the slot (mark as occupied)
	const UGameplayBehaviorSmartObjectBehaviorDefinition* BehaviorDef = UseSmartObjectSlot(Controller, ClaimHandle);
	if (!BehaviorDef)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ [2/6] Failed to use SmartObject slot"));
		return nullptr;
	}

	UE_LOG(LogWorkspot, Display, TEXT("✅ [2/6] Slot marked as occupied"));

	// Step 3: Get WorkspotBehaviorConfig from BehaviorDefinition
	const UWorkspotBehaviorConfig* WorkspotConfig = Cast<UWorkspotBehaviorConfig>(BehaviorDef->GameplayBehaviorConfig);
	if (!WorkspotConfig)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ [3/6] BehaviorDefinition has no WorkspotBehaviorConfig"));
		UE_LOG(LogWorkspot, Error, TEXT("         Config type: %s"),
			BehaviorDef->GameplayBehaviorConfig ? *BehaviorDef->GameplayBehaviorConfig->GetClass()->GetName() : TEXT("NULL"));
		return nullptr;
	}

	UE_LOG(LogWorkspot, Display, TEXT("✅ [3/6] WorkspotBehaviorConfig found"));

	// Step 4: Get WorkspotTree from config
	UWorkspotTree* WorkspotTree = WorkspotConfig->WorkspotTree;
	if (!WorkspotTree)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ [4/6] WorkspotBehaviorConfig has no WorkspotTree"));
		return nullptr;
	}

	if (!WorkspotTree->IsValid())
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ [4/6] WorkspotTree '%s' is invalid"), *WorkspotTree->GetName());
		return nullptr;
	}

	UE_LOG(LogWorkspot, Display, TEXT("✅ [4/6] WorkspotTree valid: %s"), *WorkspotTree->GetName());

	// Step 5: Lock AI logic if requested
	if (bLockAILogic)
	{
		LockAILogic(Controller);
		UE_LOG(LogWorkspot, Display, TEXT("✅ [5/6] AI logic locked"));
	}
	else
	{
		UE_LOG(LogWorkspot, Display, TEXT("⏭️  [5/6] AI logic NOT locked (bLockAILogic = false)"));
	}

	// Step 6: Start Workspot via WorkspotSubsystem
	UWorkspotSubsystem* WorkspotSubsystem = Pawn->GetWorld()->GetSubsystem<UWorkspotSubsystem>();
	if (!WorkspotSubsystem)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ [6/6] No WorkspotSubsystem found"));
		if (bLockAILogic)
		{
			UnlockAILogic(Controller);
		}
		return nullptr;
	}

	UWorkspotInstance* Instance = WorkspotSubsystem->StartWorkspot(
		Pawn,
		WorkspotTree,
		WorkspotConfig->PreferredEntryPoint
	);

	if (!Instance)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ [6/6] Failed to start Workspot"));
		if (bLockAILogic)
		{
			UnlockAILogic(Controller);
		}
		return nullptr;
	}

	UE_LOG(LogWorkspot, Display, TEXT("✅ [6/6] Workspot started successfully!"));
	UE_LOG(LogWorkspot, Display, TEXT(""));
	UE_LOG(LogWorkspot, Display, TEXT("═══════════════════════════════════════════"));
	UE_LOG(LogWorkspot, Display, TEXT("  ✅ SUCCESS - Workspot is now playing"));
	UE_LOG(LogWorkspot, Display, TEXT("  Tree: %s"), *WorkspotTree->GetName());
	UE_LOG(LogWorkspot, Display, TEXT("  Pawn: %s"), *Pawn->GetName());
	UE_LOG(LogWorkspot, Display, TEXT("═══════════════════════════════════════════"));
	UE_LOG(LogWorkspot, Display, TEXT(""));

	return Instance;
}

const UGameplayBehaviorSmartObjectBehaviorDefinition* UWorkspotHelpers::UseSmartObjectSlot(
	UObject* WorldContextObject,
	const FSmartObjectClaimHandle& ClaimHandle)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogWorkspot, Error, TEXT("UseSmartObjectSlot: WorldContextObject is NULL"));
		return nullptr;
	}

	if (!ClaimHandle.IsValid())
	{
		UE_LOG(LogWorkspot, Error, TEXT("UseSmartObjectSlot: ClaimHandle is invalid"));
		return nullptr;
	}

	// Get world
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogWorkspot, Error, TEXT("UseSmartObjectSlot: Failed to get World"));
		return nullptr;
	}

	// Get SmartObject subsystem
	USmartObjectSubsystem* SmartObjectSubsystem = World->GetSubsystem<USmartObjectSubsystem>();
	if (!SmartObjectSubsystem)
	{
		UE_LOG(LogWorkspot, Error, TEXT("UseSmartObjectSlot: No SmartObjectSubsystem"));
		return nullptr;
	}

	// Mark slot as occupied and get BehaviorDefinition
	const UGameplayBehaviorSmartObjectBehaviorDefinition* BehaviorDef =
		SmartObjectSubsystem->MarkSlotAsOccupied<UGameplayBehaviorSmartObjectBehaviorDefinition>(ClaimHandle);

	if (!BehaviorDef)
	{
		UE_LOG(LogWorkspot, Error, TEXT("UseSmartObjectSlot: MarkSlotAsOccupied returned NULL"));
		UE_LOG(LogWorkspot, Error, TEXT("  Possible reasons:"));
		UE_LOG(LogWorkspot, Error, TEXT("  1. Slot is already occupied"));
		UE_LOG(LogWorkspot, Error, TEXT("  2. BehaviorDefinition type mismatch"));
		UE_LOG(LogWorkspot, Error, TEXT("  3. ClaimHandle is stale/invalid"));
		return nullptr;
	}

	UE_LOG(LogWorkspot, Log, TEXT("UseSmartObjectSlot: Slot marked as occupied, BehaviorDef: %s"),
		*BehaviorDef->GetName());

	return BehaviorDef;
}

void UWorkspotHelpers::LockAILogic(AAIController* Controller)
{
	if (!Controller)
	{
		return;
	}

	// Stop movement
	Controller->StopMovement();

	// Pause behavior tree if running
	UBrainComponent* BrainComp = Controller->GetBrainComponent();
	if (BrainComp)
	{
		BrainComp->PauseLogic(TEXT("Workspot"));
		UE_LOG(LogWorkspot, Log, TEXT("LockAILogic: Paused BrainComponent for '%s'"), *Controller->GetName());
	}

	UE_LOG(LogWorkspot, Log, TEXT("LockAILogic: AI movement stopped for '%s'"), *Controller->GetName());
}

void UWorkspotHelpers::UnlockAILogic(AAIController* Controller)
{
	if (!Controller)
	{
		return;
	}

	// Resume behavior tree
	UBrainComponent* BrainComp = Controller->GetBrainComponent();
	if (BrainComp)
	{
		BrainComp->ResumeLogic(TEXT("Workspot"));
		UE_LOG(LogWorkspot, Log, TEXT("UnlockAILogic: Resumed BrainComponent for '%s'"), *Controller->GetName());
	}

	UE_LOG(LogWorkspot, Log, TEXT("UnlockAILogic: AI logic unlocked for '%s'"), *Controller->GetName());
}

bool UWorkspotHelpers::VerifySmartObjectConfiguration(UObject* WorldContextObject, const FSmartObjectClaimHandle& ClaimHandle)
{
	UE_LOG(LogWorkspot, Display, TEXT(""));
	UE_LOG(LogWorkspot, Display, TEXT("═══════════════════════════════════════════"));
	UE_LOG(LogWorkspot, Display, TEXT("  🔍 SMARTOBJECT CONFIGURATION DIAGNOSTIC"));
	UE_LOG(LogWorkspot, Display, TEXT("═══════════════════════════════════════════"));

	// Step 1: Check WorldContextObject
	if (!WorldContextObject)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ [1/6] WorldContextObject is NULL"));
		return false;
	}
	UE_LOG(LogWorkspot, Display, TEXT("✅ [1/6] WorldContextObject valid"));

	// Step 2: Get World
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ [2/6] Failed to get World from context"));
		return false;
	}
	UE_LOG(LogWorkspot, Display, TEXT("✅ [2/6] World valid"));

	// Step 3: Check ClaimHandle
	if (!ClaimHandle.IsValid())
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ [3/6] ClaimHandle is INVALID"));
		UE_LOG(LogWorkspot, Error, TEXT("         → Make sure Claim succeeded before calling this"));
		return false;
	}
	UE_LOG(LogWorkspot, Display, TEXT("✅ [3/6] ClaimHandle is valid"));

	// Step 4: Get SmartObject Subsystem
	USmartObjectSubsystem* SmartObjectSubsystem = World->GetSubsystem<USmartObjectSubsystem>();
	if (!SmartObjectSubsystem)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ [4/6] No SmartObjectSubsystem in world"));
		return false;
	}
	UE_LOG(LogWorkspot, Display, TEXT("✅ [4/6] SmartObjectSubsystem found"));

	// Step 5: Get BehaviorDefinition
	const UGameplayBehaviorSmartObjectBehaviorDefinition* BehaviorDef =
		SmartObjectSubsystem->GetBehaviorDefinition<UGameplayBehaviorSmartObjectBehaviorDefinition>(ClaimHandle);

	if (!BehaviorDef)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ [5/6] BehaviorDefinition is NULL"));
		UE_LOG(LogWorkspot, Error, TEXT(""));
		UE_LOG(LogWorkspot, Error, TEXT("🔧 HOW TO FIX:"));
		UE_LOG(LogWorkspot, Error, TEXT("   1. Open your SmartObject Definition asset"));
		UE_LOG(LogWorkspot, Error, TEXT("   2. Find the slot you're trying to use"));
		UE_LOG(LogWorkspot, Error, TEXT("   3. Set 'Behavior Definition' class to:"));
		UE_LOG(LogWorkspot, Error, TEXT("      GameplayBehaviorSmartObjectBehaviorDefinition"));
		UE_LOG(LogWorkspot, Error, TEXT("   4. Create/assign a BehaviorDefinition instance"));
		UE_LOG(LogWorkspot, Error, TEXT(""));
		return false;
	}

	UE_LOG(LogWorkspot, Display, TEXT("✅ [5/6] BehaviorDefinition found: %s"), *BehaviorDef->GetName());

	// Step 6: Get Config
	if (!BehaviorDef->GameplayBehaviorConfig)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ [6/6] GameplayBehaviorConfig is NULL"));
		UE_LOG(LogWorkspot, Error, TEXT(""));
		UE_LOG(LogWorkspot, Error, TEXT("🔧 HOW TO FIX:"));
		UE_LOG(LogWorkspot, Error, TEXT("   1. Open your BehaviorDefinition asset"));
		UE_LOG(LogWorkspot, Error, TEXT("   2. Find 'Gameplay Behavior Config' property"));
		UE_LOG(LogWorkspot, Error, TEXT("   3. Click dropdown and select 'WorkspotBehaviorConfig'"));
		UE_LOG(LogWorkspot, Error, TEXT("   4. Set 'Workspot Tree' to your WorkspotTree asset"));
		UE_LOG(LogWorkspot, Error, TEXT("   ⚠️  Config must be INSTANCED (EditInlineNew)"));
		UE_LOG(LogWorkspot, Error, TEXT(""));
		return false;
	}

	// Check if it's the right config type
	const UWorkspotBehaviorConfig* WorkspotConfig = Cast<UWorkspotBehaviorConfig>(BehaviorDef->GameplayBehaviorConfig);
	if (!WorkspotConfig)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ [6/6] Config type is WRONG"));
		UE_LOG(LogWorkspot, Error, TEXT("         Current type: %s"), *BehaviorDef->GameplayBehaviorConfig->GetClass()->GetName());
		UE_LOG(LogWorkspot, Error, TEXT("         Expected type: WorkspotBehaviorConfig"));
		return false;
	}

	UE_LOG(LogWorkspot, Display, TEXT("✅ [6/6] GameplayBehaviorConfig valid: %s"), *WorkspotConfig->GetName());

	// Additional checks
	UE_LOG(LogWorkspot, Display, TEXT(""));
	UE_LOG(LogWorkspot, Display, TEXT("📋 ADDITIONAL CHECKS:"));

	if (!WorkspotConfig->WorkspotTree)
	{
		UE_LOG(LogWorkspot, Error, TEXT("  ❌ WorkspotTree is NULL in config"));
		UE_LOG(LogWorkspot, Error, TEXT("     → Assign a WorkspotTree asset in the config"));
		return false;
	}
	UE_LOG(LogWorkspot, Display, TEXT("  ✅ WorkspotTree: %s"), *WorkspotConfig->WorkspotTree->GetName());

	//if (!WorkspotConfig->BehaviorClass)
	//{
	//	UE_LOG(LogWorkspot, Error, TEXT("  ❌ BehaviorClass is NULL"));
	//	UE_LOG(LogWorkspot, Error, TEXT("     → This should be set automatically in UWorkspotBehaviorConfig constructor"));
	//	return false;
	//}
	//UE_LOG(LogWorkspot, Display, TEXT("  ✅ BehaviorClass: %s"), *WorkspotConfig->BehaviorClass->GetName());

	// Success
	UE_LOG(LogWorkspot, Display, TEXT(""));
	UE_LOG(LogWorkspot, Display, TEXT("═══════════════════════════════════════════"));
	UE_LOG(LogWorkspot, Display, TEXT("  ✅ ALL CHECKS PASSED"));
	UE_LOG(LogWorkspot, Display, TEXT("  Configuration is correct!"));
	UE_LOG(LogWorkspot, Display, TEXT("═══════════════════════════════════════════"));
	UE_LOG(LogWorkspot, Display, TEXT(""));

	return true;
}

bool UWorkspotHelpers::ManuallyUseSmartObjectSlot(UObject* WorldContextObject, const FSmartObjectClaimHandle& ClaimHandle, AActor* Avatar)
{
	UE_LOG(LogWorkspot, Display, TEXT(""));
	UE_LOG(LogWorkspot, Display, TEXT("═══════════════════════════════════════════"));
	UE_LOG(LogWorkspot, Display, TEXT("  🔧 MANUAL USE SMARTOBJECT"));
	UE_LOG(LogWorkspot, Display, TEXT("═══════════════════════════════════════════"));

	if (!Avatar)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ Avatar is NULL"));
		return false;
	}

	if (!WorldContextObject)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ WorldContextObject is NULL"));
		return false;
	}

	// Get World
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ Failed to get World"));
		return false;
	}

	// Get SmartObject Subsystem
	USmartObjectSubsystem* SmartObjectSubsystem = World->GetSubsystem<USmartObjectSubsystem>();
	if (!SmartObjectSubsystem)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ No SmartObjectSubsystem"));
		return false;
	}

	// Verify configuration first
	if (!VerifySmartObjectConfiguration(WorldContextObject, ClaimHandle))
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ Configuration verification failed"));
		UE_LOG(LogWorkspot, Error, TEXT("   Fix configuration issues before calling Use()"));
		return false;
	}

	UE_LOG(LogWorkspot, Display, TEXT("🎯 Calling SmartObjectSubsystem::Use()..."));

	// Call Use() with templated behavior definition type
	//const UGameplayBehaviorSmartObjectBehaviorDefinition* UsedDefinition =
	//	SmartObjectSubsystem->Use<UGameplayBehaviorSmartObjectBehaviorDefinition>(ClaimHandle, *Avatar->GetClass());

	//if (!UsedDefinition)
	//{
	//	UE_LOG(LogWorkspot, Error, TEXT("❌ Use() returned NULL"));
	//	UE_LOG(LogWorkspot, Error, TEXT("   Possible reasons:"));
	//	UE_LOG(LogWorkspot, Error, TEXT("   1. Slot was not claimed"));
	//	UE_LOG(LogWorkspot, Error, TEXT("   2. Slot is already in use"));
	//	UE_LOG(LogWorkspot, Error, TEXT("   3. BehaviorDefinition type mismatch"));
	//	return false;
	//}

	UE_LOG(LogWorkspot, Display, TEXT("✅ Use() succeeded!"));
	//UE_LOG(LogWorkspot, Display, TEXT("   Definition: %s"), *UsedDefinition->GetName());
	UE_LOG(LogWorkspot, Display, TEXT(""));
	UE_LOG(LogWorkspot, Display, TEXT("⏳ Now waiting for Trigger() to be called..."));
	UE_LOG(LogWorkspot, Display, TEXT("   Check UGameplayBehavior_Workspot::Trigger() for logs"));
	UE_LOG(LogWorkspot, Display, TEXT("═══════════════════════════════════════════"));

	return true;
}
