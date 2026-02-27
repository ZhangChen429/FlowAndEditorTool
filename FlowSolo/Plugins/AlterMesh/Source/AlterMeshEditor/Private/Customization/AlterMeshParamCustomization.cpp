// Copyright 2023 Aechmea

#include "Customization/AlterMeshParamCustomization.h"

#include "AlterMeshActor.h"
#include "AlterMeshAsset.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "GeometryParams/AlterMeshGeometryBase.h"

void FAlterMeshParamCustomizationUtils::RenameProperty(TSharedRef<IPropertyHandle> ParamDataHandle, FName PropertyToRename)
{
	FString ParamName;
	FString Tooltip;
	if (auto BaseStructHandle = ParamDataHandle->GetParentHandle())
	{
		if (auto BaseData = BaseStructHandle->GetChildHandle(FName("BaseData")))
		{
			BaseData->GetChildHandle(FName("Name"))->GetValue(ParamName);
			BaseData->GetChildHandle(FName("Tooltip"))->GetValue(Tooltip);
			ParamDataHandle->GetChildHandle(PropertyToRename)->SetToolTipText(FText::FromString(Tooltip));
			ParamDataHandle->GetChildHandle(PropertyToRename)->SetPropertyDisplayName(FText::FromString(ParamName + (Tooltip.IsEmpty() ? "" : " *")));
		}
	}
}

UAlterMeshAssetInterface* FAlterMeshParamCustomizationUtils::GetAssetFromProperty(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);
	
	if (OuterObjects.Num())
	{
		if (AAlterMeshActor* Actor = Cast<AAlterMeshActor>(OuterObjects[0]))
		{
			return Actor->Asset ? Actor->Asset->Get() : nullptr; 
		}

		if (UAlterMeshAssetInterface* Asset = Cast<UAlterMeshAssetInterface>(OuterObjects[0]))
		{
			return Asset->Get();
		}
	}

	return nullptr;
}

template <typename T>
void FAlterMeshParamCustomizationUtils::SetMinMaxValues(TSharedRef<IPropertyHandle> PropertyHandle)
{
	const TSharedPtr<IPropertyHandle> MinValueProp = PropertyHandle->GetChildHandle(FName("MinValue"));
	const TSharedPtr<IPropertyHandle> MaxValueProp = PropertyHandle->GetChildHandle(FName("MaxValue"));
	TSharedPtr<IPropertyHandle> ValueProp = PropertyHandle->GetChildHandle(FName("Value"));

	if (MinValueProp.IsValid())
	{
		MinValueProp->MarkHiddenByCustomization();
		T MinValue;
		MinValueProp->GetValue(MinValue);

		// big numbers bugs out the ui
		if (FMath::IsNearlyEqual(MinValue, 0.f, MAX_int32/2))
		{
			ValueProp->SetInstanceMetaData(FName("UIMin"), FString::SanitizeFloat(MinValue));
			ValueProp->SetInstanceMetaData(FName("ClampMin"), FString::SanitizeFloat(MinValue));
		}
	}

	if (MaxValueProp.IsValid())
	{
		MaxValueProp->MarkHiddenByCustomization();
		T MaxValue;
		MaxValueProp->GetValue(MaxValue);

		// big numbers bugs out the ui
		if (FMath::IsNearlyEqual(MaxValue, 0.f, MAX_int32/2))
		{
			ValueProp->SetInstanceMetaData(FName("UIMax"), FString::SanitizeFloat(MaxValue));
			ValueProp->SetInstanceMetaData(FName("ClampMax"), FString::SanitizeFloat(MaxValue));
		}
	}
}

template void FAlterMeshParamCustomizationUtils::SetMinMaxValues<int32>(TSharedRef<IPropertyHandle> PropertyHandle);
template void FAlterMeshParamCustomizationUtils::SetMinMaxValues<float>(TSharedRef<IPropertyHandle> PropertyHandle);
