// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkspotIterator.h"
#include "WorkspotEntry.h"
#include "WorkspotTree.h"
#include "Workspot.h"
#include "Animation/AnimMontage.h"

//////////////////////////////////////////////////////////////////////////
// FWorkspotIterator Base
//////////////////////////////////////////////////////////////////////////

FString FWorkspotIterator::GetDebugString() const
{
	FString DetailStr = GetDetailString();
	if (DetailStr.IsEmpty())
	{
		return FString::Printf(TEXT("%s"), GetTypeName());
	}
	return FString::Printf(TEXT("%s [%s]"), GetTypeName(), *DetailStr);
}

//////////////////////////////////////////////////////////////////////////
// FAnimClipIterator
//////////////////////////////////////////////////////////////////////////

FAnimClipIterator::FAnimClipIterator(const UWorkspotAnimClip* InClip)
	: AnimClip(InClip)
	, bHasPlayed(false)
{
}

bool FAnimClipIterator::Next(FWorkspotContext& Context)
{
	if (bHasPlayed || !AnimClip)
	{
		return false;
	}

	bHasPlayed = true;

	UE_LOG(LogWorkspot, Log, TEXT("  🎬 AnimClipIterator::Next - Playing clip: %s (Idle: %s)"),
		AnimClip->AnimMontage ? *AnimClip->AnimMontage->GetName() : TEXT("NULL"),
		*AnimClip->IdleAnim.ToString());

	return true;
}

bool FAnimClipIterator::GetData(FWorkspotEntryData& OutData) const
{
	if (!bHasPlayed || !AnimClip)
	{
		return false;
	}

	OutData.AnimMontage = AnimClip->AnimMontage;
	OutData.IdleAnim = AnimClip->IdleAnim;
	OutData.BlendInTime = AnimClip->BlendInTime;
	OutData.BlendOutTime = AnimClip->BlendOutTime;
	OutData.bIsValid = true;

	return true;
}

void FAnimClipIterator::Reset()
{
	bHasPlayed = false;
}

//////////////////////////////////////////////////////////////////////////
// FEntryAnimIterator
//////////////////////////////////////////////////////////////////////////

FEntryAnimIterator::FEntryAnimIterator(const UWorkspotEntryAnim* InEntry)
	: EntryAnim(InEntry)
	, bHasPlayed(false)
{
}

bool FEntryAnimIterator::Next(FWorkspotContext& Context)
{
	if (bHasPlayed || !EntryAnim)
	{
		return false;
	}

	bHasPlayed = true;
	return true;
}

bool FEntryAnimIterator::GetData(FWorkspotEntryData& OutData) const
{
	if (!bHasPlayed || !EntryAnim)
	{
		return false;
	}

	OutData.AnimMontage = EntryAnim->AnimMontage;
	OutData.IdleAnim = EntryAnim->IdleAnim;
	OutData.BlendInTime = 0.3f;
	OutData.BlendOutTime = 0.3f;
	OutData.bIsValid = true;

	return true;
}

void FEntryAnimIterator::Reset()
{
	bHasPlayed = false;
}

//////////////////////////////////////////////////////////////////////////
// FExitAnimIterator
//////////////////////////////////////////////////////////////////////////

FExitAnimIterator::FExitAnimIterator(const UWorkspotExitAnim* InExit)
	: ExitAnim(InExit)
	, bHasPlayed(false)
{
}

bool FExitAnimIterator::Next(FWorkspotContext& Context)
{
	if (bHasPlayed || !ExitAnim)
	{
		return false;
	}

	bHasPlayed = true;
	return true;
}

bool FExitAnimIterator::GetData(FWorkspotEntryData& OutData) const
{
	if (!bHasPlayed || !ExitAnim)
	{
		return false;
	}

	OutData.AnimMontage = ExitAnim->AnimMontage;
	OutData.IdleAnim = ExitAnim->IdleAnim;
	OutData.BlendInTime = ExitAnim->bFastExit ? 0.1f : 0.3f;
	OutData.BlendOutTime = ExitAnim->bFastExit ? 0.1f : 0.3f;
	OutData.bIsValid = true;

	return true;
}

void FExitAnimIterator::Reset()
{
	bHasPlayed = false;
}

//////////////////////////////////////////////////////////////////////////
// FSequenceIterator
//////////////////////////////////////////////////////////////////////////

FSequenceIterator::FSequenceIterator(const UWorkspotSequence* InSequence)
	: Sequence(InSequence)
	, CurrentIndex(-1)  // 初始化为-1，第一次++变成0
	, ChildIterator(nullptr)
	, LoopCount(0)
{
}

