// Fill out your copyright notice in the Description page of Project Settings.

#include "Node/FlowNode_WaitContinue.h"
#include "FlowFactChannels.h"
#include "Condition/TimeCondition.h"
#include "Condition/CustomCondition.h"
#include "WaitContinue/EventListener.h"


UFlowNode_WaitContinue::UFlowNode_WaitContinue()
	: ConditionType(EWaitConditionType::Time)
	, DelaySeconds(1.0f)
{
#if WITH_EDITOR
	Category = TEXT("WaitContinue"); // Category in node menu
#endif

	// Set default input/output pins
	InputPins.Empty();
	OutputPins.Empty();

	InputPins.Add(FFlowPin(TEXT("In")));
	OutputPins.Add(FFlowPin(TEXT("Out")));
}

void UFlowNode_WaitContinue::InitializeInstance()
{
	Super::InitializeInstance();

	// Clear any previous state
	ExecutionContext.ClearAllListeners();
	CurrentCondition.Reset();
}

void UFlowNode_WaitContinue::ExecuteInput(const FName& PinName)
{
	UE_LOG(LogFlowFact, Display, TEXT("WaitContinue node executed"));

	// Create the condition based on settings
	CurrentCondition = CreateCondition();

	if (!CurrentCondition.IsValid())
	{
		UE_LOG(LogFlowFact, Warning, TEXT("Failed to create condition, triggering output immediately"));
		TriggerFirstOutput(true);
		return;
	}

	// Check if condition is immediately fulfilled
	if (CurrentCondition->IsFulfilled(ExecutionContext))
	{
		UE_LOG(LogFlowFact, Display, TEXT("Condition immediately fulfilled"));
		TriggerFirstOutput(true);
		return;
	}

	// Create and register event listener
	TSharedPtr<WaitContinueSystem::FEventListener> Listener = CurrentCondition->CreateEventListener(ExecutionContext);
	if (Listener.IsValid())
	{
		Listener->OnCreate(ExecutionContext);
		ExecutionContext.RegisterListener(NodeGuid, Listener);

		// Start checking condition periodically
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				UpdateTimerHandle,
				this,
				&UFlowNode_WaitContinue::CheckCondition,
				0.1f,  // Check every 0.1 seconds
				true   // Loop
			);
		}

		UE_LOG(LogFlowFact, Display, TEXT("Condition listener registered, waiting for: %s"), *CurrentCondition->GetFriendlyName());
	}
	else
	{
		UE_LOG(LogFlowFact, Warning, TEXT("Failed to create event listener"));
		TriggerFirstOutput(true);
	}
}

void UFlowNode_WaitContinue::CheckCondition()
{
	if (!CurrentCondition.IsValid())
	{
		return;
	}

	// Update time listeners if needed
	if (ConditionType == EWaitConditionType::Time)
	{
		if (WaitContinueSystem::FEventListener* Listener = ExecutionContext.FindListener(NodeGuid))
		{
			if (WaitContinueSystem::FTimeEventListener* TimeListener = static_cast<WaitContinueSystem::FTimeEventListener*>(Listener))
			{
				TimeListener->Update();
			}
		}
	}

	// Check if condition is fulfilled
	if (CurrentCondition->IsFulfilled(ExecutionContext))
	{
		UE_LOG(LogFlowFact, Display, TEXT("Condition fulfilled: %s"), *CurrentCondition->GetFriendlyName());

		// Stop checking
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(UpdateTimerHandle);
		}

		// Trigger output and finish
		TriggerFirstOutput(true);
	}
}

TSharedPtr<WaitContinueSystem::ICondition> UFlowNode_WaitContinue::CreateCondition()
{
	switch (ConditionType)
	{
	case EWaitConditionType::Time:
		return MakeShared<WaitContinueSystem::FTimeCondition>(NodeGuid, DelaySeconds);

	case EWaitConditionType::Custom:
		// For now, create a simple test condition
		// In the future, this could be exposed to Blueprint or C++
		return MakeShared<WaitContinueSystem::FCustomCondition>(
			NodeGuid,
			[]() { return false; } // Always false for now
		);

	default:
		return nullptr;
	}
}

void UFlowNode_WaitContinue::Cleanup()
{
	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}

	// Unregister listener
	ExecutionContext.UnregisterListener(NodeGuid);

	// Clear condition
	CurrentCondition.Reset();

	Super::Cleanup();
}

#if WITH_EDITOR
FString UFlowNode_WaitContinue::GetNodeDescription() const
{
	switch (ConditionType)
	{
	case EWaitConditionType::Time:
		return FString::Printf(TEXT("Wait %.2f seconds"), DelaySeconds);

	case EWaitConditionType::Custom:
		return TEXT("Wait for custom condition");

	default:
		return TEXT("Wait for condition");
	}
}

FString UFlowNode_WaitContinue::GetStatusString() const
{
	if (!CurrentCondition.IsValid())
	{
		return TEXT("No active condition");
	}

	if (ConditionType == EWaitConditionType::Time)
	{
		if (const WaitContinueSystem::FEventListener* Listener = ExecutionContext.FindListener(NodeGuid))
		{
			if (const WaitContinueSystem::FTimeEventListener* TimeListener = static_cast<const WaitContinueSystem::FTimeEventListener*>(Listener))
			{
				const float Remaining = TimeListener->GetRemainingTime();
				return FString::Printf(TEXT("Waiting: %.2f seconds remaining"), Remaining);
			}
		}
	}

	return FString::Printf(TEXT("Waiting for: %s"), *CurrentCondition->GetFriendlyName());
}
#endif
