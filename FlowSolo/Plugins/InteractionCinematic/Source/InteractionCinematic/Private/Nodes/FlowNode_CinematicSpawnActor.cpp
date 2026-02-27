#include "Nodes/FlowNode_CinematicSpawnActor.h"
#include "InteractionCinematicFlowAsset.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlowNode_CinematicSpawnActor)

UFlowNode_CinematicSpawnActor::UFlowNode_CinematicSpawnActor()
{
#if WITH_EDITOR
	Category = TEXT("Cinematic");
#endif

	// ✅ KEY: Restrict this node to only work in InteractionCinematicFlowAsset
	// This node will NOT appear in the palette of regular FlowAssets
	AllowedAssetClasses.Empty();
	AllowedAssetClasses.Add(UInteractionCinematicFlowAsset::StaticClass());

	InputPins = {TEXT("In")};
	OutputPins = {TEXT("Out"), TEXT("Failed")};
}

void UFlowNode_CinematicSpawnActor::ExecuteInput(const FName& PinName)
{
	if (!ActorClass)
	{
		LogError(TEXT("ActorClass is not set!"));
		TriggerOutput(TEXT("Failed"), true);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		LogError(TEXT("Failed to get World!"));
		TriggerOutput(TEXT("Failed"), true);
		return;
	}

	// Spawn the actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParams);

	if (SpawnedActor)
	{
		TriggerOutput(TEXT("Out"), true);
	}
	else
	{
		LogError(TEXT("Failed to spawn actor!"));
		TriggerOutput(TEXT("Failed"), true);
	}
}