bool FSequenceIterator::Next(FWorkspotContext& Context)
{
	if (!Sequence || Sequence->Entries.Num() == 0)
	{
		return false;
	}

	// Try to advance current child iterator
	if (ChildIterator.IsValid() && ChildIterator->Next(Context))
	{
		return true;
	}

	// Move to next entry
	CurrentIndex++;

	// Check if we've reached the end
	if (CurrentIndex >= Sequence->Entries.Num())
	{
		if (Sequence->bLoopInfinitely || LoopCount < Sequence->MaxLoops - 1)
		{
			// Loop back to beginning
			CurrentIndex = -1;  // 重置为-1，下次++变成0
			LoopCount++;

			UE_LOG(LogWorkspot, Log, TEXT("  📦 SequenceIterator - Loop %d/%d, restarting from entry 0"),
				LoopCount + 1, Sequence->MaxLoops);

			// 递归调用以开始新的循环
			return Next(Context);
		}
		else
		{
			// Finished
			UE_LOG(LogWorkspot, Log, TEXT("  📦 SequenceIterator - Finished all %d entries"), Sequence->Entries.Num());
			return false;
		}
	}

	// Create iterator for next entry
	const UWorkspotEntry* Entry = Sequence->Entries[CurrentIndex];
	if (!Entry)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("Sequence has null entry at index %d"), CurrentIndex);
		return Next(Context); // Skip to next
	}

	UE_LOG(LogWorkspot, Log, TEXT("  📦 SequenceIterator - Moving to entry [%d/%d]: %s"),
		CurrentIndex + 1, Sequence->Entries.Num(), *Entry->GetClass()->GetName());

	ChildIterator = Entry->CreateIterator(Context);
	if (!ChildIterator.IsValid())
	{
		UE_LOG(LogWorkspot, Error, TEXT("Failed to create iterator for entry"));
		return false;
	}

	return ChildIterator->Next(Context);
}

bool FSequenceIterator::GetData(FWorkspotEntryData& OutData) const
{
	if (ChildIterator.IsValid())
	{
		return ChildIterator->GetData(OutData);
	}
	return false;
}

void FSequenceIterator::Reset()
{
	CurrentIndex = -1;  // 重置为-1
	LoopCount = 0;
	ChildIterator.Reset();
}

bool FSequenceIterator::HasNext() const
{
	if (ChildIterator.IsValid() && ChildIterator->HasNext())
	{
		return true;
	}

	if (CurrentIndex + 1 < Sequence->Entries.Num())
	{
		return true;
	}

	if (Sequence->bLoopInfinitely || LoopCount < Sequence->MaxLoops - 1)
	{
		return true;
	}

	return false;
}

FString FSequenceIterator::GetDetailString() const
{
	if (!Sequence)
	{
		return TEXT("NULL");
	}

	// Show current index and child type
	if (CurrentIndex >= 0 && Sequence->Entries.Num() > 0)
	{
		FString Result = FString::Printf(TEXT("[%d/%d]"), CurrentIndex + 1, Sequence->Entries.Num());

		// If we have an active child iterator, show its type
		if (ChildIterator.IsValid())
		{
			Result += TEXT(" -> ");
			Result += ChildIterator->GetDebugString();
		}

		return Result;
	}

	return TEXT("[Initializing]");
}

//////////////////////////////////////////////////////////////////////////
// FRandomListIterator
//////////////////////////////////////////////////////////////////////////

FRandomListIterator::FRandomListIterator(const UWorkspotRandomList* InRandomList)
	: RandomList(InRandomList)
	, CurrentPlayIndex(-1)  // 初始化为-1，第一次++变成0
	, ChildIterator(nullptr)
	, PauseRemainingTime(0.0f)
	, bInPause(false)
{
}

bool FRandomListIterator::Next(FWorkspotContext& Context)
{
	if (!RandomList || RandomList->Entries.Num() == 0)
	{
		return false;
	}

	// First time: select random entries
	if (SelectedIndices.Num() == 0)
	{
		SelectRandomEntries(Context);
	}

	// Handle pause between clips
	if (bInPause)
	{
		// Note: Pause handling would need delta time, which we don't have here
		// This should be handled by the component
		bInPause = false;
	}

	// Try to advance current child iterator
	if (ChildIterator.IsValid() && ChildIterator->Next(Context))
	{
		return true;
	}

	// Move to next selected entry
	CurrentPlayIndex++;

	if (CurrentPlayIndex >= SelectedIndices.Num())
	{
		// Finished all selected clips
		return false;
	}

	// Insert pause if needed
	if (RandomList->PauseDuration.X > 0.0f || RandomList->PauseDuration.Y > 0.0f)
	{
		PauseRemainingTime = FMath::FRandRange(
			RandomList->PauseDuration.X,
			RandomList->PauseDuration.Y);
		bInPause = true;
	}

	// Create iterator for next entry
	int32 EntryIndex = SelectedIndices[CurrentPlayIndex];
	const UWorkspotEntry* Entry = RandomList->Entries[EntryIndex];
	if (!Entry)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("  🎲 RandomList - Entry at index %d is NULL, skipping"), EntryIndex);
		return Next(Context); // Skip to next
	}

	UE_LOG(LogWorkspot, Log, TEXT("  🎲 RandomList - Playing entry [%d/%d]: Index %d (%s)"),
		CurrentPlayIndex + 1, SelectedIndices.Num(), EntryIndex, *Entry->GetClass()->GetName());

	ChildIterator = Entry->CreateIterator(Context);
	if (!ChildIterator.IsValid())
	{
		return false;
	}

	return ChildIterator->Next(Context);
}

