#include "InteractionCinematicEditor.h"
#include "AssetTypeActions_InteractionCinematicAsset.h"
#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "FInteractionCinematicEditorModule"

EAssetTypeCategories::Type FInteractionCinematicEditorModule::InteractionCinematicAssetCategory;

void FInteractionCinematicEditorModule::StartupModule()
{
	// Register asset type actions
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Register a custom asset category
	InteractionCinematicAssetCategory = AssetTools.RegisterAdvancedAssetCategory(
		FName(TEXT("InteractionCinematic")),
		LOCTEXT("InteractionCinematicAssetCategory", "Interaction Cinematic")
	);

	InteractionCinematicAssetTypeActions = MakeShared<FAssetTypeActions_InteractionCinematicAsset>();
	AssetTools.RegisterAssetTypeActions(InteractionCinematicAssetTypeActions.ToSharedRef());
}

void FInteractionCinematicEditorModule::ShutdownModule()
{
	// Unregister asset type actions
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

		if (InteractionCinematicAssetTypeActions.IsValid())
		{
			AssetTools.UnregisterAssetTypeActions(InteractionCinematicAssetTypeActions.ToSharedRef());
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FInteractionCinematicEditorModule, InteractionCinematicEditor)
