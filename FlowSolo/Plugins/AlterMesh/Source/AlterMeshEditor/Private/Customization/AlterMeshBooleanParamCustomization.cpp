// Copyright 2023 Aechmea

#include "Customization/AlterMeshBooleanParamCustomization.h"

#include "AlterMeshAsset.h"
#include "IDetailChildrenBuilder.h"
#include "CommonParams/AlterMeshBoolParam.h"
#include "Customization/AlterMeshAssetDetailCustomization.h"
#include "Customization/AlterMeshParamCustomization.h"

bool FAlterMeshBooleanParamCustomization::IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshBoolParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshBoolParam>(PropertyHandle))
	{
		const bool AssetValue = Param->Data.Value;

		bool ActualValue;
		PropertyHandle->GetValue(ActualValue);

		return PropertyHandle->GetNumOuterObjects() == 1 && AssetValue != ActualValue;
	}

	return false;
}

void FAlterMeshBooleanParamCustomization::OnReset(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshBoolParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshBoolParam>(PropertyHandle))
	{
		const bool AssetValue = Param->Data.Value;
		PropertyHandle->SetValue(AssetValue);
	}
}

void FAlterMeshBooleanParamCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
															IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FAlterMeshParamCustomizationUtils::RenameProperty(PropertyHandle);
	ChildBuilder.AddProperty(PropertyHandle->GetChildHandle(FName("Value")).ToSharedRef())
	.OverrideResetToDefault(FResetToDefaultOverride::Create(
							FIsResetToDefaultVisible::CreateSP(this, &FAlterMeshBooleanParamCustomization::IsResetVisible),
							FResetToDefaultHandler::CreateSP(this, &FAlterMeshBooleanParamCustomization::OnReset)));
}

TSharedRef<IPropertyTypeCustomization> FAlterMeshBooleanParamCustomization::MakeInstance()
{
	return MakeShareable(new FAlterMeshBooleanParamCustomization);
}
