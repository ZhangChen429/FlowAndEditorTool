// Copyright 2023 Aechmea

#include "Customization/AlterMeshAssetDetailCustomization.h"

#include "AlterMeshActor.h"
#include "AlterMeshAsset.h"
#include "AlterMeshConverter.h"
#include "DetailCategoryBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "AlterMeshAssetParamDetails"

TSharedRef<IDetailCustomization> FAlterMeshAssetDetailCustomization::MakeInstance()
{
	return MakeShareable(new FAlterMeshAssetDetailCustomization);
}

void FAlterMeshAssetDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	
}

#undef LOCTEXT_NAMESPACE
