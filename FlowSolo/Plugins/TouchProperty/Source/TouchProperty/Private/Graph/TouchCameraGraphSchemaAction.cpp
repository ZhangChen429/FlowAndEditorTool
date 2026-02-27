
#include "Graph/TouchCameraGraphSchemaAction.h"
#include "Graph/CameraGraphSchema.h"
#include "Graph/TouchCameraGraph.h"
#include "TouchAsset/TouchCameraAsset.h"

#define LOCTEXT_NAMESPACE "FTouchCameraGraphSchemaAction_Action"

UTouchCameraBaseNobe* FTouchCameraGraphSchemaAction_NewNode::CreateNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin,
	const UClass* NodeClass, const FVector2D Location, const bool bSelectNewNode)
{
	const FScopedTransaction Transaction(LOCTEXT("AddNode", "Add Node"));
	// create new Graph node
	if (NodeClass == nullptr)
	{
		NodeClass = UCameraGraphSchema::StaticClass();
	}

	ParentGraph->Modify();
	if (FromPin)
	{
		FromPin->Modify();
	}
	
	UTouchCameraAsset* TouchCameraAsset = CastChecked<UTouchCameraGraph>(ParentGraph)->GetGraphSampleAsset();
	TouchCameraAsset->Modify();

	// create new Graph node
	const UClass* GraphNodeClass = UCameraGraphSchema::GetMappingGraphNodeClass(NodeClass);
	UTouchCameraBaseNobe* NewGraphNode = NewObject<UTouchCameraBaseNobe>(ParentGraph, GraphNodeClass, NAME_None, RF_Transactional);
	//UCameraGraphSchema* NewGraphSampleNode = NewObject<UCameraGraphSchema>(TouchCameraAsset, NodeClass, NAME_None, RF_Transactional);
	//NewGraphNode->SetGraphSampleNode(NewGraphSampleNode);
	

	return NewGraphNode;
}


UEdGraphNode* FTouchCameraGraphSchemaAction_NewComment::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin,
                                                                      const FVector2D Location, bool bSelectNewNode)
{
	return FEdGraphSchemaAction::PerformAction(ParentGraph, FromPin, Location, bSelectNewNode);
}

#undef LOCTEXT_NAMESPACE
