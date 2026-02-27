
#include "Asset/TouchCameraAsset_AssetEditor_DataUtility.h"
#include "FactoryAndMode/FTouchCameraAssetToolkit.h"
#include "TouchAsset/TouchCameraAsset.h"
#define LOCTEXT_NAMESPACE "TouchCameraAsset_AssetEditor_DataUtility"

 FTouchCameraAsset_AssetEditor_DataUtility::FTouchCameraAsset_AssetEditor_DataUtility(
	TSharedRef<FTouchCameraAssetToolkit>  InToolkit): FTouchCameraAsset_AssetEditor_BaseUtility(InToolkit)
{
}

 FTouchCameraAsset_AssetEditor_DataUtility::~FTouchCameraAsset_AssetEditor_DataUtility()
{
 	if (CustomDetailsViewContent.IsValid())
 	{
 		CustomDetailsViewContent.Reset();
 	}
}

void FTouchCameraAsset_AssetEditor_DataUtility::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
 {
 	InTabManager->UnregisterTabSpawner( FTouchCameraEditorTabs::DetailsID);
 }

void FTouchCameraAsset_AssetEditor_DataUtility::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager,
	const TSharedRef<FWorkspaceItem> InMenuCategory)
{
 	InTabManager->RegisterTabSpawner(
	 FTouchCameraEditorTabs::DetailsID,
	 FOnSpawnTab::CreateRaw(this, &FTouchCameraAsset_AssetEditor_DataUtility::SpawnTab_Details))
		 .SetDisplayName(LOCTEXT("TouchTab", "HHHHHH"))
		 .SetGroup(InMenuCategory)
		 .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

TSharedRef<SDockTab> FTouchCameraAsset_AssetEditor_DataUtility::SpawnTab_Details(const FSpawnTabArgs& Args)
 {
 	// Create Custom DetailsView
 	FDetailsViewArgs DetailsViewArgs;
	 {
 		DetailsViewArgs.bUpdatesFromSelection = false;
 		DetailsViewArgs.bLockable = false;
 		DetailsViewArgs.bAllowSearch = true;
 		DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::ObjectsUseNameArea;
 		DetailsViewArgs.bHideSelectionTip = false;	
	 }

 	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
 	CustomDetailsViewContent = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
 	CustomDetailsViewContent->SetObject(Toolkit.Pin()->GetTouchCameraObject());
 	
 	const auto DetailsTab = SNew(SDockTab)
	 [
	 	SNew(SVerticalBox)
  	 	+ SVerticalBox::Slot()
  	 	.AutoHeight()
  	 	[
			   CustomDetailsViewContent.ToSharedRef() 
  	 	]
  	 	+ SVerticalBox::Slot()
  	 	.AutoHeight()
  	 	[
	   		SNew(SButton)
	   			.Text(FText::FromString(TEXT("Click Me"))) 
	   			.OnClicked_Lambda([]()
	   			{
					UE_LOG(LogTemp, Warning, TEXT("Button clicked!"));
					return FReply::Handled();
	   			})
		]
	 ];
	
 	DetailsTab->ForceVolatile(true);
	
 	return DetailsTab;
 }

#undef LOCTEXT_NAMESPACE
