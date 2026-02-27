#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UInteractionCinematicAsset;

/**
 * Slate widget for editing Interaction Cinematic Assets
 */
class SInteractionCinematicAssetEditorWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInteractionCinematicAssetEditorWidget) {}
		SLATE_ARGUMENT(UInteractionCinematicAsset*, Asset)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	/** The asset being edited */
	UInteractionCinematicAsset* Asset;

	/** Get the sequence name text */
	FText GetSequenceNameText() const;

	/** Get the description text */
	FText GetDescriptionText() const;
};
