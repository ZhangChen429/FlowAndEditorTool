// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Graph/Widgets/SFlowGraphNode.h"

/**
 * Slate widget for Interaction Cinematic Node
 * Uses default Flow Graph node styling
 */
class INTERACTIONCINEMATICEDITOR_API SFlowGraphNode_InteractCinematicNode : public SFlowGraphNode
{
public:
	SLATE_BEGIN_ARGS(SFlowGraphNode_InteractCinematicNode) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UFlowGraphNode* InNode);
};
