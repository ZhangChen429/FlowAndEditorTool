#include "InteractionCinematicAsset.h"
#include "InteractionCinematicFlowAsset.h"

UInteractionCinematicAsset::UInteractionCinematicAsset()
{
	Sequence = nullptr;
}

#if WITH_EDITOR

void UInteractionCinematicAsset::PostInitProperties()
{
	Super::PostInitProperties();
	// Graph creation is handled by the editor module
}

void UInteractionCinematicAsset::PostLoad()
{
	Super::PostLoad();
	// Graph creation is handled by the editor module
}

#endif
