// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkspotTreeDetails.h"
#include "WorkspotTree.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "WorkspotTreeDetails"

TSharedRef<IDetailCustomization> FWorkspotTreeDetails::MakeInstance()
{
	return MakeShared<FWorkspotTreeDetails>();
}

void FWorkspotTreeDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	if (ObjectsBeingCustomized.Num() > 0)
	{
		CachedWorkspotTree = Cast<UWorkspotTree>(ObjectsBeingCustomized[0].Get());
	}

	// Add custom sections
	AddEntryActions(DetailBuilder);
	AddValidationSection(DetailBuilder);
}

void FWorkspotTreeDetails::AddEntryActions(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& ActionsCategory = DetailBuilder.EditCategory("Entry Actions", FText::GetEmpty(), ECategoryPriority::Important);

	ActionsCategory.AddCustomRow(LOCTEXT("RegenerateEntryIds", "Regenerate Entry IDs"))
		.WholeRowContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("RegenerateEntryIdsButton", "Regenerate Entry IDs"))
			.ToolTipText(LOCTEXT("RegenerateEntryIdsTooltip", "Regenerates unique IDs for all entries in the tree"))
			.OnClicked(this, &FWorkspotTreeDetails::OnRegenerateEntryIds)
		];
}

void FWorkspotTreeDetails::AddValidationSection(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& ValidationCategory = DetailBuilder.EditCategory("Validation", FText::GetEmpty(), ECategoryPriority::Important);

	ValidationCategory.AddCustomRow(LOCTEXT("ValidateTree", "Validate Tree"))
		.WholeRowContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("ValidateTreeButton", "Validate Tree"))
			.ToolTipText(LOCTEXT("ValidateTreeTooltip", "Validates the workspot tree and reports any errors"))
			.OnClicked(this, &FWorkspotTreeDetails::OnValidateTree)
		];
}

FReply FWorkspotTreeDetails::OnRegenerateEntryIds()
{
	if (UWorkspotTree* Tree = CachedWorkspotTree.Get())
	{
#if WITH_EDITOR
		Tree->Modify();
		Tree->RegenerateEntryIds();
		UE_LOG(LogTemp, Log, TEXT("Regenerated entry IDs for WorkspotTree '%s'"), *Tree->GetName());
#endif
	}

	return FReply::Handled();
}

FReply FWorkspotTreeDetails::OnValidateTree()
{
	if (UWorkspotTree* Tree = CachedWorkspotTree.Get())
	{
		TArray<FText> ValidationErrors;
		EDataValidationResult Result = Tree->IsDataValid(ValidationErrors);

		if (Result == EDataValidationResult::Valid)
		{
			UE_LOG(LogTemp, Log, TEXT("WorkspotTree '%s' is valid"), *Tree->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("WorkspotTree '%s' validation failed:"), *Tree->GetName());
			for (const FText& Error : ValidationErrors)
			{
				UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Error.ToString());
			}
		}
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
