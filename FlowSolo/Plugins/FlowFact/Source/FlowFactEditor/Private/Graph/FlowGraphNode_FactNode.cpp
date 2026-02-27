// Fill out your copyright notice in the Description page of Project Settings.

#include "Graph/FlowGraphNode_FactNode.h"
#include "Graph/SFlowGraphNode_FactNode.h"
#include "Node/FlowNode_FactNode.h"

UFlowGraphNode_FactNode::UFlowGraphNode_FactNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 指定这个图节点对应哪个运行时节点类
	AssignedNodeClasses = {UFlowNode_FactNode::StaticClass()};
}

TSharedPtr<SGraphNode> UFlowGraphNode_FactNode::CreateVisualWidget()
{
	// 返回自定义的 Slate Widget
	return SNew(SFlowGraphNode_FactNode, this);
}

FLinearColor UFlowGraphNode_FactNode::GetNodeTitleColor() const
{
	// 自定义节点标题颜色 (紫色)
	return FLinearColor(0.6f, 0.2f, 0.8f);
}

FSlateIcon UFlowGraphNode_FactNode::GetIconAndTint(FLinearColor& OutColor) const
{
	// 自定义节点图标
	OutColor = FLinearColor::White;
	return FSlateIcon("EditorStyle", "GraphEditor.PadEvent_16x");
}
