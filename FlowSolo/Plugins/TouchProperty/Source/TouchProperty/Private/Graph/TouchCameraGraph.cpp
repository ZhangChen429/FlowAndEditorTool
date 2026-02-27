// Fill out your copyright notice in the Description page of Project Settings.


#include "Graph/TouchCameraGraph.h"

#include "Graph/CameraGraphSchema.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "TouchAsset/TouchCameraAsset.h"


UEdGraph* UTouchCameraGraph::CreateGraph(UTouchCameraAsset* InGraphSampleAsset)
{
	return CreateGraph(InGraphSampleAsset, UCameraGraphSchema::StaticClass());
}

UEdGraph* UTouchCameraGraph::CreateGraph(UTouchCameraAsset* InGraphSampleAsset,
	const TSubclassOf<UCameraGraphSchema>& GraphSampleSchema)
{
	check(GraphSampleSchema);
	
	
	InGraphSampleAsset->GraphSampleGraph = CastChecked<UTouchCameraGraph>(FBlueprintEditorUtils::CreateNewGraph(InGraphSampleAsset, NAME_None, StaticClass(), GraphSampleSchema));
	InGraphSampleAsset->GraphSampleGraph->bAllowDeletion = false;
	InGraphSampleAsset->GraphSampleGraph->GetSchema()->CreateDefaultNodesForGraph(*InGraphSampleAsset->GraphSampleGraph);
	InGraphSampleAsset->GraphSampleGraph->GetSchema()->CreateDefaultNodesForGraph(*InGraphSampleAsset->GraphSampleGraph);
	return InGraphSampleAsset->GraphSampleGraph;

}

UTouchCameraAsset* UTouchCameraGraph::GetGraphSampleAsset() const
{
	return GetTypedOuter<UTouchCameraAsset>();
}

void UTouchCameraGraph::OnGraphSampleNodeChanged()
{
	NotifyGraphChanged();
}
