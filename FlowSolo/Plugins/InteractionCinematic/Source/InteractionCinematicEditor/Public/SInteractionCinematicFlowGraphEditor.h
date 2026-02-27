#pragma once

#include "CoreMinimal.h"
#include "GraphEditor.h"

class UFlowAsset;
class FUICommandList;
class IDetailsView;

/**
 * Custom FlowGraph editor for InteractionCinematic that properly handles node deletion
 */
class SInteractionCinematicFlowGraphEditor : public SGraphEditor
{
public:
	SLATE_BEGIN_ARGS(SInteractionCinematicFlowGraphEditor)
	{
	}
		SLATE_ARGUMENT(TSharedPtr<IDetailsView>, DetailsView)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UFlowAsset* InFlowAsset);

protected:
	/** Delete selected nodes and properly unregister them from FlowAsset */
	void DeleteSelectedNodes();

	/** Check if any selected nodes can be deleted */
	bool CanDeleteNodes() const;

	/** Called when node selection changes in the graph */
	void OnSelectedNodesChanged(const TSet<UObject*>& Nodes);

private:
	/** The FlowAsset being edited */
	TWeakObjectPtr<UFlowAsset> FlowAsset;

	/** Custom command list for handling graph operations */
	TSharedPtr<FUICommandList> CommandList;

	/** Details view to update when selection changes */
	TSharedPtr<IDetailsView> DetailsView;
};
