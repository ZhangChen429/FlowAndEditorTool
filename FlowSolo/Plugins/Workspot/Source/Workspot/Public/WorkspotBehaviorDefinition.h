// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SmartObjectDefinition.h"
#include "WorkspotBehaviorDefinition.generated.h"

class UWorkspotTree;
class UGameplayBehavior;

/**
 * Workspot behavior definition for SmartObjects integration
 *
 * This is a SmartObject BehaviorDefinition that contains a WorkspotTree asset.
 * When AI occupies a SmartObject slot with this definition, the SmartObject system
 * will instantiate UGameplayBehavior_Workspot and pass this definition to it.
 *
 * Flow:
 * 1. SmartObject slot has WorkspotBehaviorDefinition with WorkspotTree reference
 * 2. AI claims slot → SmartObject instantiates GameplayBehavior_Workspot
 * 3. GameplayBehavior reads WorkspotTree from this definition
 * 4. Behavior starts WorkspotTree on AI's WorkspotComponent
 * 5. Monitors completion and releases slot when done
 */
UCLASS(DisplayName = "Workspot")
class WORKSPOT_API UWorkspotBehaviorDefinition : public USmartObjectBehaviorDefinition
{
	GENERATED_BODY()

public:
	//////////////////////////////////////////////////////////////////////////
	// Core Configuration
	//////////////////////////////////////////////////////////////////////////

	/** The workspot tree to execute when this behavior is used */
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
	UWorkspotBehaviorDefinition();

	//////////////////////////////////////////////////////////////////////////
	// GameplayBehavior Integration
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Get the GameplayBehavior class to use for this definition
	 * Returns UGameplayBehavior_Workspot by default
	 */
	virtual TSubclassOf<UGameplayBehavior> GetBehaviorClass() const;

	/**
	 * Helper function to start workspot on an actor
	 * @param Actor - The actor to start workspot on (must have WorkspotComponent)
	 * @return true if workspot started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot")
	bool StartWorkspotOnActor(AActor* Actor) const;

	/**
	 * Get the workspot tree for this definition
	 */
	UFUNCTION(BlueprintPure, Category = "Workspot")
	UWorkspotTree* GetWorkspotTree() const { return WorkspotTree; }

#if WITH_EDITOR
	/** Validate behavior configuration */
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
};
