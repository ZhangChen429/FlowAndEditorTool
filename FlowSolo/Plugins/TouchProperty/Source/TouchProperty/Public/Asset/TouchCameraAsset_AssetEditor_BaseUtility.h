#pragma once

class FTouchCameraAssetToolkit;

class TOUCHPROPERTY_API FTouchCameraAsset_AssetEditor_BaseUtility
{
public:
	static FName GetUtilityTypeNameStatic() { return FName("FTouchCameraAsset_AssetEditor_BaseUtility"); }
	explicit FTouchCameraAsset_AssetEditor_BaseUtility(TSharedRef<FTouchCameraAssetToolkit> InToolkit);
	virtual ~FTouchCameraAsset_AssetEditor_BaseUtility();


	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager, const TSharedRef<FWorkspaceItem> InMenuCategory) {}
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) {}
	
protected:
	/** Toolkit */
	TWeakPtr<FTouchCameraAssetToolkit>Toolkit;
	//FTouchCameraAssetToolkit* Toolkit;

	static TSharedRef<SDockTab> DoCreateTab(const TSharedRef<SWidget>& Content);
};


