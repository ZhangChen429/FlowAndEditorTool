// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "WaitContinue/ExecutionContext.h"
#include "Condition/ICondition.h"
#include "FlowNode_WaitContinue.generated.h"

/**
 * Condition type for the Wait Continue node
 */
UENUM(BlueprintType)
enum class EWaitConditionType : uint8
{
	Time		UMETA(DisplayName = "Time Delay"),
	Custom		UMETA(DisplayName = "Custom Condition")
};

/**
 * FlowNode that implements Wait-Continue-Condition mechanism
 * Allows the flow to pause and wait for specific conditions before continuing
 */
UCLASS(meta = (DisplayName = "Wait Continue", Keywords = "wait, condition, pause, delay"))
class FLOWFACTEDITOR_API UFlowNode_WaitContinue : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_WaitContinue();

protected:
	// Condition type to use
	UPROPERTY(EditAnywhere, Category = "WaitContinue")
	EWaitConditionType ConditionType;

	// Delay time in seconds (for Time condition)
	UPROPERTY(EditAnywhere, Category = "WaitContinue", meta = (EditCondition = "ConditionType == EWaitConditionType::Time", ClampMin = "0.0"))
	float DelaySeconds;

	// FlowNode interface
	virtual void InitializeInstance() override;
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void Cleanup() override;
	// --

private:
	// Execution context for managing listeners
	WaitContinueSystem::FExecutionContext ExecutionContext;

	// The condition being waited on
	TSharedPtr<WaitContinueSystem::ICondition> CurrentCondition;

	// Timer handle for updating time conditions
	FTimerHandle UpdateTimerHandle;

	// Check if the condition is met
	void CheckCondition();

	// Create the appropriate condition based on settings
	TSharedPtr<WaitContinueSystem::ICondition> CreateCondition();

#if WITH_EDITOR
	// Optional: Custom node display text
	virtual FString GetNodeDescription() const override;
	virtual FString GetStatusString() const override;
#endif
};
