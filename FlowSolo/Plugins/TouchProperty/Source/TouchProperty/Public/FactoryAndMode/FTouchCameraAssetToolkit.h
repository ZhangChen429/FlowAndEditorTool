#pragma once
#include "Asset/FlowAssetEditor.h"
#include "Slate/TouchCameraViewer_EditorGraph.h"
#include "Toolkits/ToolkitManager.h"
#include "TouchAsset/TouchCameraAsset.h"


class SFlowGraphEditor;
class FTouchCameraAsset_AssetEditor_FlowGraphUtility;
class FTouchCameraAsset_AssetEditor_ViewUtility;
class FTouchCameraAsset_AssetEditor_DataUtility;


struct FTouchCameraEditorTabs
{
	// Tab identifiers
	static  const FName DetailsID		;
	static  const FName ViewportID		;
	static  const FName EditorGraphID	;
};


//////////////////////////////////////////////////////////////////////////


//const FName FTouchCameraEditorTabs::ToolboxHostID(TEXT("ToolboxHostID"));

//////////////////////////////////////////////////////////////////////////


class FTouchCameraAssetToolkit: public FAssetEditorToolkit, public FEditorUndoClient
{
public:

	/** BaseEpisode_AssetEditorToolkit */
	virtual FName GetTouchObjectAssetEditorName() const { return FName("TouchCameraAsset"); }

	/** FAssetEditorToolkit */
	void Initialize(const EToolkitMode::Type Mode, const TSharedPtr<  IToolkitHost >& InitToolkitHost,  UTouchCameraAsset* Asset);
	virtual  FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	UTouchCameraAsset* GetTouchCameraObject() const { return TouchCameraObject; }

	void Initialize_SetupLayout(const EToolkitMode::Type InMode,
	const TSharedPtr<IToolkitHost>& InToolkitHost,UObject* INTouchCameraObject);

	//===========FlowGraph============
public:
	
	TSharedPtr<FFlowAssetEditor>GetSubToolkit(){return SubFlowAssetToolkit; }
	
	
	
protected:

	
	void InitializeSubToolkit(const EToolkitMode::Type Mode,
	const TSharedPtr< IToolkitHost>& InitToolkitHost,  UFlowAsset* Asset);
	

	//~  ===========FlowGraph============
	TSharedPtr<class IDetailsView> DetailsView;
	TSharedPtr<FFlowAssetEditor> SubFlowAssetToolkit;
	TObjectPtr<UFlowAsset> FlowAsset;
	// ========FEditorUndoClient

	//virtual void HandleUndoTransaction();
	//~========FEditorUndoClient
protected:

	TSharedPtr<FTouchCameraAsset_AssetEditor_DataUtility> TouchCameraDataUtility;
	TSharedPtr<FTouchCameraAsset_AssetEditor_ViewUtility> TouchCameraViewUtility;
	TSharedPtr<FTouchCameraAsset_AssetEditor_FlowGraphUtility>TouchCameraGraphUtility ;
private:

	static const FName TouchCameraAssetToolkitTabId;
	UTouchCameraAsset* TouchCameraObject = nullptr;
	
};

