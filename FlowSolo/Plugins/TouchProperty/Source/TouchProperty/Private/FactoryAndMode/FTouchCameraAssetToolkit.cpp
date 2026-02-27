
#include "FactoryAndMode/FTouchCameraAssetToolkit.h"
#include "FlowAsset.h"
#include "Asset/TouchCameraAsset_AssetEditor_DataUtility.h"
#include "Asset/TouchCameraAsset_AssetEditor_FlowGraphUtility.h"
#include "Asset/TouchCameraAsset_AssetEditor_ViewUtility.h"
#include "Graph/TouchCameraGraph.h"
#include "TouchAsset/TouchCameraAsset.h"

#define LOCTEXT_NAMESPACE "TouchCameraAssetToolkit"
const FName FTouchCameraEditorTabs::DetailsID(TEXT("Details"));
const FName FTouchCameraEditorTabs::ViewportID(TEXT("Viewport"));
const FName FTouchCameraEditorTabs::EditorGraphID(TEXT("Graph"));

const FName FTouchCameraAssetToolkit::TouchCameraAssetToolkitTabId = FName("TouchCameraAssetToolkit");
FName FTouchCameraAssetToolkit::GetToolkitFName() const
{
	return FName("TouchCameraAssetToolkit");
}

 FText FTouchCameraAssetToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("TouchCameraAssetLabel", "TouchCameraAssetBase");
}

 FString FTouchCameraAssetToolkit::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("TouchCameraAssetToolkitTabPrefix", "TouchCameraAssetToolkitPrefix ").ToString();
}

 FLinearColor FTouchCameraAssetToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0,0.5f,0.6f,0.5f);
}

 void FTouchCameraAssetToolkit::Initialize(const EToolkitMode::Type Mode,
	const TSharedPtr< IToolkitHost>& InitToolkitHost,  UTouchCameraAsset* Asset)
{
	TouchCameraObject=Asset;
	if (!TouchCameraObject->GetMainFlowAsset())
	{
		FString AssetPath = TEXT("/TouchProperty/DefaultFlow");
    
		// 当资产出现问题时保留一个固定可加载的FlowAsset
		UFlowAsset* MainFlowAsset = LoadObject<UFlowAsset>(nullptr, *AssetPath);

		TouchCameraObject->SetDefaultFlowAsset(MainFlowAsset);
	}
	// 获取当前实例的TSharedRef（不是新建实例）
	InitializeSubToolkit(Mode,InitToolkitHost,Asset->GetMainFlowAsset());
	const TSharedRef<FTouchCameraAssetToolkit> ThisRef = SharedThis(this);
	TouchCameraDataUtility= MakeShareable(new FTouchCameraAsset_AssetEditor_DataUtility(ThisRef));
	TouchCameraViewUtility= MakeShareable(new FTouchCameraAsset_AssetEditor_ViewUtility(ThisRef));
	TouchCameraGraphUtility = MakeShareable(new FTouchCameraAsset_AssetEditor_FlowGraphUtility(ThisRef));
	Initialize_SetupLayout(Mode, InitToolkitHost,Asset);

}

 void FTouchCameraAssetToolkit::RegisterTabSpawners(const TSharedRef< FTabManager>& InTabManager)
{
	WorkspaceMenuCategory=InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("TouchCameraAssetTab", "TouchCamerasAssetTab"));
	const auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();
	//InitializeSubToolkit();
	UTouchCameraGraph::CreateGraph(GetTouchCameraObject());
	TouchCameraDataUtility->RegisterTabSpawners(InTabManager,WorkspaceMenuCategoryRef);
	TouchCameraViewUtility->RegisterTabSpawners(InTabManager,WorkspaceMenuCategoryRef);
	TouchCameraGraphUtility->RegisterTabSpawners(InTabManager,WorkspaceMenuCategoryRef);
	
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
}

void FTouchCameraAssetToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	
	
	TouchCameraDataUtility->UnregisterTabSpawners(InTabManager);
	TouchCameraViewUtility->UnregisterTabSpawners(InTabManager);
	TouchCameraGraphUtility->UnregisterTabSpawners(InTabManager);
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
}
//
void FTouchCameraAssetToolkit::Initialize_SetupLayout(const EToolkitMode::Type InMode,
	const TSharedPtr<IToolkitHost>& InToolkitHost, UObject* INTouchCameraObject)
{
	if (TouchCameraDataUtility)
	{
		UE_LOG(LogTemp,Display,TEXT("FTouchCameraAssetToolkit::Initialize_SetupLayout   TouchCameraDataUtility"));
	}
	if (TouchCameraViewUtility)
	{
		UE_LOG(LogTemp,Display,TEXT("FTouchCameraAssetToolkit::Initialize_SetupLayout   TouchCameraViewUtility"));
	}
	if (TouchCameraViewUtility)
	{
		UE_LOG(LogTemp,Display,TEXT("FTouchCameraAssetToolkit::Initialize_SetupLayout   TouchCameraGraphUtility"));
	}
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout(TouchCameraAssetToolkitTabId)
	->AddArea
	(
		FTabManager::NewPrimaryArea()->SetOrientation(Orient_Horizontal)
			
		->Split
		(
			FTabManager::NewSplitter()
			->SetSizeCoefficient(0.8f)
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetHideTabWell(true)
				->AddTab(FTouchCameraEditorTabs::DetailsID, ETabState::OpenedTab)
			)
		)
		->Split
		(
			FTabManager::NewSplitter()
			->SetSizeCoefficient(0.2f)
			->SetOrientation(Orient_Vertical)
			->Split
			(
			FTabManager::NewStack()
				->SetHideTabWell(true)
				->AddTab(FTouchCameraEditorTabs::ViewportID, ETabState::OpenedTab)
			)
		)
		->Split
		(
			FTabManager::NewSplitter()
			->SetSizeCoefficient(0.2f)
			->SetOrientation(Orient_Vertical)
			->Split
			(
			FTabManager::NewStack()
				->SetHideTabWell(true)
				->AddTab(FTouchCameraEditorTabs::EditorGraphID, ETabState::OpenedTab)
			)
		)
	);

	InitAssetEditor(InMode, InToolkitHost, TouchCameraAssetToolkitTabId, Layout, true, true,INTouchCameraObject);
}

// FTouchCameraAssetToolkit.cpp
void FTouchCameraAssetToolkit::InitializeSubToolkit(const EToolkitMode::Type Mode,
	const TSharedPtr< IToolkitHost>& InitToolkitHost,  UFlowAsset* Asset)
{
	// 1. 从父资产（UTouchCameraAsset）中获取子资产 UFlowAsset
	if (!TouchCameraObject || !TouchCameraObject->GetMainFlowAsset())
	{
		
		SubFlowAssetToolkit.Reset(); // 清空无效的子Toolkit
		FlowAsset = nullptr;
		return;
	}
	// 2. 记录子资产（用于后续同步）
	FlowAsset = TouchCameraObject->GetMainFlowAsset();
	// 3. 创建子Toolkit（FFlowAssetEditor）实例
	SubFlowAssetToolkit = MakeShareable(new FFlowAssetEditor());
	UTouchCameraGraph::CreateGraph(TouchCameraObject);
	SubFlowAssetToolkit->InitFlowAssetEditor(
		Mode,         
		InitToolkitHost,                  
		Asset);            
	
}
#undef LOCTEXT_NAMESPACE
