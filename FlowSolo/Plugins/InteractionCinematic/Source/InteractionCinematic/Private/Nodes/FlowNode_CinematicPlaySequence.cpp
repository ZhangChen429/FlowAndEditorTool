#include "Nodes/FlowNode_CinematicPlaySequence.h"
#include "InteractionCinematicFlowAsset.h"
#include "LevelSequence.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlowNode_CinematicPlaySequence)

UFlowNode_CinematicPlaySequence::UFlowNode_CinematicPlaySequence()
{
#if WITH_EDITOR
	Category = TEXT("Cinematic");
#endif

	// ✅ KEY: Restrict this node to only work in InteractionCinematicFlowAsset
	// This makes sense because playing sequences is specifically a cinematic operation
	AllowedAssetClasses.Empty();
	AllowedAssetClasses.Add(UInteractionCinematicFlowAsset::StaticClass());

	InputPins = {TEXT("Play")};
	OutputPins = {TEXT("Started"), TEXT("Completed"), TEXT("Failed")};
}

void UFlowNode_CinematicPlaySequence::ExecuteInput(const FName& PinName)
{
	if (Sequence.IsNull())
	{
		LogError(TEXT("Sequence is not set!"));
		TriggerOutput(TEXT("Failed"), true);
		return;
	}

	// In a real implementation, you would:
	// 1. Get or create a LevelSequencePlayer
	// 2. Play the sequence
	// 3. Optionally wait for completion before triggering output

	// For now, just trigger Started
	LogNote(FString::Printf(TEXT("Playing sequence: %s"), *Sequence.ToString()));
	TriggerOutput(TEXT("Started"), true);

	// If not waiting for completion, trigger Completed immediately
	if (!bWaitForCompletion)
	{
		TriggerOutput(TEXT("Completed"), true);
	}
	// Otherwise, you would bind to sequence completion event
}
