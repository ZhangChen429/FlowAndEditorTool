// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayBehaviorConfig.h"
#include "WorkspotBehaviorConfig.generated.h"

class UWorkspotTree;
class UGameplayBehavior;

/**
 * Configuration for Workspot GameplayBehavior
 *
 * This is a UGameplayBehaviorConfig that contains all settings needed to execute a WorkspotTree.
 * It's designed to work with UE's standard GameplayBehavior + SmartObject integration.
 *
 * Usage in SmartObject:
 * 1. Create a SmartObject Definition
 * 2. Add a Slot with UGameplayBehaviorSmartObjectBehaviorDefinition
 * 3. In that definition, set GameplayBehaviorConfig to an instanced UWorkspotBehaviorConfig
 * 4. Configure WorkspotTree and other settings in the instanced config
 * 5. When AI claims the slot, UE will automatically instantiate UGameplayBehavior_Workspot
 *
 * This approach follows UE's standard GameplayBehavior pattern and works correctly with
 * UAITask_UseGameplayBehaviorSmartObject.
 */
UCLASS(DisplayName = "Workspot Config", EditInlineNew)
class WORKSPOT_API UWorkspotBehaviorConfig : public UGameplayBehaviorConfig
{
	GENERATED_BODY()

public:
	UWorkspotBehaviorConfig();

	//////////////////////////////////////////////////////////////////////////
	// Core Configuration
	//////////////////////////////////////////////////////////////////////////

	/** The workspot tree to execute when this behavior is triggered */
	UPROPERTY(EditAnywhere, Category = "Workspot")
	TObjectPtr<UWorkspotTree> WorkspotTree;

	//////////////////////////////////////////////////////////////////////////
	// Entry Point Selection
	//////////////////////////////////////////////////////////////////////////

	/** Preferred entry point name (empty = auto-select closest) */
	UPROPERTY(EditAnywhere, Category = "Entry", meta = (EditCondition = "!bAutoSelectClosestEntry"))
	FName PreferredEntryPoint = NAME_None;

	/** Whether to automatically select the closest entry point */
	UPROPERTY(EditAnywhere, Category = "Entry")
	bool bAutoSelectClosestEntry = true;

	//////////////////////////////////////////////////////////////////////////
	// Occupy Phase Configuration
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Occupy duration in seconds (0 = infinite loop)
	 * If > 0, workspot will automatically exit after this duration
	 */
	UPROPERTY(EditAnywhere, Category = "Occupy", meta = (ClampMin = "0.0", Units = "Seconds"))
	float OccupyDuration = 0.0f;

	/**
	 * Minimum occupy time before allowing interrupt
	 * Prevents premature exits
	 */
	UPROPERTY(EditAnywhere, Category = "Occupy", meta = (ClampMin = "0.0", Units = "Seconds"))
	float MinimumOccupyTime = 3.0f;

	//////////////////////////////////////////////////////////////////////////
	// Exit Configuration
	//////////////////////////////////////////////////////////////////////////

	/** Whether to use fast exit animation in combat/urgent situations */
	UPROPERTY(EditAnywhere, Category = "Exit")
	bool bAllowFastExit = true;

	//////////////////////////////////////////////////////////////////////////
	// Advanced
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Whether to take control of the character's movement
	 * If true, disables AI movement while in workspot
	 */
	UPROPERTY(EditAnywhere, Category = "Advanced")
	bool bTakeCharacterControl = true;

public:
	/**
	 * Get the workspot tree for this config
	 */
	UFUNCTION(BlueprintPure, Category = "Workspot")
	UWorkspotTree* GetWorkspotTree() const { return WorkspotTree; }

	/**
	 * Validate configuration
	 */
	bool IsValid() const;

#if WITH_EDITOR
	/** Validate configuration in editor */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
