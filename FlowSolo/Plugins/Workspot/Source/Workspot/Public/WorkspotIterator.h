// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WorkspotTypes.h"

class UWorkspotExitAnim;
class UWorkspotEntryAnim;
class UWorkspotEntry;
class UWorkspotAnimClip;
class UWorkspotSequence;
class UWorkspotRandomList;
class UWorkspotSelector;
class UWorkspotTree;

/**
 * Base iterator interface for traversing Workspot entries
 * Implements the Iterator Pattern
 */
class WORKSPOT_API FWorkspotIterator
{
public:
	virtual ~FWorkspotIterator() = default;

	/**
	 * Advance to next entry
	 * @param Context - Execution context
	 * @return true if advanced successfully, false if iteration complete
	 */
	virtual bool Next(FWorkspotContext& Context) = 0;

	/**
	 * Get data for current entry
	 * @param OutData - Output entry data
	 * @return true if data is valid
	 */
	virtual bool GetData(FWorkspotEntryData& OutData) const = 0;

	/**
	 * Reset iterator to beginning
	 */
	virtual void Reset() = 0;

	/**
	 * Check if iterator has more entries
	 */
	virtual bool HasNext() const { return false; }

	/**
	 * Get debug string for this iterator (type and current state)
	 */
	FString GetDebugString() const;

	/**
	 * Get iterator type name (override in subclasses)
	 */
	virtual const TCHAR* GetTypeName() const = 0;

	/**
	 * Get iterator detail string (override in subclasses for extra info)
	 */
	virtual FString GetDetailString() const { return FString(); }
};

/**
 * Iterator for AnimClip (leaf node)
 */
class WORKSPOT_API FAnimClipIterator : public FWorkspotIterator
{
public:
	explicit FAnimClipIterator(const UWorkspotAnimClip* InClip);

	virtual bool Next(FWorkspotContext& Context) override;
	virtual bool GetData(FWorkspotEntryData& OutData) const override;
	virtual void Reset() override;
	virtual bool HasNext() const override { return !bHasPlayed; }
	virtual const TCHAR* GetTypeName() const override { return TEXT("AnimClip"); }

private:
	const UWorkspotAnimClip* AnimClip;
	bool bHasPlayed;
};

/**
 * Iterator for EntryAnim (leaf node)
 */
class WORKSPOT_API FEntryAnimIterator : public FWorkspotIterator
{
public:
	explicit FEntryAnimIterator(const UWorkspotEntryAnim* InEntry);

	virtual bool Next(FWorkspotContext& Context) override;
	virtual bool GetData(FWorkspotEntryData& OutData) const override;
	virtual void Reset() override;
	virtual const TCHAR* GetTypeName() const override { return TEXT("EntryAnim"); }

private:
	const UWorkspotEntryAnim* EntryAnim;
	bool bHasPlayed;
};

/**
 * Iterator for ExitAnim (leaf node)
 */
class WORKSPOT_API FExitAnimIterator : public FWorkspotIterator
{
public:
	explicit FExitAnimIterator(const UWorkspotExitAnim* InExit);

	virtual bool Next(FWorkspotContext& Context) override;
	virtual bool GetData(FWorkspotEntryData& OutData) const override;
	virtual void Reset() override;
	virtual const TCHAR* GetTypeName() const override { return TEXT("ExitAnim"); }

private:
	const UWorkspotExitAnim* ExitAnim;
	bool bHasPlayed;
};

/**
 * Iterator for Sequence (container node)
 * Plays children in order
 */
class WORKSPOT_API FSequenceIterator : public FWorkspotIterator
{
public:
	explicit FSequenceIterator(const UWorkspotSequence* InSequence);

	virtual bool Next(FWorkspotContext& Context) override;
	virtual bool GetData(FWorkspotEntryData& OutData) const override;
	virtual void Reset() override;
	virtual bool HasNext() const override;
	virtual const TCHAR* GetTypeName() const override { return TEXT("Sequence"); }
	virtual FString GetDetailString() const override;

private:
	const UWorkspotSequence* Sequence;
	int32 CurrentIndex;
	TSharedPtr<FWorkspotIterator> ChildIterator;
	int32 LoopCount;
};

/**
 * Iterator for RandomList (container node)
 * Randomly selects children based on weights
 */
class WORKSPOT_API FRandomListIterator : public FWorkspotIterator
{
public:
	explicit FRandomListIterator(const UWorkspotRandomList* InRandomList);

	virtual bool Next(FWorkspotContext& Context) override;
	virtual bool GetData(FWorkspotEntryData& OutData) const override;
	virtual void Reset() override;
	virtual bool HasNext() const override;
	virtual const TCHAR* GetTypeName() const override { return TEXT("RandomList"); }
	virtual FString GetDetailString() const override;

private:
	const UWorkspotRandomList* RandomList;
	TArray<int32> SelectedIndices;
	int32 CurrentPlayIndex;
	TSharedPtr<FWorkspotIterator> ChildIterator;
	float PauseRemainingTime;
	bool bInPause;

	void SelectRandomEntries(FWorkspotContext& Context);
	int32 ChooseWeightedRandom(const TArray<float>& Weights, FRandomStream& Random) const;
};

/**
 * Iterator for Selector (container node)
 * ⭐ Core feature: automatic idle transition insertion
 */
class WORKSPOT_API FSelectorIterator : public FWorkspotIterator
{
public:
	explicit FSelectorIterator(const UWorkspotSelector* InSelector, UWorkspotTree* InTree);

	virtual bool Next(FWorkspotContext& Context) override;
	virtual bool GetData(FWorkspotEntryData& OutData) const override;
	virtual void Reset() override;
	virtual bool HasNext() const override;
	virtual const TCHAR* GetTypeName() const override { return TEXT("Selector"); }
	virtual FString GetDetailString() const override;

private:
	const UWorkspotSelector* Selector;
	UWorkspotTree* Tree; // Needed for transition anim lookup
	int32 CurrentSelection;
	TSharedPtr<FWorkspotIterator> ChildIterator;
	bool bFirstRun;
	bool bPlayingTransition;
	FWorkspotEntryData TransitionData;

	int32 ChooseWeightedRandom(const TArray<float>& Weights, FRandomStream& Random) const;
	bool TryInsertTransitionAnim(FName FromIdle, FName ToIdle, FWorkspotContext& Context);
};
