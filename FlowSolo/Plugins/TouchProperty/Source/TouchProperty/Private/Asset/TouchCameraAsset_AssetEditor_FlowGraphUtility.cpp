// Fill out your copyright notice in the Description page of Project Settings.


#include "Asset/TouchCameraAsset_AssetEditor_FlowGraphUtility.h"

#include "Asset/FlowAssetEditor.h"
#include "FactoryAndMode/FTouchCameraAssetToolkit.h"
#include "FlowEditor/Public/Graph/FlowGraphEditor.h"
#include "..\..\Public\Graph\TouchCamera_FlowGraph.h"
#include "Slate/TouchCameraViewer_EditorGraph.h"
#include "TouchAsset/TouchCameraAsset.h"

#define LOCTEXT_NAMESPACE "FTouchCameraAsset_AssetEditor_GraphUtility"

FTouchCameraAsset_AssetEditor_FlowGraphUtility::FTouchCameraAsset_AssetEditor_FlowGraphUtility(
	TSharedRef<FTouchCameraAssetToolkit>  InToolkit): FTouchCameraAsset_AssetEditor_BaseUtility(InToolkit)
{
	
}

FTouchCameraAsset_AssetEditor_FlowGraphUtility::~FTouchCameraAsset_AssetEditor_FlowGraphUtility()
{
	if (GraphEditor.IsValid())
	{
		GraphEditor.Reset();
	}
}

void FTouchCameraAsset_AssetEditor_FlowGraphUtility::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager,
                                                                     const TSharedRef<FWorkspaceItem> InMenuCategory)
{
	if (Toolkit.Pin()->GetTouchCameraObject())
	{
		STouchCameraViewer_EditorGraph::FArguments WidgetArgs;
		
		FDetailsViewArgs Args;
		Args.bHideSelectionTip = true;
		Args.bShowPropertyMatrixButton = false;
		Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;

		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		DetailsView = PropertyModule.CreateDetailView(Args);
		DetailsView->SetObject(Toolkit.Pin()->GetTouchCameraObject()->GetMainFlowAsset());
		GraphEditor	= SAssignNew(GraphEditor, STouchCameraViewer_EditorGraph, Toolkit.Pin());
		
	}
	
	InTabManager->RegisterTabSpawner(
			 FTouchCameraEditorTabs::EditorGraphID,FOnSpawnTab::CreateRaw(this, &FTouchCameraAsset_AssetEditor_FlowGraphUtility::SpawnTab_EditorGraph))
			 .SetDisplayName(LOCTEXT("EditorGraph", "Graph"))
			 .SetGroup(InMenuCategory)
			 .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FTouchCameraAsset_AssetEditor_FlowGraphUtility::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	InTabManager->UnregisterTabSpawner(FTouchCameraEditorTabs::EditorGraphID);
}

TSharedRef<SDockTab> FTouchCameraAsset_AssetEditor_FlowGraphUtility::SpawnTab_EditorGraph(const FSpawnTabArgs& Args) const
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Label(LOCTEXT("GraphSampleGraphTitle", "TouchCmeraGraph"));
	if (GraphEditor.IsValid())
	{
		SpawnedTab->SetContent(GraphEditor.ToSharedRef());
	}

	if (SpawnedTab.IsUnique())
	{
		return  SpawnedTab ;
	}
	return SNew(SDockTab);
}

#undef LOCTEXT_NAMESPACE

