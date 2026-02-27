// Fill out your copyright notice in the Description page of Project Settings.


#include "FactoryAndMode/TouchCameraObjectFactory.h"

#include "Graph/CameraGraphSchema.h"

#define LOCTEXT_NAMESPACE "TouchCameraAssetFactory"
bool UTouchCameraObjectFactory::ConfigureProperties()
{
	SupportedClass = UTouchCameraAsset::StaticClass();
	bCreateNew = true;
	bEditorImport = false;
	bEditAfterNew = true;
	const FText TitleText = LOCTEXT("TouchCameraAssetFactory", "Pick TouchCameraAsset Asset Class");
	return true;
}

UObject* UTouchCameraObjectFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName,
                                                     EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UTouchCameraAsset* NewFlowAsset= NewObject<UTouchCameraAsset>(InParent, InName, Flags);
	
	return NewFlowAsset;
	
}
#undef LOCTEXT_NAMESPACE