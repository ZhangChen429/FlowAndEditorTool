// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Graph/Nodes/FlowGraphNode.h"
#include "FlowGraphNode_FactNode.generated.h"

/**
 * Graph representation of the Fact Node
 * This controls how the node appears in the Flow Graph editor
 */
UCLASS()
class FLOWFACTEDITOR_API UFlowGraphNode_FactNode : public UFlowGraphNode
{
	GENERATED_UCLASS_BODY()

public:
	// UEdGraphNode
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	// --
};
