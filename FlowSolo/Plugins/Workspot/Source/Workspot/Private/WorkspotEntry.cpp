// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkspotEntry.h"
#include "WorkspotIterator.h"
#include "Workspot.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

//////////////////////////////////////////////////////////////////////////
// UWorkspotEntry
//////////////////////////////////////////////////////////////////////////

UWorkspotEntry::UWorkspotEntry()
{
	EntryId = FGuid::NewGuid();
	IdleAnim = "stand";
	Flags = 0;
}

#if WITH_EDITOR
EDataValidationResult UWorkspotEntry::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = Super::IsDataValid(ValidationErrors);

	if (!EntryId.IsValid())
	{
		ValidationErrors.Add(FText::FromString(TEXT("Entry has invalid GUID")));
		Result = EDataValidationResult::Invalid;
	}

	if (IdleAnim.IsNone())
	{
		ValidationErrors.Add(FText::FromString(TEXT("Entry has no idle anim specified")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

//////////////////////////////////////////////////////////////////////////
// UWorkspotAnimClip
//////////////////////////////////////////////////////////////////////////

TSharedPtr<FWorkspotIterator> UWorkspotAnimClip::CreateIterator(FWorkspotContext& Context) const
{
	return MakeShared<FAnimClipIterator>(this);
}

#if WITH_EDITOR
EDataValidationResult UWorkspotAnimClip::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = Super::IsDataValid(ValidationErrors);

	if (!AnimMontage)
	{
		ValidationErrors.Add(FText::FromString(TEXT("AnimClip has no animation montage assigned")));
		Result = EDataValidationResult::Invalid;
	}

	if (BlendInTime < 0.0f || BlendInTime > 2.0f)
	{
		ValidationErrors.Add(FText::FromString(TEXT("AnimClip blend in time out of range [0, 2]")));
		Result = EDataValidationResult::Invalid;
	}

	if (BlendOutTime < 0.0f || BlendOutTime > 2.0f)
	{
		ValidationErrors.Add(FText::FromString(TEXT("AnimClip blend out time out of range [0, 2]")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

//////////////////////////////////////////////////////////////////////////
// UWorkspotEntryAnim
//////////////////////////////////////////////////////////////////////////

TSharedPtr<FWorkspotIterator> UWorkspotEntryAnim::CreateIterator(FWorkspotContext& Context) const
{
	return MakeShared<FEntryAnimIterator>(this);
}

//////////////////////////////////////////////////////////////////////////
// UWorkspotExitAnim
//////////////////////////////////////////////////////////////////////////

TSharedPtr<FWorkspotIterator> UWorkspotExitAnim::CreateIterator(FWorkspotContext& Context) const
{
	return MakeShared<FExitAnimIterator>(this);
}

//////////////////////////////////////////////////////////////////////////
// UWorkspotSequence
//////////////////////////////////////////////////////////////////////////

TSharedPtr<FWorkspotIterator> UWorkspotSequence::CreateIterator(FWorkspotContext& Context) const
{
	return MakeShared<FSequenceIterator>(this);
}

#if WITH_EDITOR
EDataValidationResult UWorkspotSequence::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = Super::IsDataValid(ValidationErrors);

	if (Entries.Num() == 0)
	{
		ValidationErrors.Add(FText::FromString(TEXT("Sequence has no entries")));
		Result = EDataValidationResult::Invalid;
	}

	// Validate all children
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		if (!Entries[i])
		{
			ValidationErrors.Add(FText::Format(
				FText::FromString(TEXT("Sequence entry {0} is null")),
				FText::AsNumber(i)));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
#endif

//////////////////////////////////////////////////////////////////////////
// UWorkspotRandomList
//////////////////////////////////////////////////////////////////////////

TSharedPtr<FWorkspotIterator> UWorkspotRandomList::CreateIterator(FWorkspotContext& Context) const
{
	return MakeShared<FRandomListIterator>(this);
}

#if WITH_EDITOR
void UWorkspotRandomList::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Auto-sync weights array with entries
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UWorkspotRandomList, Entries))
	{
		if (Weights.Num() != Entries.Num())
		{
			Weights.SetNum(Entries.Num());
			// Initialize new weights to 1.0
			for (int32 i = 0; i < Weights.Num(); ++i)
			{
				if (Weights[i] <= 0.0f)
				{
					Weights[i] = 1.0f;
				}
			}
		}
	}
}

EDataValidationResult UWorkspotRandomList::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = Super::IsDataValid(ValidationErrors);

	if (Entries.Num() == 0)
	{
		ValidationErrors.Add(FText::FromString(TEXT("RandomList has no entries")));
		Result = EDataValidationResult::Invalid;
	}

	if (Weights.Num() != Entries.Num())
	{
		ValidationErrors.Add(FText::Format(
			FText::FromString(TEXT("RandomList weights count ({0}) doesn't match entries count ({1})")),
			FText::AsNumber(Weights.Num()),
			FText::AsNumber(Entries.Num())));
		Result = EDataValidationResult::Invalid;
	}

	if (MinClips > MaxClips)
	{
		ValidationErrors.Add(FText::FromString(TEXT("RandomList MinClips > MaxClips")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

//////////////////////////////////////////////////////////////////////////
// UWorkspotSelector
//////////////////////////////////////////////////////////////////////////

TSharedPtr<FWorkspotIterator> UWorkspotSelector::CreateIterator(FWorkspotContext& Context) const
{
	// Pass the tree from context to selector iterator for transition lookup
	return MakeShared<FSelectorIterator>(this, Context.Tree.Get());
}

#if WITH_EDITOR
void UWorkspotSelector::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Auto-sync weights array with entries
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UWorkspotSelector, Entries))
	{
		if (Weights.Num() != Entries.Num())
		{
			Weights.SetNum(Entries.Num());
			for (int32 i = 0; i < Weights.Num(); ++i)
			{
				if (Weights[i] <= 0.0f)
				{
					Weights[i] = 1.0f;
				}
			}
		}
	}
}

EDataValidationResult UWorkspotSelector::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = Super::IsDataValid(ValidationErrors);

	if (Entries.Num() == 0)
	{
		ValidationErrors.Add(FText::FromString(TEXT("Selector has no entries")));
		Result = EDataValidationResult::Invalid;
	}

	if (Weights.Num() != Entries.Num())
	{
		ValidationErrors.Add(FText::Format(
			FText::FromString(TEXT("Selector weights count ({0}) doesn't match entries count ({1})")),
			FText::AsNumber(Weights.Num()),
			FText::AsNumber(Entries.Num())));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
