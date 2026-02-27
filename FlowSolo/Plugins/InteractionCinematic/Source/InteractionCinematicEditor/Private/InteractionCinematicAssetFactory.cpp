#include "InteractionCinematicAssetFactory.h"
#include "InteractionCinematicAsset.h"

UInteractionCinematicAssetFactory::UInteractionCinematicAssetFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UInteractionCinematicAsset::StaticClass();
}

UObject* UInteractionCinematicAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UInteractionCinematicAsset* NewAsset = NewObject<UInteractionCinematicAsset>(InParent, InClass, InName, Flags);
	return NewAsset;
}

bool UInteractionCinematicAssetFactory::ShouldShowInNewMenu() const
{
	return true;
}
