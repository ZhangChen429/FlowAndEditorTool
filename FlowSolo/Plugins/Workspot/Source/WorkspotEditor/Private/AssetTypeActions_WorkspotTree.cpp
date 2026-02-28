// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetTypeActions_WorkspotTree.h"
#include "WorkspotTree.h"
#include "WorkspotEditorToolkit.h"
#include "ToolMenuSection.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_WorkspotTree"

FText FAssetTypeActions_WorkspotTree::GetName() const
{
	return LOCTEXT("AssetTypeActions_WorkspotTree_Name", "Workspot Tree");
}

FColor FAssetTypeActions_WorkspotTree::GetTypeColor() const
{
	// Teal color - matches behavior tree style
	return FColor(0, 162, 162);
}

UClass* FAssetTypeActions_WorkspotTree::GetSupportedClass() const
{
	return UWorkspotTree::StaticClass();
}

uint32 FAssetTypeActions_WorkspotTree::GetCategories()
{
	return EAssetTypeCategories::Animation;
}

void FAssetTypeActions_WorkspotTree::OpenAssetEditor(
	const TArray<UObject*>& InObjects,
	TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Obj : InObjects)
	{
		if (UWorkspotTree* Tree = Cast<UWorkspotTree>(Obj))
		{
			TSharedRef<FWorkspotEditorToolkit> EditorToolkit = MakeShared<FWorkspotEditorToolkit>();
			EditorToolkit->InitWorkspotEditor(Mode, EditWithinLevelEditor, Tree);
		}
	}
}

bool FAssetTypeActions_WorkspotTree::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}

void FAssetTypeActions_WorkspotTree::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	TArray<TWeakObjectPtr<UWorkspotTree>> Trees;
	for (UObject* Obj : InObjects)
	{
		if (UWorkspotTree* Tree = Cast<UWorkspotTree>(Obj))
		{
			Trees.Add(Tree);
		}
	}

	if (Trees.Num() > 0)
	{
		Section.AddMenuEntry(
			"WorkspotTree_Validate",
			LOCTEXT("WorkspotTree_Validate", "Validate"),
			LOCTEXT("WorkspotTree_ValidateTooltip", "Validates the workspot tree and reports any errors"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &FAssetTypeActions_WorkspotTree::ExecuteValidate, Trees))
		);
	}
}

void FAssetTypeActions_WorkspotTree::ExecuteValidate(TArray<TWeakObjectPtr<UWorkspotTree>> Objects)
{
	for (TWeakObjectPtr<UWorkspotTree> TreePtr : Objects)
	{
		if (UWorkspotTree* Tree = TreePtr.Get())
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
	}
}

#undef LOCTEXT_NAMESPACE
