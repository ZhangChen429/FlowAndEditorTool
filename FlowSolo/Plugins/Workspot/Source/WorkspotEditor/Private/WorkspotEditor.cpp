// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkspotEditor.h"
#include "AssetTypeActions_WorkspotTree.h"
#include "WorkspotTreeDetails.h"
#include "WorkspotTree.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FWorkspotEditorModule"

void FWorkspotEditorModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("WorkspotEditor: Module starting up"));

	RegisterAssetTools();
	RegisterDetailsCustomizations();
}

void FWorkspotEditorModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("WorkspotEditor: Module shutting down"));

	UnregisterDetailsCustomizations();
	UnregisterAssetTools();
}

void FWorkspotEditorModule::RegisterAssetTools()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Register WorkspotTree asset type actions
	TSharedPtr<FAssetTypeActions_WorkspotTree> WorkspotTreeActions = MakeShared<FAssetTypeActions_WorkspotTree>();
	AssetTools.RegisterAssetTypeActions(WorkspotTreeActions.ToSharedRef());
	CreatedAssetTypeActions.Add(WorkspotTreeActions);

	UE_LOG(LogTemp, Log, TEXT("WorkspotEditor: Registered asset type actions"));
}

void FWorkspotEditorModule::UnregisterAssetTools()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

		for (TSharedPtr<FAssetTypeActions_WorkspotTree>& Action : CreatedAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
		}
	}

	CreatedAssetTypeActions.Empty();
}

void FWorkspotEditorModule::RegisterDetailsCustomizations()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Register WorkspotTree details customization
	PropertyModule.RegisterCustomClassLayout(
		UWorkspotTree::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FWorkspotTreeDetails::MakeInstance)
	);

	UE_LOG(LogTemp, Log, TEXT("WorkspotEditor: Registered details customizations"));
}

void FWorkspotEditorModule::UnregisterDetailsCustomizations()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(UWorkspotTree::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWorkspotEditorModule, WorkspotEditor)