bool FRandomListIterator::GetData(FWorkspotEntryData& OutData) const
{
	if (ChildIterator.IsValid())
	{
		return ChildIterator->GetData(OutData);
	}
	return false;
}

void FRandomListIterator::Reset()
{
	SelectedIndices.Empty();
	CurrentPlayIndex = -1;  // 重置为-1
	ChildIterator.Reset();
	PauseRemainingTime = 0.0f;
	bInPause = false;
}

bool FRandomListIterator::HasNext() const
{
	if (ChildIterator.IsValid() && ChildIterator->HasNext())
	{
		return true;
	}

	return CurrentPlayIndex + 1 < SelectedIndices.Num();
}

FString FRandomListIterator::GetDetailString() const
{
	if (!RandomList)
	{
		return TEXT("NULL");
	}

	// Show current play index and selected count
	if (SelectedIndices.Num() > 0 && CurrentPlayIndex >= 0)
	{
		FString Result = FString::Printf(TEXT("[%d/%d]"), CurrentPlayIndex + 1, SelectedIndices.Num());

		// If we have an active child iterator, show its type
		if (ChildIterator.IsValid())
		{
			Result += TEXT(" -> ");
			Result += ChildIterator->GetDebugString();
		}

		return Result;
	}

	return TEXT("[Initializing]");
}

void FRandomListIterator::SelectRandomEntries(FWorkspotContext& Context)
{
	int32 NumToSelect = FMath::RandRange(RandomList->MinClips, RandomList->MaxClips);
	NumToSelect = FMath::Min(NumToSelect, RandomList->Entries.Num());

	SelectedIndices.Empty();

	UE_LOG(LogWorkspot, Log, TEXT("  🎲 RandomList - Selecting %d clips from %d entries"),
		NumToSelect, RandomList->Entries.Num());

	for (int32 i = 0; i < NumToSelect; ++i)
	{
		int32 SelectedIndex = ChooseWeightedRandom(RandomList->Weights, Context.RandomGen);
		SelectedIndices.Add(SelectedIndex);

		UE_LOG(LogWorkspot, Log, TEXT("     [%d] Selected Entry Index: %d (Weight: %.1f)"),
			i, SelectedIndex,
			RandomList->Weights.IsValidIndex(SelectedIndex) ? RandomList->Weights[SelectedIndex] : 0.0f);
	}
}

int32 FRandomListIterator::ChooseWeightedRandom(const TArray<float>& Weights, FRandomStream& Random) const
{
	if (Weights.Num() == 0)
	{
		return 0;
	}

	// Calculate total weight
	float TotalWeight = 0.0f;
	for (float Weight : Weights)
	{
		TotalWeight += FMath::Max(Weight, 0.0f);
	}

	if (TotalWeight <= 0.0f)
	{
		// All weights are zero, pick randomly
		return Random.RandRange(0, Weights.Num() - 1);
	}

	// Pick a random value
	float RandomValue = Random.FRandRange(0.0f, TotalWeight);

	// Find which index it falls into
	float AccumulatedWeight = 0.0f;
	for (int32 i = 0; i < Weights.Num(); ++i)
	{
		AccumulatedWeight += FMath::Max(Weights[i], 0.0f);
		if (RandomValue < AccumulatedWeight)  // Use < not <=
		{
			return i;
		}
	}

	return Weights.Num() - 1;
}

//////////////////////////////////////////////////////////////////////////
// FSelectorIterator (⭐ Core Innovation)
//////////////////////////////////////////////////////////////////////////

FSelectorIterator::FSelectorIterator(const UWorkspotSelector* InSelector, UWorkspotTree* InTree)
	: Selector(InSelector)
	, Tree(InTree)
	, CurrentSelection(-1)
	, ChildIterator(nullptr)
	, bFirstRun(true)
	, bPlayingTransition(false)
{
}

