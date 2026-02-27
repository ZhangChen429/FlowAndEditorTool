// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshParam.h"
#include "DetailLayoutBuilder.h"
#include "IDetailCustomization.h"

class AAlterMeshActor;
class SAssetPicker;

class ALTERMESHEDITOR_API FAlterMeshAssetDetailCustomization : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailBuilder ) override;

};
