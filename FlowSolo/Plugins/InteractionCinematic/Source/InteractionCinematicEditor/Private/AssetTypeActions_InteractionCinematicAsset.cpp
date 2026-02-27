#include "AssetTypeActions_InteractionCinematicAsset.h"
#include "InteractionCinematicAsset.h"
#include "InteractionCinematicAssetEditorToolkit.h"
#include "InteractionCinematicEditor.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_InteractionCinematicAsset"

FText FAssetTypeActions_InteractionCinematicAsset::GetName() const
{
	return LOCTEXT("AssetTypeActions_InteractionCinematicAsset", "Interaction Cinematic");
}

FColor FAssetTypeActions_InteractionCinematicAsset::GetTypeColor() const
{
	return FColor(255, 150, 100); // Orange color
}

UClass* FAssetTypeActions_InteractionCinematicAsset::GetSupportedClass() const
{
	return UInteractionCinematicAsset::StaticClass();
}

void FAssetTypeActions_InteractionCinematicAsset::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UInteractionCinematicAsset* Asset = Cast<UInteractionCinematicAsset>(*ObjIt))
		{
			TSharedRef<FInteractionCinematicAssetEditorToolkit> EditorToolkit = MakeShared<FInteractionCinematicAssetEditorToolkit>();
			EditorToolkit->Initialize(Asset, Mode, EditWithinLevelEditor);
		}
	}
}

uint32 FAssetTypeActions_InteractionCinematicAsset::GetCategories()
{
	return FInteractionCinematicEditorModule::GetAssetCategory();
}

#undef LOCTEXT_NAMESPACE
