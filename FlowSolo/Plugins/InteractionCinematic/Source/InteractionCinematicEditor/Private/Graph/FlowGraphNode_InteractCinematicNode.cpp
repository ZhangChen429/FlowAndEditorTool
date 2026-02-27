// Fill out your copyright notice in the Description page of Project Settings.

#include "Graph/FlowGraphNode_InteractCinematicNode.h"
#include "Graph/SFlowGraphNode_InteractCinematicNode.h"
#include "Nodes/FlowNode_InteractCinematic.h"
#include "EditorStyleSet.h"

UFlowGraphNode_InteractCinematicNode::UFlowGraphNode_InteractCinematicNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Assign the runtime FlowNode class that this graph node represents
	AssignedNodeClasses = {UFlowNode_InteractCinematic::StaticClass()};
}

TSharedPtr<SGraphNode> UFlowGraphNode_InteractCinematicNode::CreateVisualWidget()
{
	// Return custom Slate widget
	return SNew(SFlowGraphNode_InteractCinematicNode, this);
}

FLinearColor UFlowGraphNode_InteractCinematicNode::GetNodeTitleColor() const
{
	// Custom node title color (teal/cyan for cinematic)
	return FLinearColor(0.2f, 0.7f, 0.8f);
}

FSlateIcon UFlowGraphNode_InteractCinematicNode::GetIconAndTint(FLinearColor& OutColor) const
{
	// Custom node icon
	OutColor = FLinearColor::White;
	return FSlateIcon("EditorStyle", "GraphEditor.Sequence_16x");
}


