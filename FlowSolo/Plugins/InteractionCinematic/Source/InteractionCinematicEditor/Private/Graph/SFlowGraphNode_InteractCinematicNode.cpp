// Fill out your copyright notice in the Description page of Project Settings.

#include "Graph/SFlowGraphNode_InteractCinematicNode.h"
#include "Graph/FlowGraphNode_InteractCinematicNode.h"

void SFlowGraphNode_InteractCinematicNode::Construct(const FArguments& InArgs, UFlowGraphNode* InNode)
{
	// Call parent construct to use default Flow Graph node styling
	SFlowGraphNode::Construct(SFlowGraphNode::FArguments(), InNode);
}
