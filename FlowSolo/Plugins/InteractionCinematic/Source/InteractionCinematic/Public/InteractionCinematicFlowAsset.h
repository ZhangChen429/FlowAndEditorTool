#pragma once

#include "CoreMinimal.h"
#include "FlowAsset.h"
#include "InteractionCinematicFlowAsset.generated.h"

/**
 * Custom FlowAsset for InteractionCinematic
 *
 * This FlowAsset subclass allows nodes to restrict themselves to only work within
 * InteractionCinematic context by using AllowedAssetClasses.
 *
 * Example:
 *   A node can set: AllowedAssetClasses.Add(UInteractionCinematicFlowAsset::StaticClass());
 *   This node will only appear in InteractionCinematic's node palette.
 */
UCLASS()
class INTERACTIONCINEMATIC_API UInteractionCinematicFlowAsset : public UFlowAsset
{
	GENERATED_BODY()

public:
	UInteractionCinematicFlowAsset(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
