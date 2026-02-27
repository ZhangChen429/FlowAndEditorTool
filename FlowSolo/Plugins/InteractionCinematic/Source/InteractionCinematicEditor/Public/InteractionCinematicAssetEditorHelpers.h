#pragma once

#include "CoreMinimal.h"

class UInteractionCinematicAsset;

/**
 * Editor helper functions for InteractionCinematicAsset
 */
class INTERACTIONCINEMATICEDITOR_API FInteractionCinematicAssetEditorHelpers
{
public:
	/** Create the embedded Flow Asset and its Graph for the given asset */
	static void CreateFlowAssetForAsset(UInteractionCinematicAsset* Asset);
};
