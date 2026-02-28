// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayBehavior.h"
#include "GameplayBehaviorConfig.h" 
#include "WorkspotGameplayBehavior.generated.h"

class UWorkspotTree;
class UWorkspotInstance;
class UWorkspotBehaviorConfig;

/**
 * GameplayBehavior that executes a WorkspotTree
 *
 * This is the actual behavior that runs when AI occupies a SmartObject slot with UWorkspotBehaviorConfig.
 *
 * Integration with SmartObject (Standard UE5 Pattern):
 * 1. Create SmartObject Definition with UGameplayBehaviorSmartObjectBehaviorDefinition
 * 2. In that definition, set GameplayBehaviorConfig to an instanced UWorkspotBehaviorConfig
 * 3. Configure WorkspotTree and settings in the UWorkspotBehaviorConfig instance
 * 4. When AI claims slot, UAITask_UseGameplayBehaviorSmartObject triggers this behavior
 * 5. Trigger() is called with Avatar (AI) and Config (UWorkspotBehaviorConfig)
 * 6. We extract WorkspotTree from config and start it via WorkspotSubsystem
 * 7. Monitor workspot state until completion
 * 8. EndBehavior() cleans up and releases slot
 *
 * Note: This behavior is automatically instantiated by UE's GameplayBehavior system.
 * You don't need to manually create instances of this class.
 */
UCLASS()
class WORKSPOT_API UGameplayBehavior_Workspot : public UGameplayBehavior
{
	GENERATED_BODY()

public:
	UGameplayBehavior_Workspot(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	// UGameplayBehavior interface
	virtual bool Trigger(AActor& Avatar, const UGameplayBehaviorConfig* Config = nullptr, AActor* SmartObjectOwner = nullptr) override;
	virtual void EndBehavior(AActor& Avatar, const bool bInterrupted) override;

	// End of UGameplayBehavior interface

private:
	/** Start the workspot on the avatar */
	bool StartWorkspot(AActor* Avatar, const UWorkspotBehaviorConfig* Config);

	/** Stop the workspot */
	void StopWorkspot(AActor* Avatar, bool bForceStop);

	/** Check if workspot has finished */
	bool IsWorkspotFinished(AActor* Avatar) const;

	/** Callback when workspot instance completes */
	void OnWorkspotCompleted(UWorkspotInstance* Instance);

private:
	/** The avatar actor (cached for callbacks) */
	TWeakObjectPtr<AActor> CachedAvatar;

	/** The active workspot instance */
	UPROPERTY()
	TWeakObjectPtr<UWorkspotInstance> ActiveWorkspotInstance;

	/** The config used to start this behavior */
	UPROPERTY()
	TObjectPtr<const UWorkspotBehaviorConfig> CachedConfig;

	/** Time when behavior started */
	float StartTime;

	/** Whether we should force-stop on next tick */
	bool bPendingForceStop;
};
