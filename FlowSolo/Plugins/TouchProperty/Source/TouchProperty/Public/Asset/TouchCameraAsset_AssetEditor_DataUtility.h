#pragma once
#include "TouchCameraAsset_AssetEditor_BaseUtility.h"

class FTouchCameraAssetToolkit;

class TOUCHPROPERTY_API FTouchCameraAsset_AssetEditor_DataUtility :public FTouchCameraAsset_AssetEditor_BaseUtility
{
public:
	FTouchCameraAsset_AssetEditor_DataUtility(TSharedRef<FTouchCameraAssetToolkit> InToolkit);
	virtual ~FTouchCameraAsset_AssetEditor_DataUtility() override;
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager, const TSharedRef<FWorkspaceItem> InMenuCategory) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	
	TSharedPtr<IDetailsView> CustomDetailsViewContent;
};


