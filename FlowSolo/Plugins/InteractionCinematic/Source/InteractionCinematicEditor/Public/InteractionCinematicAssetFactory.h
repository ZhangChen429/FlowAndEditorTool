#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "InteractionCinematicAssetFactory.generated.h"

/**
 * Factory for creating Interaction Cinematic Assets
 */
UCLASS()
class INTERACTIONCINEMATICEDITOR_API UInteractionCinematicAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UInteractionCinematicAssetFactory();

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
	// End of UFactory interface
};
