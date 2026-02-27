#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "FlowNode_CinematicPlaySequence.generated.h"

class ULevelSequence;

/**
 * Plays a Level Sequence during cinematic flow
 *
 * This node is restricted to InteractionCinematicFlowAsset only.
 * It provides cinematic-specific functionality that doesn't make sense in regular flows.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Play Cinematic Sequence"))
class INTERACTIONCINEMATIC_API UFlowNode_CinematicPlaySequence : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_CinematicPlaySequence();

#if WITH_EDITOR
	virtual FString GetNodeCategory() const override { return TEXT("Cinematic"); }
#endif

protected:
	/** The sequence to play */
	UPROPERTY(EditAnywhere, Category = "Sequence")
	TSoftObjectPtr<ULevelSequence> Sequence;

	/** Whether to wait for sequence to complete before triggering output */
	UPROPERTY(EditAnywhere, Category = "Sequence")
	bool bWaitForCompletion = true;

	virtual void ExecuteInput(const FName& PinName) override;
};
