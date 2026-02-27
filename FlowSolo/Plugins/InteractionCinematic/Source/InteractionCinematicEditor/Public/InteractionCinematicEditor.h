#pragma once

#include "CoreMinimal.h"
#include "AssetTypeCategories.h"
#include "Modules/ModuleManager.h"

class FInteractionCinematicEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Get the asset category for Interaction Cinematic assets */
	static EAssetTypeCategories::Type GetAssetCategory() { return InteractionCinematicAssetCategory; }

private:
	/** Handle to the registered AssetTypeActions */
	TSharedPtr<class FAssetTypeActions_InteractionCinematicAsset> InteractionCinematicAssetTypeActions;

	/** The asset category for Interaction Cinematic assets */
	static EAssetTypeCategories::Type InteractionCinematicAssetCategory;
};
