
#include "Asset/TouchCameraAsset_AssetEditor_ViewUtility.h"
#include "FactoryAndMode/FTouchCameraAssetToolkit.h"
#include "Slate/TouchCameraViewer_Viewport.h"



#define LOCTEXT_NAMESPACE "FTouchCameraAsset_AssetEditor_ViewUtility"


 FTouchCameraAsset_AssetEditor_ViewUtility::FTouchCameraAsset_AssetEditor_ViewUtility(
	TSharedRef<FTouchCameraAssetToolkit>  InToolkit): FTouchCameraAsset_AssetEditor_BaseUtility(InToolkit)
{

}

 FTouchCameraAsset_AssetEditor_ViewUtility::~FTouchCameraAsset_AssetEditor_ViewUtility()
{
 	if (ViewportWidget.IsValid())
 	{
 		ViewportWidget.Reset();
 	}
}

 void FTouchCameraAsset_AssetEditor_ViewUtility::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager,
	const TSharedRef<FWorkspaceItem> InMenuCategory)
{
 	
 	InTabManager->RegisterTabSpawner(
		 	FTouchCameraEditorTabs::ViewportID,
		 FOnSpawnTab::CreateRaw(this, &FTouchCameraAsset_AssetEditor_ViewUtility::SpawnTab_Selector))
			 .SetDisplayName(LOCTEXT("ViewTabKK", "KKKKKKK"))
			 .SetGroup(InMenuCategory)
			 .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}
TSharedRef<SDockTab> FTouchCameraAsset_AssetEditor_ViewUtility::SpawnTab_Selector(const FSpawnTabArgs& Args) 
{
	if (Toolkit.Pin()->GetTouchCameraObject())
	{
		ViewportWidget	=SNew( STouchCameraViewer_Viewport)
					.TouchCameraToolkit(Toolkit.Pin());
		auto Tab = DoCreateTab(ViewportWidget.ToSharedRef());
		return Tab;
	}
 	return SNew(SDockTab);
 }
 void FTouchCameraAsset_AssetEditor_ViewUtility::UnregisterTabSpawners(
	const TSharedRef<FTabManager>& InTabManager)
{
 	InTabManager->UnregisterTabSpawner(FTouchCameraEditorTabs::ViewportID);
}


#undef LOCTEXT_NAMESPACE
