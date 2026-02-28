// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WorkspotTypes.generated.h"

class UAnimMontage;
class AActor;
class UWorkspotTree;

/**
 * Workspot execution context - passed to iterators for tree traversal
 *
 * This is a pure data context used by Iterator system to traverse WorkspotTree.
 * It does NOT contain playback-related objects (AnimInstance, SkeletalMeshComponent).
 * Playback is handled separately by WorkspotInstance.
 */
USTRUCT(BlueprintType)
struct WORKSPOT_API FWorkspotContext
{
	GENERATED_BODY()

	/** The actor using the workspot (for position/rotation queries) */
	UPROPERTY(BlueprintReadWrite, Category = "Workspot")
	TWeakObjectPtr<AActor> User;

	/** The WorkspotTree being executed (for transition lookup) */
	UPROPERTY(BlueprintReadWrite, Category = "Workspot")
	TWeakObjectPtr<UWorkspotTree> Tree;

	/** Random number generator for selection */
	FRandomStream RandomGen;

	/** Current idle animation name */
	UPROPERTY(BlueprintReadWrite, Category = "Workspot")
	FName CurrentIdle = "stand";

	/** Previous idle animation name (for transitions) */
	UPROPERTY(BlueprintReadWrite, Category = "Workspot")
	FName PreviousIdle = "stand";

	FWorkspotContext()
	{
		RandomGen.Initialize(FMath::Rand());
	}
};

/**
 * Data retrieved from an Entry during iteration
 */
USTRUCT(BlueprintType)
struct WORKSPOT_API FWorkspotEntryData
{
	GENERATED_BODY()

	/** Animation montage to play */
	UPROPERTY(BlueprintReadWrite, Category = "Workspot")
	TObjectPtr<UAnimMontage> AnimMontage = nullptr;

	/** Idle animation name for this entry */
	UPROPERTY(BlueprintReadWrite, Category = "Workspot")
	FName IdleAnim = "stand";

	/** Blend in time for animation */
	UPROPERTY(BlueprintReadWrite, Category = "Workspot")
	float BlendInTime = 0.3f;

	/** Blend out time for animation */
	UPROPERTY(BlueprintReadWrite, Category = "Workspot")
	float BlendOutTime = 0.3f;

	/** Whether this data is valid */
	UPROPERTY(BlueprintReadWrite, Category = "Workspot")
	bool bIsValid = false;

	FWorkspotEntryData() = default;
};

/**
 * Workspot component state
 */
UENUM(BlueprintType)
enum class EWorkspotState : uint8
{
	Inactive,
	Starting,
	Playing,
	Stopping,
	Finished
};

/**
 * Entry flags (bitfield)
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EWorkspotEntryFlags : uint8
{
	None = 0,
	SlowEnter = 1 << 0,
	SlowExit = 1 << 1,
	FastExit = 1 << 2,
	CanBeInterrupted = 1 << 3
};
ENUM_CLASS_FLAGS(EWorkspotEntryFlags);
