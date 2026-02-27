// Fill out your copyright notice in the Description page of Project Settings.

#include "Nodes/FlowNode_InteractCinematic.h"

#include "InteractionCinematicAsset.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "MovieSceneSequencePlayer.h"

UFlowNode_InteractCinematic::UFlowNode_InteractCinematic(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITOR
	// Category in the node menu
	Category = TEXT("Cinematic");
#endif

	// Add default input/output pins
	InputPins.Add(FFlowPin(TEXT("Play")));
	OutputPins.Add(FFlowPin(TEXT("Finished")));
}

void UFlowNode_InteractCinematic::ExecuteInput(const FName& PinName)
{
	if (PinName == TEXT("Play"))
	{
		if (InteractionCinematicAsset.IsNull())
		{
			LogError(TEXT("Level Sequence is not set!"));
			TriggerFirstOutput(true);
			return;
		}

		// Load the sequence if it's not already loaded
		ULevelSequence* Sequence = InteractionCinematicAsset.LoadSynchronous()->Sequence;
		if (!Sequence)
		{
			LogError(TEXT("Failed to load Level Sequence!"));
			TriggerFirstOutput(true);
			return;
		}

		// TODO: Implement actual sequence playback
		// You'll need to create a LevelSequencePlayer and handle playback
		// For now, we'll just trigger the output immediately

		if (bWaitForCompletion)
		{
			// In a real implementation, you would:
			// 1. Create/get a LevelSequencePlayer
			// 2. Play the sequence
			// 3. Bind to the OnFinished event
			// 4. Call TriggerFirstOutput when finished

			// Placeholder: trigger immediately
			OnSequenceFinished();
		}
		else
		{
			// Don't wait, trigger output immediately
			TriggerFirstOutput(true);
		}
	}
}

void UFlowNode_InteractCinematic::Cleanup()
{
	// Clean up any sequence players or bindings here
	Super::Cleanup();
}

void UFlowNode_InteractCinematic::OnSequenceFinished()
{
	// Trigger the output pin when sequence is finished
	TriggerFirstOutput(true);
}

#if WITH_EDITOR
FString UFlowNode_InteractCinematic::GetNodeDescription() const
{
	if (!InteractionCinematicAsset.IsNull())
	{
		return FString::Printf(TEXT("Play: %s"), *InteractionCinematicAsset.GetAssetName());
	}
	return TEXT("Interaction Cinematic Node");
}

UObject* UFlowNode_InteractCinematic::GetAssetToEdit()
{
	return InteractionCinematicAsset.IsNull() ? nullptr : InteractionCinematicAsset.LoadSynchronous();
}
#endif
