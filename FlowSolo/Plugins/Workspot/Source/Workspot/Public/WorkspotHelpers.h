// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SmartObjectSubsystem.h"
#include "WorkspotHelpers.generated.h"

class UWorkspotTree;
class UWorkspotBehaviorConfig;

/**
 * Helper functions for Workspot integration with SmartObjects
 */
UCLASS()
class WORKSPOT_API UWorkspotHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Get WorkspotTree from a SmartObject claim handle
	 * @param WorldContextObject - World context for accessing subsystems
	 * @param ClaimHandle - The claim handle from SmartObject system
	 * @return The WorkspotTree if the slot has a WorkspotBehaviorConfig, nullptr otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "Workspot", meta = (WorldContext = "WorldContextObject"))
	static UWorkspotTree* GetWorkspotTreeFromClaimHandle(UObject* WorldContextObject, const FSmartObjectClaimHandle& ClaimHandle);

	/**
	 * Get WorkspotBehaviorConfig from a SmartObject claim handle
	 * @param WorldContextObject - World context for accessing subsystems
	 * @param ClaimHandle - The claim handle from SmartObject system
	 * @return The WorkspotBehaviorConfig if found, nullptr otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "Workspot", meta = (WorldContext = "WorldContextObject"))
	static UWorkspotBehaviorConfig* GetWorkspotConfigFromClaimHandle(UObject* WorldContextObject, const FSmartObjectClaimHandle& ClaimHandle);

	/**
	 * Start workspot from a SmartObject claim handle
	 * This is the main function to use when AI occupies a SmartObject slot
	 *
	 * @param Avatar - The actor to use the workspot
	 * @param ClaimHandle - The claim handle from SmartObject system
	 * @return true if workspot started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot")
	static bool StartWorkspotFromClaimHandle(AActor* Avatar, const FSmartObjectClaimHandle& ClaimHandle);

	/**
	 * Directly play a WorkspotTree on an actor (bypasses SmartObject system)
	 * Use this for simple scenarios or testing
	 *
	 * @param Actor - The actor to execute workspot on (must have SkeletalMeshComponent)
	 * @param WorkspotTree - The workspot tree to execute
	 * @param EntryPointTag - Optional entry point tag (empty = auto-select closest)
	 * @return true if workspot started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot", meta = (AdvancedDisplay = "EntryPointTag"))
	static bool PlayWorkspotOnActor(AActor* Actor, UWorkspotTree* WorkspotTree, FName EntryPointTag = NAME_None);

	/**
	 * Stop workspot on an actor
	 *
	 * @param Actor - The actor to stop workspot on
	 * @param bForceStop - If true, immediately stops without exit animation
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot")
	static void StopWorkspotOnActor(AActor* Actor, bool bForceStop = false);

	/**
	 * Check if an actor is currently playing a workspot
	 *
	 * @param Actor - The actor to check
	 * @return true if actor has an active workspot
	 */
	UFUNCTION(BlueprintPure, Category = "Workspot")
	static bool IsActorPlayingWorkspot(AActor* Actor);

	/**
	 * ⭐ MAIN WORKFLOW: Claim and Use SmartObject slot, then start Workspot
	 * Complete integration between SmartObject system and Workspot system
	 *
	 * Workflow:
	 * 1. Claim the slot (if not already claimed) using MarkSlotAsClaimed
	 * 2. Use the slot (mark as occupied) using MarkSlotAsOccupied
	 * 3. Get WorkspotBehaviorConfig from the occupied slot
	 * 4. Get WorkspotTree from config
	 * 5. Start Workspot on the controller's pawn via WorkspotSubsystem
	 * 6. Lock AI logic if requested (stop movement, disable behavior tree)
	 *
	 * @param Controller - AI controller whose pawn will use the workspot
	 * @param ClaimHandle - Handle to the claimed SmartObject slot
	 * @param bLockAILogic - If true, stops AI movement during workspot execution
	 * @return The WorkspotInstance if successful, nullptr otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot")
	static class UWorkspotInstance* ClaimAndUseWorkspotSlot(
		AAIController* Controller,
		const FSmartObjectClaimHandle& ClaimHandle,
		bool bLockAILogic = true
	);

	/**
	 * Use (occupy) a claimed SmartObject slot
	 * Marks the slot as occupied and returns the BehaviorDefinition
	 *
	 * @param WorldContextObject - World context for accessing subsystems
	 * @param ClaimHandle - Handle to the claimed slot
	 * @return The GameplayBehaviorSmartObjectBehaviorDefinition if successful, nullptr otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot", meta = (WorldContext = "WorldContextObject"))
	static const class UGameplayBehaviorSmartObjectBehaviorDefinition* UseSmartObjectSlot(
		UObject* WorldContextObject,
		const FSmartObjectClaimHandle& ClaimHandle
	);

	/**
	 * Lock AI logic during Workspot execution
	 * Stops movement and pauses behavior tree
	 *
	 * @param Controller - AI controller to lock
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot")
	static void LockAILogic(AAIController* Controller);

	/**
	 * Unlock AI logic after Workspot execution
	 * Resumes behavior tree (movement is controlled by BT)
	 *
	 * @param Controller - AI controller to unlock
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot")
	static void UnlockAILogic(AAIController* Controller);

	/**
	 * 🔍 DIAGNOSTIC: Verify SmartObject slot configuration
	 * Logs detailed information about BehaviorDefinition and Config setup
	 * Use this to debug why Trigger() isn't being called
	 *
	 * @param WorldContextObject - World context for accessing subsystems
	 * @param ClaimHandle - The claim handle to verify
	 * @return true if configuration is valid and complete
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot|Debug", meta = (WorldContext = "WorldContextObject"))
	static bool VerifySmartObjectConfiguration(UObject* WorldContextObject, const FSmartObjectClaimHandle& ClaimHandle);

	/**
	 * 🔧 MANUAL TRIGGER: Use SmartObject slot with GameplayBehavior
	 * Manually calls SmartObjectSubsystem::Use() on the claimed slot
	 * Use this if MoveToAndUseSmartObjectWithGameplayBehavior fails
	 *
	 * @param WorldContextObject - World context for accessing subsystems
	 * @param ClaimHandle - The claim handle to use
	 * @param Avatar - The actor that will execute the behavior
	 * @return true if Use() succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot|Debug", meta = (WorldContext = "WorldContextObject"))
	static bool ManuallyUseSmartObjectSlot(UObject* WorldContextObject, const FSmartObjectClaimHandle& ClaimHandle, AActor* Avatar);
};
