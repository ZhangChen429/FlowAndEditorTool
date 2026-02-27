// Copyright 2023 Aechmea

#include "Customization/AlterMeshStringParamCustomization.h"

#include "AlterMeshAsset.h"
#include "IDetailChildrenBuilder.h"
#include "CommonParams/AlterMeshStringParam.h"
#include "Customization/AlterMeshAssetDetailCustomization.h"
#include "Customization/AlterMeshParamCustomization.h"

bool FAlterMeshStringParamCustomization::IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshStringParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshStringParam>(PropertyHandle))
	{
		const FString AssetValue = Param->Data.Value;

		FString ActualValue;
		PropertyHandle->GetValue(ActualValue);

		return PropertyHandle->GetNumOuterObjects() == 1 && AssetValue != ActualValue;
	}

	return false;
}

void FAlterMeshStringParamCustomization::OnReset(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshStringParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshStringParam>(PropertyHandle))
	{
		const FString Value = Param->Data.Value;
		PropertyHandle->SetValue(Value);
	}
}

void FAlterMeshStringParamCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FAlterMeshParamCustomizationUtils::RenameProperty(PropertyHandle);
	ChildBuilder.AddProperty(PropertyHandle->GetChildHandle(FName("Value")).ToSharedRef())
	.OverrideResetToDefault(FResetToDefaultOverride::Create(
							FIsResetToDefaultVisible::CreateSP(this, &FAlterMeshStringParamCustomization::IsResetVisible),
							FResetToDefaultHandler::CreateSP(this, &FAlterMeshStringParamCustomization::OnReset)));
}

TSharedRef<IPropertyTypeCustomization> FAlterMeshStringParamCustomization::MakeInstance()
{
	return MakeShareable(new FAlterMeshStringParamCustomization);
}
