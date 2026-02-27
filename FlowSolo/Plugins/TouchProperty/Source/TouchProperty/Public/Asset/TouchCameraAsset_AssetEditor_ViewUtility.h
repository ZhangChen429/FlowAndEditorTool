#pragma once
#include "TouchCameraAsset_AssetEditor_BaseUtility.h"

class STouchCameraViewer_Viewport;
class UCustomDetailsViewContent;

class TOUCHPROPERTY_API FTouchCameraAsset_AssetEditor_ViewUtility:public FTouchCameraAsset_AssetEditor_BaseUtility
{
	public:
	explicit FTouchCameraAsset_AssetEditor_ViewUtility(TSharedRef<FTouchCameraAssetToolkit>  InToolkit);
	virtual ~FTouchCameraAsset_AssetEditor_ViewUtility() override;
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager, const TSharedRef<FWorkspaceItem> InMenuCategory) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	TSharedRef<SDockTab> SpawnTab_Selector(const FSpawnTabArgs& Args) ;


	TSharedPtr<STouchCameraViewer_Viewport> ViewportWidget;
private:


};


