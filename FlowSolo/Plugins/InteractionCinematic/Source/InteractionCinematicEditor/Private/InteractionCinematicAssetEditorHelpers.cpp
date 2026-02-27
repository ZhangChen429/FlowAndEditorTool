#include "InteractionCinematicAssetEditorHelpers.h"
#include "InteractionCinematicAsset.h"
#include "InteractionCinematicFlowAsset.h"
#include "Graph/FlowGraph.h"

void FInteractionCinematicAssetEditorHelpers::CreateFlowAssetForAsset(UInteractionCinematicAsset* Asset)
{
	if (!Asset)
	{
		return;
	}

	// Only create if it doesn't exist
	if (Asset->GetFlowAsset())
	{
		return;
	}

	// Create an embedded InteractionCinematicFlowAsset as a subobject
	// Using our custom FlowAsset subclass allows nodes to restrict themselves
	// to only work within InteractionCinematic context
	UInteractionCinematicFlowAsset* NewFlowAsset = NewObject<UInteractionCinematicFlowAsset>(
		Asset,                                    // Outer
		UInteractionCinematicFlowAsset::StaticClass(), // Class - using custom subclass
		NAME_None,                                // Name
		RF_Transactional | RF_Public             // Flags
	);

	// Create the FlowGraph for this FlowAsset
	// This will set NewFlowAsset->FlowGraph automatically
	UFlowGraph::CreateGraph(NewFlowAsset);

	// Assign the embedded FlowAsset to our asset
	Asset->EmbeddedFlowAsset = NewFlowAsset;
}
