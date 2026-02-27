
#include "Graph/TouchCamera_FlowGraph.h"
#include "FlowEditorCommands.h"
#include "Debugger/FlowDebuggerSubsystem.h"
#include "FactoryAndMode/FTouchCameraAssetToolkit.h"

void STouchCamera_FlowGraph::Construct(const FArguments& InArgs,
                                      const TSharedPtr<FTouchCameraAssetToolkit> InAssetEditor)
{
	FlowAssetEditor = InAssetEditor->GetSubToolkit();
	TouchCameraObject = InAssetEditor->GetTouchCameraObject();
	FlowAsset = TouchCameraObject->GetMainFlowAsset();
	DetailsView = InArgs._DetailsView;
	DebuggerSubsystem = GEngine->GetEngineSubsystem<UFlowDebuggerSubsystem>();
	
	BindGraphCommands();

	SGraphEditor::FArguments Arguments;
	Arguments._AdditionalCommands = CommandList;
	Arguments._Appearance = GetGraphAppearanceInfo();
	Arguments._GraphToEdit = TouchCameraObject->GetGraph();
	Arguments._GraphEvents = InArgs._GraphEvents;
	Arguments._AutoExpandActionMenu = true;
	Arguments._GraphEvents.OnSelectionChanged = FOnSelectionChanged::CreateSP(this, &STouchCamera_FlowGraph::OnSelectedNodesChanged);
	Arguments._GraphEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &STouchCamera_FlowGraph::OnNodeDoubleClicked);
	Arguments._GraphEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &STouchCamera_FlowGraph::OnNodeTitleCommitted);
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 6
	Arguments._GraphEvents.OnSpawnNodeByShortcut = FOnSpawnNodeByShortcut::CreateStatic(&STouchCamera_FlowGraph::TCOnSpawnGraphNodeByShortcut, static_cast<UEdGraph*>(TouchCameraObject->GetGraph()));
#else
	Arguments._GraphEvents.OnSpawnNodeByShortcutAtLocation = FOnSpawnNodeByShortcutAtLocation::CreateStatic(&STouchCamera_FlowGraph::TCOnSpawnGraphNodeByShortcut, static_cast<UEdGraph*>(TouchCameraObject->GetGraph()));
#endif

	SGraphEditor::Construct(Arguments);
}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 6
FReply STouchCamera_FlowGraph::TCOnSpawnGraphNodeByShortcut(FInputChord InChord, const FVector2D& InPosition, UEdGraph* InGraph)
#else
FReply STouchCamera_FlowGraph::TCOnSpawnGraphNodeByShortcut(FInputChord InChord, const FVector2f& InPosition, UEdGraph* InGraph)
#endif
{

	UEdGraph* Graph = InGraph;

	if (FFlowSpawnNodeCommands::IsRegistered())
	{
		const TSharedPtr<FEdGraphSchemaAction> Action = FFlowSpawnNodeCommands::Get().GetActionByChord(InChord);
		if (Action.IsValid())
		{
			TArray<UEdGraphPin*> DummyPins;

			Action->PerformAction(Graph, DummyPins, InPosition);
			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

