// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "UObject/Object.h"
#include "FlowNode_FactNode.generated.h"

/**
 * 
 */
USTRUCT()
struct FFactNodeParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Fact")
	FString FactName;

	UPROPERTY(EditAnywhere, Category = "Fact")
	int32 FactValue;

	UPROPERTY(EditAnywhere, Category = "Fact")
	bool bSomeBool;

	FFactNodeParams()
		: FactValue(0)
		, bSomeBool(false)
	{
	}
};

UCLASS(meta = (DisplayName = "Fact Node", Keywords = "Fact"))
class FLOWFACTEDITOR_API UFlowNode_FactNode : public UFlowNode
{
	GENERATED_BODY()
	UFlowNode_FactNode();
protected:
	// 你的自定义属性
	UPROPERTY(EditAnywhere, Category = "FlowNode", meta = (ShowOnlyInnerProperties))
	FFactNodeParams CustomProperty;

protected:
	// 实现ExecuteInput来处理节点执行
	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	// 可选: 自定义节点显示文本
	virtual FString GetNodeDescription() const override;
#endif

};
