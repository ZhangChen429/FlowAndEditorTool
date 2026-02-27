
#include "Asset/TouchCameraAsset_AssetEditor_BaseUtility.h"

FTouchCameraAsset_AssetEditor_BaseUtility::FTouchCameraAsset_AssetEditor_BaseUtility(
	TSharedRef<FTouchCameraAssetToolkit> InToolkit): Toolkit(InToolkit)
{
	
}

FTouchCameraAsset_AssetEditor_BaseUtility::~FTouchCameraAsset_AssetEditor_BaseUtility()
{
	//if (Toolkit.IsUnique())
	//{
	//	Toolkit.Reset();
	//}

}
TSharedRef<SDockTab> FTouchCameraAsset_AssetEditor_BaseUtility::DoCreateTab(const TSharedRef<SWidget>& Content)
{
	return SNew(SDockTab)
		.TabRole(PanelTab)
		.CanEverClose(false)
		.ForceVolatile(true)
		[
			Content	
		];
}
