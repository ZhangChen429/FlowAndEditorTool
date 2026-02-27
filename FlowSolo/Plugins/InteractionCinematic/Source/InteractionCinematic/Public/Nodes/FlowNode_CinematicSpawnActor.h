#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "FlowNode_CinematicSpawnActor.generated.h"

class AActor;

/**
 * Spawns an actor during cinematic playback
 *
 * This node is restricted to InteractionCinematicFlowAsset only.
 * It won't appear in the node palette of regular FlowAssets.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Spawn Cinematic Actor"))
class INTERACTIONCINEMATIC_API UFlowNode_CinematicSpawnActor : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_CinematicSpawnActor();

#if WITH_EDITOR
	virtual FString GetNodeCategory() const override { return TEXT("Cinematic"); }
#endif

protected:
	/** The actor class to spawn */
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AActor> ActorClass;

	/** Spawn transform */
	UPROPERTY(EditAnywhere, Category = "Spawn")
	FTransform SpawnTransform;

	virtual void ExecuteInput(const FName& PinName) override;
};
