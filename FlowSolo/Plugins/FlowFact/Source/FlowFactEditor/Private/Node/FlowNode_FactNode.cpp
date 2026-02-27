// Fill out your copyright notice in the Description page of Project Settings.


#include "Node/FlowNode_FactNode.h"

#include "FlowFactChannels.h"

UFlowNode_FactNode::UFlowNode_FactNode()
{
	
	// 设置输入输出引脚
#if WITH_EDITOR
	Category = TEXT("Fact"); // 在节点菜单中的分类
#endif

	// 添加默认输入输出引脚 (可选)
	InputPins.Add(FFlowPin(TEXT("Create")));
	OutputPins.Add(FFlowPin(TEXT("Destory")));

}

void UFlowNode_FactNode::ExecuteInput(const FName& PinName)
{
	UE_LOG(LogFlowFact,Display,TEXT("FlowFact"));
	TriggerFirstOutput(true);

}

FString UFlowNode_FactNode::GetNodeDescription() const
{
	return TEXT("Fact node description");

}
