// Copyright 2023 Aechmea

#include "Customization/AlterMeshTextureParamCustomization.h"

#include "AlterMeshAsset.h"
#include "IDetailChildrenBuilder.h"
#include "CommonParams/AlterMeshTextureParam.h"
#include "Customization/AlterMeshAssetDetailCustomization.h"
#include "Customization/AlterMeshParamCustomization.h"

bool FAlterMeshTextureParamCustomization::IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshTextureParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshTextureParam>(PropertyHandle))
	{
		const UObject* AssetValue = Param->Data.Value;

		UObject* ActualValue;
		PropertyHandle->GetValue(ActualValue);

		return PropertyHandle->GetNumOuterObjects() == 1 && AssetValue != ActualValue;
	}

	return false;
}

void FAlterMeshTextureParamCustomization::OnReset(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshTextureParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshTextureParam>(PropertyHandle))
	{
		const UObject* Value = Param->Data.Value;
		PropertyHandle->SetValue(Value);
	}
}

void FAlterMeshTextureParamCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FAlterMeshParamCustomizationUtils::RenameProperty(PropertyHandle);
	ChildBuilder.AddProperty(PropertyHandle->GetChildHandle(FName("Value")).ToSharedRef())
	.OverrideResetToDefault(FResetToDefaultOverride::Create(
							FIsResetToDefaultVisible::CreateSP(this, &FAlterMeshTextureParamCustomization::IsResetVisible),
							FResetToDefaultHandler::CreateSP(this, &FAlterMeshTextureParamCustomization::OnReset)));
}

TSharedRef<IPropertyTypeCustomization> FAlterMeshTextureParamCustomization::MakeInstance()
{
	return MakeShareable(new FAlterMeshTextureParamCustomization);
}