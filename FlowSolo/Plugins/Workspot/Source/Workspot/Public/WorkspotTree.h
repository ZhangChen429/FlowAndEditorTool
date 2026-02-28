// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "WorkspotTypes.h"
#include "WorkspotTree.generated.h"

class UWorkspotEntry;
class UAnimMontage;
class USkeleton;

/**
 * Transition animation definition
 * Defines animation to play when transitioning between idle states
 */
USTRUCT(BlueprintType)
struct WORKSPOT_API FWorkspotTransitionAnim
{
	GENERATED_BODY()

	/** Source idle animation name */
	UPROPERTY(EditAnywhere, Category = "Transition")
	FName FromIdle;

	/** Target idle animation name */
	UPROPERTY(EditAnywhere, Category = "Transition")
	FName ToIdle;

	/** Transition animation montage */
	UPROPERTY(EditAnywhere, Category = "Transition")
	TObjectPtr<UAnimMontage> TransitionMontage;

	FWorkspotTransitionAnim()
		: FromIdle(NAME_None)
		, ToIdle(NAME_None)
		, TransitionMontage(nullptr)
	{}
};

/**
 * Global prop definition
 * Props that are spawned/despawned based on idle state changes
 */
USTRUCT(BlueprintType)
struct WORKSPOT_API FWorkspotGlobalProp
{
	GENERATED_BODY()

	/** Unique prop identifier */
	UPROPERTY(EditAnywhere, Category = "Prop")
	FName PropId;

	/** Socket name to attach prop to */
	UPROPERTY(EditAnywhere, Category = "Prop")
	FName AttachSocketName;

	/** Prop actor class to spawn */
	UPROPERTY(EditAnywhere, Category = "Prop")
	TSubclassOf<AActor> PropClass;

	/** Spawn prop when idle changes */
	UPROPERTY(EditAnywhere, Category = "Prop")
	bool bSpawnOnIdleChange = true;

	/** Despawn prop when idle changes */
	UPROPERTY(EditAnywhere, Category = "Prop")
	bool bDespawnOnIdleChange = false;

	/** Despawn prop on reaction */
	UPROPERTY(EditAnywhere, Category = "Prop")
	bool bDespawnOnReaction = true;

	/** Which idle state triggers spawn (empty = any) */
	UPROPERTY(EditAnywhere, Category = "Prop", meta = (EditCondition = "bSpawnOnIdleChange"))
	FName SpawnOnIdle;

	FWorkspotGlobalProp()
		: PropId(NAME_None)
		, AttachSocketName(NAME_None)
		, PropClass(nullptr)
	{}
};

/**
 * WorkspotTree - Main asset defining NPC behavior composition
 *
 * Core innovation: Entry composition pattern
 * Combines simple entries into complex behaviors through recursion
 *
 * Uses custom Toolkit editor for visual authoring with:
 * - Visual graph view for Entry tree composition
 * - Details panel for configuring entries and transitions
 * - Preview viewport for testing animations
 */
UCLASS(BlueprintType)
class WORKSPOT_API UWorkspotTree : public UObject
{
	GENERATED_BODY()

public:
	//////////////////////////////////////////////////////////////////////////
	// Core Behavior Definition
	//////////////////////////////////////////////////////////////////////////

	/** Root entry (must be a container: Sequence, Selector, etc.) */
	UPROPERTY(EditAnywhere, Instanced, Category = "Workspot", meta = (ShowOnlyInnerProperties))
	TObjectPtr<UWorkspotEntry> RootEntry;

	//////////////////////////////////////////////////////////////////////////
	// Animation Binding
	//////////////////////////////////////////////////////////////////////////

	/** Skeleton this workspot is designed for */
	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<USkeleton> WorkspotSkeleton;

	//////////////////////////////////////////////////////////////////////////
	// Posture Transition System (⭐ Workspot Innovation)
	//////////////////////////////////////////////////////////////////////////

	/** Custom transition animations (higher priority than naming convention) */
	UPROPERTY(EditAnywhere, Category = "Posture", meta = (TitleProperty = "FromIdle"))
	TArray<FWorkspotTransitionAnim> CustomTransitionAnims;

	/** Default blend time for automatic transitions (using Inertialization) */
	UPROPERTY(EditAnywhere, Category = "Posture", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float DefaultTransitionBlendTime = 0.3f;

	//////////////////////////////////////////////////////////////////////////
	// Props System
	//////////////////////////////////////////////////////////////////////////

	/** Global props that can be spawned during workspot execution */
	UPROPERTY(EditAnywhere, Category = "Props", meta = (TitleProperty = "PropId"))
	TArray<FWorkspotGlobalProp> GlobalProps;

	//////////////////////////////////////////////////////////////////////////
	// Filtering System
	//////////////////////////////////////////////////////////////////////////

	/** Tags describing this workspot (e.g., "Sit", "Drink", "Work") */
	UPROPERTY(EditAnywhere, Category = "Filtering")
	FGameplayTagContainer Tags;

	/** Query to filter which users can use this workspot */
	UPROPERTY(EditAnywhere, Category = "Filtering")
	FGameplayTagQuery UserTagFilter;

	//////////////////////////////////////////////////////////////////////////
	// Performance Optimization
	//////////////////////////////////////////////////////////////////////////

	/** Inertialization duration when entering workspot */
	UPROPERTY(EditAnywhere, Category = "Performance", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float InertializationDurationEnter = 0.5f;

	/** Inertialization duration when naturally exiting */
	UPROPERTY(EditAnywhere, Category = "Performance", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float InertializationDurationExitNatural = 0.5f;

	/** Inertialization duration when force exiting (combat) */
	UPROPERTY(EditAnywhere, Category = "Performance", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float InertializationDurationExitForced = 0.2f;

public:
	UWorkspotTree();

	/**
	 * Find transition animation between two idle states
	 * Priority: Custom > Naming convention ("FromIdle__2__ToIdle")
	 */
	UAnimMontage* FindTransitionAnim(FName FromIdle, FName ToIdle) const;

	/**
	 * Check if this workspot is valid for use
	 */
	bool IsValid() const;

	/**
	 * Get all animation montages referenced by this tree
	 * Used for streaming and validation
	 */
	void GetReferencedAnimations(TArray<UAnimMontage*>& OutAnimations) const;

	/**
	 * Count total number of entries in tree
	 * Used for debugging
	 */
	int32 GetEntryCount() const;

#if WITH_EDITOR
	/** Validate workspot configuration */
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;

	/** Generate unique IDs for entries that don't have one */
	void RegenerateEntryIds();
#endif
};
