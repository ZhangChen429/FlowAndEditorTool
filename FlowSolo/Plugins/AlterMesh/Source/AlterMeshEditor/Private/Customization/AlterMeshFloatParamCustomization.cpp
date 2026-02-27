// Copyright 2023 Aechmea

#include "Customization/AlterMeshFloatParamCustomization.h"

#include "AlterMeshAsset.h"
#include "IDetailChildrenBuilder.h"
#include "CommonParams/AlterMeshFloatParam.h"
#include "Customization/AlterMeshAssetDetailCustomization.h"
#include "Customization/AlterMeshParamCustomization.h"

bool FAlterMeshFloatParamCustomization::IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshFloatParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshFloatParam>(PropertyHandle))
	{
		const float AssetValue = Param->Data.Value;

		float ActualValue;
		PropertyHandle->GetValue(ActualValue);

		return PropertyHandle->GetNumOuterObjects() == 1 && !FMath::IsNearlyEqual(AssetValue, ActualValue);
	}

	return false;
}

void FAlterMeshFloatParamCustomization::OnReset(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshFloatParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshFloatParam>(PropertyHandle))
	{
		const float Value = Param->Data.Value;
		PropertyHandle->SetValue(Value);
	}
}

void FAlterMeshFloatParamCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
														IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FAlterMeshParamCustomizationUtils::SetMinMaxValues<float>(PropertyHandle);
	FAlterMeshParamCustomizationUtils::RenameProperty(PropertyHandle);
	ChildBuilder.AddProperty(PropertyHandle->GetChildHandle(FName("Value")).ToSharedRef())
	.OverrideResetToDefault(FResetToDefaultOverride::Create(
							FIsResetToDefaultVisible::CreateSP(this, &FAlterMeshFloatParamCustomization::IsResetVisible),
							FResetToDefaultHandler::CreateSP(this, &FAlterMeshFloatParamCustomization::OnReset)));
}

TSharedRef<IPropertyTypeCustomization> FAlterMeshFloatParamCustomization::MakeInstance()
{
	return MakeShareable(new FAlterMeshFloatParamCustomization);
}