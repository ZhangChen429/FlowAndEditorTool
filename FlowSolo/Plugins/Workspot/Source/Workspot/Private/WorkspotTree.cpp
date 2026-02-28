// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkspotTree.h"
#include "WorkspotEntry.h"
#include "Workspot.h"
#include "Animation/AnimMontage.h"
#include "Animation/Skeleton.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

UWorkspotTree::UWorkspotTree()
{
	DefaultTransitionBlendTime = 0.3f;
	InertializationDurationEnter = 0.5f;
	InertializationDurationExitNatural = 0.5f;
	InertializationDurationExitForced = 0.2f;
}

UAnimMontage* UWorkspotTree::FindTransitionAnim(FName FromIdle, FName ToIdle) const
{
	// 1. Check custom transitions first (higher priority)
	for (const FWorkspotTransitionAnim& Transition : CustomTransitionAnims)
	{
		if (Transition.FromIdle == FromIdle && Transition.ToIdle == ToIdle)
		{
			return Transition.TransitionMontage;
		}
	}

	// 2. Check naming convention: "FromIdle__2__ToIdle"
	// Note: This would require searching through animation assets
	// For now, only custom transitions are supported
	// TODO: Implement animation asset search by name

	return nullptr;
}

bool UWorkspotTree::IsValid() const
{
	return RootEntry != nullptr && WorkspotSkeleton != nullptr;
}

void UWorkspotTree::GetReferencedAnimations(TArray<UAnimMontage*>& OutAnimations) const
{
	if (!RootEntry)
	{
		return;
	}

	// Traverse tree and collect all animation montages
	RootEntry->ForEachNode([&OutAnimations](const UWorkspotEntry* Entry)
	{
		if (const UWorkspotAnimClip* AnimClip = Cast<UWorkspotAnimClip>(Entry))
		{
			if (AnimClip->AnimMontage)
			{
				OutAnimations.AddUnique(AnimClip->AnimMontage);
			}
		}
		else if (const UWorkspotEntryAnim* EntryAnim = Cast<UWorkspotEntryAnim>(Entry))
		{
			if (EntryAnim->AnimMontage)
			{
				OutAnimations.AddUnique(EntryAnim->AnimMontage);
			}
		}
		else if (const UWorkspotExitAnim* ExitAnim = Cast<UWorkspotExitAnim>(Entry))
		{
			if (ExitAnim->AnimMontage)
			{
				OutAnimations.AddUnique(ExitAnim->AnimMontage);
			}
		}
	});

	// Add transition animations
	for (const FWorkspotTransitionAnim& Transition : CustomTransitionAnims)
	{
		if (Transition.TransitionMontage)
		{
			OutAnimations.AddUnique(Transition.TransitionMontage);
		}
	}
}

int32 UWorkspotTree::GetEntryCount() const
{
	if (!RootEntry)
	{
		return 0;
	}

	int32 Count = 0;
	RootEntry->ForEachNode([&Count](const UWorkspotEntry* Entry)
	{
		Count++;
	});

	return Count;
}

#if WITH_EDITOR
EDataValidationResult UWorkspotTree::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = Super::IsDataValid(ValidationErrors);

	if (!RootEntry)
	{
		ValidationErrors.Add(FText::FromString(TEXT("WorkspotTree has no root entry")));
		Result = EDataValidationResult::Invalid;
	}
	else
	{
		// Check if root is a container
		if (!Cast<UWorkspotSequence>(RootEntry) &&
			!Cast<UWorkspotRandomList>(RootEntry) &&
			!Cast<UWorkspotSelector>(RootEntry))
		{
			ValidationErrors.Add(FText::FromString(TEXT("WorkspotTree root must be a container (Sequence, RandomList, or Selector)")));
			Result = EDataValidationResult::Invalid;
		}
	}

	if (!WorkspotSkeleton)
	{
		ValidationErrors.Add(FText::FromString(TEXT("WorkspotTree has no skeleton assigned")));
		Result = EDataValidationResult::Invalid;
	}

	// Validate all animations belong to the skeleton
	TArray<UAnimMontage*> ReferencedAnims;
	GetReferencedAnimations(ReferencedAnims);

	for (UAnimMontage* Anim : ReferencedAnims)
	{
		if (Anim && Anim->GetSkeleton() != WorkspotSkeleton)
		{
			ValidationErrors.Add(FText::Format(
				FText::FromString(TEXT("Animation '{0}' uses different skeleton than WorkspotTree")),
				FText::FromString(Anim->GetName())));
			Result = EDataValidationResult::Invalid;
		}
	}

	// Validate transition animations
	for (const FWorkspotTransitionAnim& Transition : CustomTransitionAnims)
	{
		if (Transition.FromIdle.IsNone() || Transition.ToIdle.IsNone())
		{
			ValidationErrors.Add(FText::FromString(TEXT("Transition has empty FromIdle or ToIdle")));
			Result = EDataValidationResult::Invalid;
		}

		if (!Transition.TransitionMontage)
		{
			ValidationErrors.Add(FText::Format(
				FText::FromString(TEXT("Transition {0} -> {1} has no animation")),
				FText::FromName(Transition.FromIdle),
				FText::FromName(Transition.ToIdle)));
			Result = EDataValidationResult::Invalid;
		}
	}

	// Validate props
	for (const FWorkspotGlobalProp& Prop : GlobalProps)
	{
		if (Prop.PropId.IsNone())
		{
			ValidationErrors.Add(FText::FromString(TEXT("Prop has no ID")));
			Result = EDataValidationResult::Invalid;
		}

		if (!Prop.PropClass)
		{
			ValidationErrors.Add(FText::Format(
				FText::FromString(TEXT("Prop '{0}' has no actor class")),
				FText::FromName(Prop.PropId)));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}

void UWorkspotTree::RegenerateEntryIds()
{
	if (!RootEntry)
	{
		return;
	}

	RootEntry->ForEachNode([](const UWorkspotEntry* Entry)
	{
		if (UWorkspotEntry* MutableEntry = const_cast<UWorkspotEntry*>(Entry))
		{
			if (!MutableEntry->EntryId.IsValid())
			{
				MutableEntry->EntryId = FGuid::NewGuid();
			}
		}
	});
}
#endif
