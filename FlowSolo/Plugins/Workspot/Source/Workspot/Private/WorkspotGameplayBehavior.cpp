// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkspotGameplayBehavior.h"
#include "WorkspotBehaviorConfig.h"
#include "WorkspotSubsystem.h"
#include "WorkspotInstance.h"
#include "WorkspotTree.h"
#include "Workspot.h"
#include "GameFramework/Actor.h"
#include "AIController.h"

UGameplayBehavior_Workspot::UGameplayBehavior_Workspot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StartTime = 0.0f;
	bPendingForceStop = false;
}

bool UGameplayBehavior_Workspot::Trigger(AActor& Avatar, const UGameplayBehaviorConfig* Config, AActor* SmartObjectOwner)
{
	// Cache avatar for callbacks
	CachedAvatar = &Avatar;

	// Get the workspot config
	const UWorkspotBehaviorConfig* WorkspotConfig = Cast<UWorkspotBehaviorConfig>(Config);
	if (!WorkspotConfig)
	{
		UE_LOG(LogWorkspot, Error, TEXT("UGameplayBehavior_Workspot::Trigger - Config is not a UWorkspotBehaviorConfig"));
		return false;
	}

	CachedConfig = WorkspotConfig;

	// Start the workspot
	if (!StartWorkspot(&Avatar, WorkspotConfig))
	{
		UE_LOG(LogWorkspot, Error, TEXT("UGameplayBehavior_Workspot::Trigger - Failed to start workspot on %s"), *Avatar.GetName());
		return false;
	}

	StartTime = GetWorld()->GetTimeSeconds();

	UE_LOG(LogWorkspot, Log, TEXT("UGameplayBehavior_Workspot::Trigger - Started workspot on %s"), *Avatar.GetName());

	return Super::Trigger(Avatar, Config, SmartObjectOwner);
}

void UGameplayBehavior_Workspot::EndBehavior(AActor& Avatar, const bool bInterrupted)
{
	UE_LOG(LogWorkspot, Log, TEXT("UGameplayBehavior_Workspot::EndBehavior - %s (Interrupted: %s)"),
		*Avatar.GetName(), bInterrupted ? TEXT("Yes") : TEXT("No"));

	// Stop the workspot
	StopWorkspot(&Avatar, bInterrupted);

	// Cleanup
	ActiveWorkspotInstance.Reset();
	CachedAvatar.Reset();
	CachedConfig = nullptr;

	Super::EndBehavior(Avatar, bInterrupted);
}

bool UGameplayBehavior_Workspot::StartWorkspot(AActor* Avatar, const UWorkspotBehaviorConfig* Config)
{
	if (!Avatar || !Config)
	{
		return false;
	}

	if (!Config->WorkspotTree)
	{
		UE_LOG(LogWorkspot, Error, TEXT("StartWorkspot: UWorkspotBehaviorConfig has no WorkspotTree assigned"));
		return false;
	}

	// Get workspot subsystem
	UWorkspotSubsystem* Subsystem = Avatar->GetWorld()->GetSubsystem<UWorkspotSubsystem>();
	if (!Subsystem)
	{
		UE_LOG(LogWorkspot, Error, TEXT("StartWorkspot: No WorkspotSubsystem found"));
		return false;
	}

	// Disable AI movement if requested
	if (Config->bTakeCharacterControl)
	{
		if (AAIController* AIController = Cast<AAIController>(Avatar->GetInstigatorController()))
		{
			AIController->StopMovement();
		}
	}

	// Start workspot through subsystem
	UWorkspotInstance* Instance = Subsystem->StartWorkspot(
		Avatar,
		Config->WorkspotTree,
		Config->PreferredEntryPoint
	);

	if (!Instance)
	{
		UE_LOG(LogWorkspot, Error, TEXT("StartWorkspot: Failed to start WorkspotTree on %s"), *Avatar->GetName());
		return false;
	}

	// Store instance reference
	ActiveWorkspotInstance = Instance;

	// Bind completion callback
	Instance->OnCompleted.AddUObject(this, &UGameplayBehavior_Workspot::OnWorkspotCompleted);

	UE_LOG(LogWorkspot, Log, TEXT("StartWorkspot: Successfully started WorkspotTree '%s' on %s"),
		*Config->WorkspotTree->GetName(), *Avatar->GetName());

	return true;
}

void UGameplayBehavior_Workspot::StopWorkspot(AActor* Avatar, bool bForceStop)
{
	if (!Avatar)
	{
		return;
	}

	// Get subsystem
	UWorkspotSubsystem* Subsystem = Avatar->GetWorld()->GetSubsystem<UWorkspotSubsystem>();
	if (Subsystem)
	{
		Subsystem->StopWorkspot(Avatar, bForceStop);
	}

	// Re-enable AI movement if we disabled it
	if (CachedConfig && CachedConfig->bTakeCharacterControl)
	{
		if (AAIController* AIController = Cast<AAIController>(Avatar->GetInstigatorController()))
		{
			// Resume AI movement if needed
			// The AI system will handle this automatically in most cases
		}
	}

	ActiveWorkspotInstance.Reset();
}

bool UGameplayBehavior_Workspot::IsWorkspotFinished(AActor* Avatar) const
{
	if (!Avatar)
	{
		return true;
	}

	UWorkspotSubsystem* Subsystem = Avatar->GetWorld()->GetSubsystem<UWorkspotSubsystem>();
	if (!Subsystem)
	{
		return true;
	}

	UWorkspotInstance* Instance = Subsystem->GetActiveWorkspot(Avatar);
	return !Instance || Instance->IsFinished();
}

void UGameplayBehavior_Workspot::OnWorkspotCompleted(UWorkspotInstance* Instance)
{
	// Workspot completed, end behavior
	if (AActor* Avatar = CachedAvatar.Get())
	{
		UE_LOG(LogWorkspot, Log, TEXT("OnWorkspotCompleted: Workspot finished on %s"), *Avatar->GetName());
		EndBehavior(*Avatar, false);
	}
}
