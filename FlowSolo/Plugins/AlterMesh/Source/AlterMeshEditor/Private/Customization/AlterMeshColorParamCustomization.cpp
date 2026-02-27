// Copyright 2023 Aechmea

#include "Customization/AlterMeshColorParamCustomization.h"

#include "AlterMeshAsset.h"
#include "IDetailChildrenBuilder.h"
#include "CommonParams/AlterMeshColorParam.h"
#include "Customization/AlterMeshAssetDetailCustomization.h"
#include "Customization/AlterMeshParamCustomization.h"
#include "PropertyHandle.h"

bool FAlterMeshColorParamCustomization::IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshColorParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshColorParam>(PropertyHandle))
	{
		const FLinearColor AssetValue = Param->Data.Value;

		void* Data;
		PropertyHandle->GetValueData(Data);
		FLinearColor* ActualValue = reinterpret_cast<FLinearColor*>(Data);

		return PropertyHandle->GetNumOuterObjects() == 1 && ActualValue && !AssetValue.Equals(*ActualValue);
	}

	return false;
}

void FAlterMeshColorParamCustomization::OnReset(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshColorParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshColorParam>(PropertyHandle))
	{
		const FLinearColor Value = Param->Data.Value;
		PropertyHandle->SetValueFromFormattedString(Value.ToString());
	}	
}

void FAlterMeshColorParamCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
														IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FAlterMeshParamCustomizationUtils::SetMinMaxValues<float>(PropertyHandle);
	FAlterMeshParamCustomizationUtils::RenameProperty(PropertyHandle);
	ChildBuilder.AddProperty(PropertyHandle->GetChildHandle(FName("Value")).ToSharedRef())
	.OverrideResetToDefault(FResetToDefaultOverride::Create(
							FIsResetToDefaultVisible::CreateSP(this, &FAlterMeshColorParamCustomization::IsResetVisible),
							FResetToDefaultHandler::CreateSP(this, &FAlterMeshColorParamCustomization::OnReset)));
}

TSharedRef<IPropertyTypeCustomization> FAlterMeshColorParamCustomization::MakeInstance()
{
	return MakeShareable(new FAlterMeshColorParamCustomization);
}