bool FSelectorIterator::Next(FWorkspotContext& Context)
{
	if (!Selector || Selector->Entries.Num() == 0)
	{
		return false;
	}

	// If playing transition, return it once then continue
	if (bPlayingTransition)
	{
		bPlayingTransition = false;
		return true; // Transition data is already in TransitionData
	}

	// Try to advance current child iterator
	if (!bFirstRun && ChildIterator.IsValid() && ChildIterator->Next(Context))
	{
		return true;
	}

	// Select new entry (weighted random)
	bFirstRun = false;
	CurrentSelection = ChooseWeightedRandom(Selector->Weights, Context.RandomGen);

	const UWorkspotEntry* SelectedEntry = Selector->Entries[CurrentSelection];
	if (!SelectedEntry)
	{
		UE_LOG(LogWorkspot, Error, TEXT("Selector has null entry at selected index %d"), CurrentSelection);
		return false;
	}

	// ⭐ IdleGuard: Check for idle transition
	if (Context.CurrentIdle != SelectedEntry->IdleAnim)
	{
		UE_LOG(LogWorkspot, Log, TEXT("Selector: Idle transition %s -> %s"),
			*Context.CurrentIdle.ToString(),
			*SelectedEntry->IdleAnim.ToString());

		// Try to insert transition animation
		if (TryInsertTransitionAnim(Context.CurrentIdle, SelectedEntry->IdleAnim, Context))
		{
			// Transition inserted, update context
			Context.PreviousIdle = Context.CurrentIdle;
			Context.CurrentIdle = SelectedEntry->IdleAnim;

			// Return transition this iteration, child entry next iteration
			return true;
		}
		else
		{
			// No transition animation found, use fallback
			UE_LOG(LogWorkspot, Warning, TEXT("No transition animation: %s -> %s, using fallback"),
				*Context.CurrentIdle.ToString(),
				*SelectedEntry->IdleAnim.ToString());

			// Update idle state without transition
			Context.CurrentIdle = SelectedEntry->IdleAnim;
		}
	}

	// Create child iterator
	ChildIterator = SelectedEntry->CreateIterator(Context);
	if (!ChildIterator.IsValid())
	{
		return false;
	}

	return ChildIterator->Next(Context);
}

bool FSelectorIterator::GetData(FWorkspotEntryData& OutData) const
{
	if (bPlayingTransition)
	{
		OutData = TransitionData;
		return true;
	}

	if (ChildIterator.IsValid())
	{
		return ChildIterator->GetData(OutData);
	}

	return false;
}

void FSelectorIterator::Reset()
{
	CurrentSelection = -1;
	ChildIterator.Reset();
	bFirstRun = true;
	bPlayingTransition = false;
}

bool FSelectorIterator::HasNext() const
{
	// Selector can always select a new entry
	return true;
}

FString FSelectorIterator::GetDetailString() const
{
	if (!Selector)
	{
		return TEXT("NULL");
	}

	FString Result;

	// If playing transition, indicate it
	if (bPlayingTransition)
	{
		Result = TEXT("[Transition]");
	}
	// If we have an active child iterator, show its type
	else if (ChildIterator.IsValid())
	{
		Result = TEXT("-> ");
		Result += ChildIterator->GetDebugString();
	}

	return Result;
}

int32 FSelectorIterator::ChooseWeightedRandom(const TArray<float>& Weights, FRandomStream& Random) const
{
	if (Weights.Num() == 0)
	{
		return 0;
	}

	float TotalWeight = 0.0f;
	for (float Weight : Weights)
	{
		TotalWeight += FMath::Max(Weight, 0.0f);
	}

	if (TotalWeight <= 0.0f)
	{
		return Random.RandRange(0, Weights.Num() - 1);
	}

	float RandomValue = Random.FRandRange(0.0f, TotalWeight);
	float AccumulatedWeight = 0.0f;

	for (int32 i = 0; i < Weights.Num(); ++i)
	{
		AccumulatedWeight += FMath::Max(Weights[i], 0.0f);
		if (RandomValue < AccumulatedWeight)  // Use < not <=
		{
			return i;
		}
	}

	return Weights.Num() - 1;
}

bool FSelectorIterator::TryInsertTransitionAnim(FName FromIdle, FName ToIdle, FWorkspotContext& Context)
{
	if (!Tree)
	{
		return false;
	}

	// Look up transition animation in tree
	UAnimMontage* TransitionMontage = Tree->FindTransitionAnim(FromIdle, ToIdle);
	if (!TransitionMontage)
	{
		return false;
	}

	// Set up transition data
	TransitionData.AnimMontage = TransitionMontage;
	TransitionData.IdleAnim = ToIdle; // Transition ends in new idle
	TransitionData.BlendInTime = 0.2f;
	TransitionData.BlendOutTime = 0.2f;
	TransitionData.bIsValid = true;

	bPlayingTransition = true;
	return true;
}
