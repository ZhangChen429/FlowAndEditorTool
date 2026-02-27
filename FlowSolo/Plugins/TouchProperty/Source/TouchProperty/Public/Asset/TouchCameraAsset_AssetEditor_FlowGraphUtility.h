// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TouchCameraAsset_AssetEditor_BaseUtility.h"
class STouchCamera_FlowGraph;
class SFlowGraphEditor;
class STouchCameraViewer_EditorGraph;
class TOUCHPROPERTY_API FTouchCameraAsset_AssetEditor_FlowGraphUtility :public FTouchCameraAsset_AssetEditor_BaseUtility
{
public:
	explicit FTouchCameraAsset_AssetEditor_FlowGraphUtility(TSharedRef<FTouchCameraAssetToolkit>  InToolkit);
	virtual ~FTouchCameraAsset_AssetEditor_FlowGraphUtility() override;
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager, const TSharedRef<FWorkspaceItem> InMenuCategory) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	TSharedRef<SDockTab> SpawnTab_EditorGraph(const FSpawnTabArgs& Args) const;

protected:
	
	TSharedPtr<SGraphEditor> GraphEditor;
	TSharedPtr< IDetailsView> DetailsView;
};
