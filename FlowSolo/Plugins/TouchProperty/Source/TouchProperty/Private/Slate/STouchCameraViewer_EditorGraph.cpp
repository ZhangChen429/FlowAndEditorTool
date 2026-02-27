#include "FlowAsset.h"
#include "FactoryAndMode/FTouchCameraAssetToolkit.h"
#include "Graph/FlowGraphSchema_Actions.h"
#include "Slate/TouchCameraViewer_EditorGraph.h"
#include "TouchAsset/TouchCameraAsset.h"

#define LOCTEXT_NAMESPACE "TouchCameraViewer_EditorGraph"
void STouchCameraViewer_EditorGraph::Construct(const FArguments& InArgs,
											   const TSharedPtr<FTouchCameraAssetToolkit> InAssetEditor)
{
	AssetEditorToolkit=InAssetEditor;
	
	//const FTouchCameraAssetToolkit*TouchCameraAssetToolkit =Cast<FTouchCameraAssetToolkit>(AssetEditorToolkit.Pin().Get());
	SGraphEditor::FArguments Arguments;
	if (InAssetEditor)
	{
		FlowAssetEditor=InAssetEditor;
		TouchCameraObject=InAssetEditor->GetTouchCameraObject();
		FlowAsset = TouchCameraObject->GetMainFlowAsset();
		DetailsView =InArgs._DetailsView;
		

		Arguments._AdditionalCommands = CommandList;
		Arguments._GraphToEdit = TouchCameraObject->GetGraph()	;	//GetGraph()	->GetGraph();
		
	}
	Arguments._AssetEditorToolkit = AssetEditorToolkit;
	Arguments._GraphEvents = InArgs._GraphEvents;
    Arguments._AutoExpandActionMenu = true;
	Arguments._Appearance = GetGraphAppearanceInfo();
	Arguments._GraphEvents.OnSelectionChanged = FOnSelectionChanged::CreateSP(this, &STouchCameraViewer_EditorGraph::OnSelectedNodesChanged);
	Arguments._GraphEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &STouchCameraViewer_EditorGraph::OnNodeDoubleClicked);
	Arguments._GraphEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &STouchCameraViewer_EditorGraph::OnNodeTitleCommitted);

	SGraphEditor::Construct(Arguments);
}

STouchCameraViewer_EditorGraph::~STouchCameraViewer_EditorGraph()
{
	if (TouchCameraCommandList.IsValid())
	{
		TouchCameraCommandList.Reset();
	}
	
	//DetailsView.Reset();
	//SGraphEditor::~SGraphEditor();
}

FGraphAppearanceInfo STouchCameraViewer_EditorGraph::GetGraphAppearanceInfo() const
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = GetCornerText();
	return AppearanceInfo;
}

FText STouchCameraViewer_EditorGraph::GetCornerText() const
{
	return LOCTEXT("AppearanceCornerText_TouchCameraAsset", "TouchSample");
}

void STouchCameraViewer_EditorGraph::SelectSingleNode(UEdGraphNode* Node)
{
}


void STouchCameraViewer_EditorGraph::OnCreateComment() const
{
	FFlowGraphSchemaAction_NewComment CommentAction;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 6
	CommentAction.PerformAction(FlowAsset->GetGraph(), nullptr, GetPasteLocation());
#else
	CommentAction.PerformAction(FlowAsset->GetGraph(), nullptr, GetPasteLocation2f());
#endif
}

bool STouchCameraViewer_EditorGraph::CanEdit()
{
	return GEditor->PlayWorld == nullptr;
}


void STouchCameraViewer_EditorGraph::OnSelectedNodesChanged(const TSet<UObject*>& Nodes)
{
	UE_LOG(LogTemp, Log, TEXT("mNumFaces: %d"),12);
}
//
//bool STouchCameraViewer_EditorGraph::IsTabFocused() const
//{
//	if (Cast<FTouchCameraAssetToolkit>(AssetEditorToolkit.Pin().Get()))
//	{
//		return Cast<FTouchCameraAssetToolkit>(AssetEditorToolkit.Pin().Get())->IsTabFocused( FTouchCameraEditorTabs::EditorGraphID);
//	}
//	return false;
//}

void STouchCameraViewer_EditorGraph::OnNodeDoubleClicked(UEdGraphNode* Node) const
{
	UE_LOG(LogTemp, Log, TEXT("mNumFaces: %d"),13);
}

void STouchCameraViewer_EditorGraph::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo,
	UEdGraphNode* NodeBeingChanged)
{
	UE_LOG(LogTemp, Log, TEXT("mNumFaces: %d"),14);
}



#undef LOCTEXT_NAMESPACE