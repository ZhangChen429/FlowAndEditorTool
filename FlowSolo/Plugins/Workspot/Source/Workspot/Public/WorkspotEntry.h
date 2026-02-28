// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WorkspotTypes.h"
#include "WorkspotEntry.generated.h"

class FWorkspotIterator;
class UAnimMontage;

/**
 * Base class for all Workspot entries
 * Uses Composite Pattern to build behavior trees
 */
UCLASS(Abstract, EditInlineNew, BlueprintType, CollapseCategories)
class WORKSPOT_API UWorkspotEntry : public UObject
{
	GENERATED_BODY()

public:
	/** Unique ID for this entry */
	UPROPERTY(VisibleAnywhere, Category = "Entry")
	FGuid EntryId;

	/** Idle animation name (stand, sit, crouch, etc.) */
	UPROPERTY(EditAnywhere, Category = "Entry", meta = (GetOptions = "GetIdleAnimOptions"))
	FName IdleAnim = "stand";

	/** Entry flags */
	UPROPERTY(EditAnywhere, Category = "Entry", meta = (Bitmask, BitmaskEnum = "/Script/Workspot.EWorkspotEntryFlags"))
	int32 Flags = 0;

public:
	UWorkspotEntry();

	/**
	 * Create an iterator for this entry
	 * Pure virtual - must be implemented by subclasses
	 */
	virtual TSharedPtr<FWorkspotIterator> CreateIterator(FWorkspotContext& Context) const
	{
		unimplemented();
		return nullptr;
	}

	/**
	 * Recursively visit all nodes in the tree
	 * Used for debugging and editor visualization
	 */
	virtual void ForEachNode(TFunctionRef<void(const UWorkspotEntry*)> Func) const
	{
		Func(this);
	}

	/** Get idle anim options for dropdown */
	UFUNCTION()
	TArray<FString> GetIdleAnimOptions() const
	{
		return TArray<FString>{"stand", "sit", "crouch", "prone", "kneel"};
	}

#if WITH_EDITOR
	/** Validate entry configuration */
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
};

//////////////////////////////////////////////////////////////////////////
// Leaf Nodes
//////////////////////////////////////////////////////////////////////////

/**
 * AnimClip - Plays a single animation montage
 */
UCLASS(DisplayName = "Anim Clip")
class WORKSPOT_API UWorkspotAnimClip : public UWorkspotEntry
{
	GENERATED_BODY()

public:
	/** Animation montage to play */
	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> AnimMontage;

