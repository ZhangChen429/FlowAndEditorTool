// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WorkspotTypes.h"
#include "WorkspotInstance.generated.h"

class UWorkspotTree;
class FWorkspotIterator;
class UAnimInstance;
class UAnimMontage;
struct FWorkspotGlobalProp;

/**
 * WorkspotInstance - Runtime executor for a WorkspotTree
 *
 * Architecture (based on Cyberpunk 2077 WorkspotInstanceWrapper):
 * - Created by WorkspotSubsystem
 * - One instance per Actor using a workspot
 * - Contains Iterator and execution state
 * - Drives animation playback
 * - Manages props and idle transitions
 *
 * Lifecycle:
 * 1. Created by Subsystem->StartWorkspot()
 * 2. Setup() - Initialize with tree and actor
 * 3. Tick() - Update animation and iterator
 * 4. OnCompleted() - Cleanup when finished
 */
UCLASS()
class WORKSPOT_API UWorkspotInstance : public UObject
{
	GENERATED_BODY()

public:
	//////////////////////////////////////////////////////////////////////////
	// Lifecycle
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Initialize the workspot instance
	 * @param InActor - The actor using this workspot
	 * @param InTree - The workspot tree to execute
	 * @param EntryPointTag - Optional entry point tag
	 * @return true if setup succeeded
	 */
	bool Setup(AActor* InActor, UWorkspotTree* InTree, FName EntryPointTag = NAME_None);

	/**
	 * Update the workspot instance
	 * @param DeltaTime - Time since last tick
	 */
	void Tick(float DeltaTime);

	/**
	 * Stop the workspot
	 * @param bForceStop - If true, immediately stop without exit animation
	 */
	void Stop(bool bForceStop = false);

	/**
	 * Check if instance is active
	 */
	bool IsActive() const { return State == EWorkspotState::Playing; }

	/**
	 * Check if instance has finished
	 */
	bool IsFinished() const { return State == EWorkspotState::Inactive || State == EWorkspotState::Finished; }

	//////////////////////////////////////////////////////////////////////////
	// State Access
	//////////////////////////////////////////////////////////////////////////

	/** Get current state */
	EWorkspotState GetState() const { return State; }

	/** Get the actor using this workspot */
	AActor* GetActor() const { return Actor.Get(); }

	/** Get the workspot tree being executed */
	UWorkspotTree* GetWorkspotTree() const { return WorkspotTree.Get(); }

	/** Get current idle animation name */
	FName GetCurrentIdle() const { return CurrentIdleAnim; }

	/** Get current idle animation name (alias for compatibility) */
	FName GetCurrentIdleAnim() const { return CurrentIdleAnim; }

	/** Get current play time */
	float GetCurrentPlayTime() const { return CurrentPlayTime; }

	/** Get current entry data */
	const FWorkspotEntryData& GetCurrentEntryData() const { return CurrentEntryData; }

	/** Get iterator (for debug only) */
	TSharedPtr<FWorkspotIterator> GetIterator() const { return Iterator; }

	//////////////////////////////////////////////////////////////////////////
	// Callbacks
	//////////////////////////////////////////////////////////////////////////

	/** Completion callback delegate */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnWorkspotCompleted, UWorkspotInstance*);
	FOnWorkspotCompleted OnCompleted;

	/** Idle changed callback delegate */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnIdleChanged, FName /*OldIdle*/, FName /*NewIdle*/);
	FOnIdleChanged OnIdleChanged;

	//////////////////////////////////////////////////////////////////////////
	// Advanced
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Trigger a reaction
	 * @param ReactionType - Type of reaction (e.g., "Bump", "Hit")
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot")
	void TriggerReaction(FName ReactionType);

private:
	//////////////////////////////////////////////////////////////////////////
	// Internal State
	//////////////////////////////////////////////////////////////////////////

	/** The actor using this workspot */
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> Actor;

	/** The workspot tree being executed */
	UPROPERTY(Transient)
	TObjectPtr<UWorkspotTree> WorkspotTree;

	/** Current execution state */
	UPROPERTY(Transient)
	EWorkspotState State;

	/** Iterator for traversing the entry tree */
	TSharedPtr<FWorkspotIterator> Iterator;

	/** Current entry data being played */
	FWorkspotEntryData CurrentEntryData;

	/** Current idle animation name */
	FName CurrentIdleAnim;

	/** Previous idle animation name */
	FName PreviousIdleAnim;

	/** Current animation play time */
	float CurrentPlayTime;

	/** Spawned props (PropId -> Actor) */
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<AActor>> SpawnedProps;

	//////////////////////////////////////////////////////////////////////////
	// Internal Methods
	//////////////////////////////////////////////////////////////////////////

	/** Get animation instance from actor */
	UAnimInstance* GetAnimInstance() const;

	/** Get skeletal mesh component from actor */
	class USkeletalMeshComponent* GetSkeletalMeshComponent() const;

	/** Play current animation */
	void PlayCurrentAnimation();

	/** Check if current animation has finished */
	bool IsCurrentAnimationFinished() const;

	/** Handle idle state change */
	void HandleIdleChange(FName NewIdle);

	/** Try to play transition animation */
	bool PlayTransitionAnimation(FName FromIdle, FName ToIdle);

	/** Spawn a prop */
	void SpawnProp(const FWorkspotGlobalProp& PropDef);

	/** Despawn a prop */
	void DespawnProp(FName PropId);

	/** Despawn all props */
	void DespawnAllProps();
};
