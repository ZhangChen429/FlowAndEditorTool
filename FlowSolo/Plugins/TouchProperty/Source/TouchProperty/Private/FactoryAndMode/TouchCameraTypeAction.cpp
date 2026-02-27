
#include "FactoryAndMode/TouchCameraTypeAction.h"

#include "FactoryAndMode/FTouchCameraAssetToolkit.h"
#include "FactoryAndMode/TouchCameraObjectFactory.h"
#include "TouchProperty.h"


FText FTouchCameraTypeAction::GetName() const
{
	return FText::FromString("TouchCamera Editor Base Class");
}

UClass* FTouchCameraTypeAction::GetSupportedClass() const
{
	return UTouchCameraAsset::StaticClass();
}

 FColor FTouchCameraTypeAction::GetTypeColor() const
{
	return FColor::Silver;
}

 uint32 FTouchCameraTypeAction::GetCategories()
{
	return FTouchPropertyModule::GetAssetCategory();
}
void FTouchCameraTypeAction::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{

	EToolkitMode::Type Mode = EToolkitMode::Standalone;
	
	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Asset = Cast<UTouchCameraAsset>(*ObjIt);
		if (Asset != NULL)
		{
			TSharedRef< FTouchCameraAssetToolkit > NewToolkit(new FTouchCameraAssetToolkit());
			NewToolkit->Initialize(Mode, EditWithinLevelEditor, Asset);
		}
	}
}
