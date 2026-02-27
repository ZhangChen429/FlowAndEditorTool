// Copyright 2023 Aechmea

#include "Customization/AlterMeshIntParamCustomization.h"

#include "AlterMeshAsset.h"
#include "IDetailChildrenBuilder.h"
#include "CommonParams/AlterMeshIntParam.h"
#include "Customization/AlterMeshAssetDetailCustomization.h"
#include "Customization/AlterMeshParamCustomization.h"
#include "PropertyHandle.h"

bool FAlterMeshIntParamCustomization::IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshIntParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshIntParam>(PropertyHandle))
	{
		const int32 AssetValue = Param->Data.Value;

		int32 ActualValue;
		PropertyHandle->GetValue(ActualValue);

		return PropertyHandle->GetNumOuterObjects() == 1 && AssetValue != ActualValue;
	}

	return false;
}

void FAlterMeshIntParamCustomization::OnReset(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshIntParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshIntParam>(PropertyHandle))
	{
		const int32 Value = Param->Data.Value;
		PropertyHandle->SetValue(Value);
	}
}

void FAlterMeshIntParamCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
														IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FAlterMeshParamCustomizationUtils::SetMinMaxValues<int32>(PropertyHandle);
	FAlterMeshParamCustomizationUtils::RenameProperty(PropertyHandle);
	ChildBuilder.AddProperty(PropertyHandle->GetChildHandle(FName("Value")).ToSharedRef())
	.OverrideResetToDefault(FResetToDefaultOverride::Create(
							FIsResetToDefaultVisible::CreateSP(this, &FAlterMeshIntParamCustomization::IsResetVisible),
							FResetToDefaultHandler::CreateSP(this, &FAlterMeshIntParamCustomization::OnReset)));
}

TSharedRef<IPropertyTypeCustomization> FAlterMeshIntParamCustomization::MakeInstance()
{
	return MakeShareable(new FAlterMeshIntParamCustomization);
}
