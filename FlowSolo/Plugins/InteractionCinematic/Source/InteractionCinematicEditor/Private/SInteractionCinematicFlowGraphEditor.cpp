#include "SInteractionCinematicFlowGraphEditor.h"
#include "FlowAsset.h"
#include "Graph/Nodes/FlowGraphNode.h"
#include "Nodes/FlowNode.h"
#include "Framework/Commands/GenericCommands.h"
#include "ScopedTransaction.h"
#include "IDetailsView.h"

#define LOCTEXT_NAMESPACE "SInteractionCinematicFlowGraphEditor"

void SInteractionCinematicFlowGraphEditor::Construct(const FArguments& InArgs, UFlowAsset* InFlowAsset)
{
	FlowAsset = InFlowAsset;
	DetailsView = InArgs._DetailsView;

	// Create custom command list
	CommandList = MakeShareable(new FUICommandList);

	// Bind delete command to our custom implementation
	CommandList->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateSP(this, &SInteractionCinematicFlowGraphEditor::DeleteSelectedNodes),
		FCanExecuteAction::CreateSP(this, &SInteractionCinematicFlowGraphEditor::CanDeleteNodes)
	);

	// Setup SGraphEditor arguments
	SGraphEditor::FArguments Arguments;
	Arguments._AdditionalCommands = CommandList;
	Arguments._GraphToEdit = InFlowAsset ? InFlowAsset->GetGraph() : nullptr;
	// Bind selection changed event
	Arguments._GraphEvents.OnSelectionChanged = FOnSelectionChanged::CreateSP(this, &SInteractionCinematicFlowGraphEditor::OnSelectedNodesChanged);

	// Construct the base SGraphEditor
	SGraphEditor::Construct(Arguments);
}

void SInteractionCinematicFlowGraphEditor::DeleteSelectedNodes()
{
	if (!FlowAsset.IsValid())
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteSelectedNode", "Delete Selected Node"));
	UEdGraph* Graph = GetCurrentGraph();

	if (!Graph)
	{
		return;
	}

	Graph->Modify();
	FlowAsset->Modify();

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*NodeIt);
		if (Node && Node->CanUserDeleteNode())
		{
			// If this is a FlowGraphNode, we need to unregister it from the FlowAsset
			if (const UFlowGraphNode* FlowGraphNode = Cast<UFlowGraphNode>(Node))
			{
				if (const UFlowNode* FlowNode = Cast<UFlowNode>(FlowGraphNode->GetFlowNodeBase()))
				{
					// Break links, destroy node, and unregister from FlowAsset
					Graph->GetSchema()->BreakNodeLinks(*Node);
					Node->DestroyNode();
					FlowAsset->UnregisterNode(FlowNode->GetGuid());
					continue;
				}
			}

			// For non-FlowNode nodes, just break links and destroy
			Graph->GetSchema()->BreakNodeLinks(*Node);
			Node->DestroyNode();
		}
	}
}

bool SInteractionCinematicFlowGraphEditor::CanDeleteNodes() const
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		if (const UEdGraphNode* Node = Cast<UEdGraphNode>(*NodeIt))
		{
			if (Node->CanUserDeleteNode())
			{
				return true;
			}
		}
	}

	return false;
}

void SInteractionCinematicFlowGraphEditor::OnSelectedNodesChanged(const TSet<UObject*>& Nodes)
{
	TArray<UObject*> SelectedObjects;

	if (Nodes.Num() > 0)
	{
		// Has selected nodes - show their properties in Details panel
		for (TSet<UObject*>::TConstIterator SetIt(Nodes); SetIt; ++SetIt)
		{
			if (const UFlowGraphNode* GraphNode = Cast<UFlowGraphNode>(*SetIt))
			{
				// Convert FlowGraphNode (editor node) to FlowNodeBase (runtime node)
				// This is what the Details panel should display
				SelectedObjects.Add(Cast<UObject>(GraphNode->GetFlowNodeBase()));
			}
			else
			{
				SelectedObjects.Add(*SetIt);
			}
		}
	}
	else
	{
		// No nodes selected - show the FlowAsset itself
		if (FlowAsset.IsValid())
		{
			SelectedObjects.Add(FlowAsset.Get());
		}
	}

	// Update the Details panel to show the selected objects
	if (DetailsView.IsValid())
	{
		DetailsView->SetObjects(SelectedObjects);
	}
}

#undef LOCTEXT_NAMESPACE