	/** Blend in time */
	UPROPERTY(EditAnywhere, Category = "Animation", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float BlendInTime = 0.3f;

	/** Blend out time */
	UPROPERTY(EditAnywhere, Category = "Animation", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float BlendOutTime = 0.3f;

public:
	virtual TSharedPtr<FWorkspotIterator> CreateIterator(FWorkspotContext& Context) const override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
};

/**
 * EntryAnim - Entry point with multiple approach angles
 */
UCLASS(DisplayName = "Entry Anim")
class WORKSPOT_API UWorkspotEntryAnim : public UWorkspotEntry
{
	GENERATED_BODY()

public:
	/** Animation montage to play */
	UPROPERTY(EditAnywhere, Category = "Entry")
	TObjectPtr<UAnimMontage> AnimMontage;

	/** Entry point transform (relative to workspot) */
	UPROPERTY(EditAnywhere, Category = "Entry")
	FTransform EntryTransform;

	/** Whether this is a slow entry (walking) */
	UPROPERTY(EditAnywhere, Category = "Entry")
	bool bSlowEnter = false;

public:
	virtual TSharedPtr<FWorkspotIterator> CreateIterator(FWorkspotContext& Context) const override;
};

/**
 * ExitAnim - Exit animation
 */
UCLASS(DisplayName = "Exit Anim")
class WORKSPOT_API UWorkspotExitAnim : public UWorkspotEntry
{
	GENERATED_BODY()

public:
	/** Animation montage to play */
	UPROPERTY(EditAnywhere, Category = "Exit")
	TObjectPtr<UAnimMontage> AnimMontage;

	/** Whether this is a fast exit (combat) */
	UPROPERTY(EditAnywhere, Category = "Exit")
	bool bFastExit = false;

public:
	virtual TSharedPtr<FWorkspotIterator> CreateIterator(FWorkspotContext& Context) const override;
};

//////////////////////////////////////////////////////////////////////////
// Container Nodes
//////////////////////////////////////////////////////////////////////////

/**
 * Sequence - Plays entries in order
 */
UCLASS(DisplayName = "Sequence")
class WORKSPOT_API UWorkspotSequence : public UWorkspotEntry
{
	GENERATED_BODY()

public:
	/** Child entries to play in sequence */
	UPROPERTY(EditAnywhere, Instanced, Category = "Sequence")
	TArray<TObjectPtr<UWorkspotEntry>> Entries;

	/** Whether to loop infinitely */
	UPROPERTY(EditAnywhere, Category = "Sequence")
	bool bLoopInfinitely = false;

	/** Maximum number of loops (if not infinite) */
	UPROPERTY(EditAnywhere, Category = "Sequence", meta = (EditCondition = "!bLoopInfinitely", ClampMin = "1"))
	int32 MaxLoops = 1;

public:
	virtual TSharedPtr<FWorkspotIterator> CreateIterator(FWorkspotContext& Context) const override;

	virtual void ForEachNode(TFunctionRef<void(const UWorkspotEntry*)> Func) const override
	{
		Func(this);
		for (const auto& Entry : Entries)
		{
			if (Entry)
			{
				Entry->ForEachNode(Func);
			}
		}
	}

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
};

/**
 * RandomList - Randomly selects entries to play
 */
UCLASS(DisplayName = "Random List")
class WORKSPOT_API UWorkspotRandomList : public UWorkspotEntry
{
	GENERATED_BODY()

public:
	/** Child entries to randomly select from */
	UPROPERTY(EditAnywhere, Instanced, Category = "Random")
	TArray<TObjectPtr<UWorkspotEntry>> Entries;

	/** Selection weights (must match Entries count) */
	UPROPERTY(EditAnywhere, Category = "Random")
	TArray<float> Weights;

	/** Minimum number of clips to play */
	UPROPERTY(EditAnywhere, Category = "Random", meta = (ClampMin = "1"))
	int32 MinClips = 1;

	/** Maximum number of clips to play */
	UPROPERTY(EditAnywhere, Category = "Random", meta = (ClampMin = "1"))
	int32 MaxClips = 1;

	/** Pause duration between clips (min, max) */
	UPROPERTY(EditAnywhere, Category = "Random")
	FVector2D PauseDuration = FVector2D(0.0f, 1.0f);

public:
	virtual TSharedPtr<FWorkspotIterator> CreateIterator(FWorkspotContext& Context) const override;

	virtual void ForEachNode(TFunctionRef<void(const UWorkspotEntry*)> Func) const override
	{
		Func(this);
		for (const auto& Entry : Entries)
		{
			if (Entry)
			{
				Entry->ForEachNode(Func);
			}
		}
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
};

/**
 * Selector - Selects one branch, handles idle transitions
 * ⭐ Core innovation: automatic posture transitions
 */
UCLASS(DisplayName = "Selector")
class WORKSPOT_API UWorkspotSelector : public UWorkspotEntry
{
	GENERATED_BODY()

public:
	/** Child entries to select from */
	UPROPERTY(EditAnywhere, Instanced, Category = "Selector")
	TArray<TObjectPtr<UWorkspotEntry>> Entries;

	/** Selection weights (must match Entries count) */
	UPROPERTY(EditAnywhere, Category = "Selector")
	TArray<float> Weights;

	/** Base idle anim (used for fallback if transition missing) */
	UPROPERTY(EditAnywhere, Category = "Selector")
	FName BaseIdleAnim = "stand";

public:
	virtual TSharedPtr<FWorkspotIterator> CreateIterator(FWorkspotContext& Context) const override;

	virtual void ForEachNode(TFunctionRef<void(const UWorkspotEntry*)> Func) const override
	{
		Func(this);
		for (const auto& Entry : Entries)
		{
			if (Entry)
			{
				Entry->ForEachNode(Func);
			}
		}
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
};
