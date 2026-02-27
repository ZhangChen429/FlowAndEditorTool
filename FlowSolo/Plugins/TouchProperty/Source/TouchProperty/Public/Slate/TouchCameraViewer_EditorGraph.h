#pragma once

#include "GraphEditor.h"
#include "CoreMinimal.h"

class UFlowAsset;
class FTouchCameraAssetToolkit;
class UTouchCameraAsset;
class UTouchCameraGraph;

class TOUCHPROPERTY_API STouchCameraViewer_EditorGraph : public SGraphEditor
{
	SLATE_BEGIN_ARGS(STouchCameraViewer_EditorGraph){}
	SLATE_ARGUMENT(FGraphEditorEvents, GraphEvents)
	SLATE_ARGUMENT(TSharedPtr<IDetailsView>, DetailsView)
SLATE_END_ARGS()
	
public:

	void Construct(const FArguments& InArgs, const TSharedPtr<FTouchCameraAssetToolkit> InAssetEditor);
	~STouchCameraViewer_EditorGraph();

	FGraphAppearanceInfo GetGraphAppearanceInfo() const;
	FText GetCornerText() const;
	void SelectSingleNode(UEdGraphNode* Node);
	void OnSelectedNodesChanged(const TSet<UObject*>& Nodes);
	//bool IsTabFocused() const;
	
	//======Flow====

	
protected:
	TWeakObjectPtr<UTouchCameraAsset> TouchCameraObject;
	TWeakObjectPtr<UFlowAsset> FlowAsset;
	TWeakPtr<FTouchCameraAssetToolkit> FlowAssetEditor;
	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<FUICommandList> TouchCameraCommandList;
	TSharedPtr<FUICommandList> CommandList;
protected:

	virtual void OnNodeDoubleClicked(class UEdGraphNode* Node) const;
	virtual void OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged);
	
protected:


	/** Callback */
	void OnCreateComment() const;
	static bool CanEdit();
};


