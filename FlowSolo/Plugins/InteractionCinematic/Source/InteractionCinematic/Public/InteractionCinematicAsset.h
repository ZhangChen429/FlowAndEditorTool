#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InteractionCinematicAsset.generated.h"

class ULevelSequence;
class UInteractionCinematicFlowAsset;

/**
 * Interaction Cinematic Asset that contains a Level Sequence and Flow Graph
 */
UCLASS(BlueprintType)
class INTERACTIONCINEMATIC_API UInteractionCinematicAsset : public UObject
{
	GENERATED_BODY()

public:
	UInteractionCinematicAsset();

	/** The Level Sequence associated with this asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	TObjectPtr<ULevelSequence> Sequence;

	/** Optional description of this cinematic interaction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	FText Description;

	//////////////////////////////////////////////////////////////////////////
	// Flow Graph

private:
	friend class FInteractionCinematicAssetEditorHelpers;

	/** Embedded InteractionCinematicFlowAsset that owns the Flow Graph */
	UPROPERTY()
	TObjectPtr<UInteractionCinematicFlowAsset> EmbeddedFlowAsset;

public:
	/** Get the embedded Flow Asset */
	UInteractionCinematicFlowAsset* GetFlowAsset() const { return EmbeddedFlowAsset; }

#if WITH_EDITOR
	// UObject interface
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	// End of UObject interface
#endif
};
