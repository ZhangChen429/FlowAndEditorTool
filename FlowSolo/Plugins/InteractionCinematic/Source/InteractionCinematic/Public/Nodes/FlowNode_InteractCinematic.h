// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "FlowNode_InteractCinematic.generated.h"

class UInteractionCinematicAsset;
class ULevelSequence;

/**
 * Interaction Cinematic Flow Node
 * Plays a cinematic sequence as part of the Flow Graph execution
 */
UCLASS(meta = (DisplayName = "InteractionCinematic Node", Keywords = "Cinematic"))
class INTERACTIONCINEMATIC_API UFlowNode_InteractCinematic : public UFlowNode
{
	GENERATED_UCLASS_BODY()

public:
	// The Level Sequence to play
	UPROPERTY(EditAnywhere, Category = "Cinematic")
	TSoftObjectPtr<UInteractionCinematicAsset> InteractionCinematicAsset;

	// Whether to wait for the sequence to finish before triggering output
	UPROPERTY(EditAnywhere, Category = "Cinematic")
	bool bWaitForCompletion = true;

protected:
	// Execute the node when input is triggered
	virtual void ExecuteInput(const FName& PinName) override;

	// Cleanup when node execution is finished
	virtual void Cleanup() override;

#if WITH_EDITOR
	// Optional: Custom node display text
	virtual FString GetNodeDescription() const override;

	// Override to support double-click opening the InteractionCinematicAsset
	virtual UObject* GetAssetToEdit() override;
#endif

private:
	// Handle sequence finished event
	void OnSequenceFinished();
};
