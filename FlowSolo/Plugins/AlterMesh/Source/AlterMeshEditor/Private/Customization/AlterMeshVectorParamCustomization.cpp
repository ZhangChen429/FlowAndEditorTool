// Copyright 2023 Aechmea

#include "Customization/AlterMeshVectorParamCustomization.h"

#include "AlterMeshAsset.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "CommonParams/AlterMeshVectorParam.h"
#include "Customization/AlterMeshAssetDetailCustomization.h"
#include "Customization/AlterMeshParamCustomization.h"

bool FAlterMeshVectorParamCustomization::IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshVectorParam* AssetParam = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshVectorParam>(PropertyHandle))
	{
		const FVector AssetValue = AssetParam->Data.Value;

		void* Data;
		PropertyHandle->GetValueData(Data);
		FVector* ActualValue = static_cast<FVector*>(Data);

		return PropertyHandle->GetNumOuterObjects() == 1 && ActualValue && AssetValue != *ActualValue;
	}

	return false;
}

void FAlterMeshVectorParamCustomization::OnReset(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshVectorParam* AssetParam = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshVectorParam>(PropertyHandle))
	{
		const FVector Value = AssetParam->Data.Value;
	
		void* Data;
		PropertyHandle->GetValueData(Data);
		FVector* PropertyValue = reinterpret_cast<FVector*>(Data);
	
		PropertyHandle->NotifyPreChange();
		if (PropertyValue)
		{
			*PropertyValue = Value;
		}
		PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	}
}

void FAlterMeshVectorParamCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
															IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FAlterMeshParamCustomizationUtils::RenameProperty(PropertyHandle);

	auto ValueProperty = PropertyHandle->GetChildHandle(FName("Value")).ToSharedRef();
	auto CoordinateProperty = PropertyHandle->GetChildHandle(FName("bConvertCoordinateSpace")).ToSharedRef();
	auto XProperty = ValueProperty->GetChildHandle(FName("X")).ToSharedRef();
	auto YProperty = ValueProperty->GetChildHandle(FName("Y")).ToSharedRef();
	auto ZProperty = ValueProperty->GetChildHandle(FName("Z")).ToSharedRef();

	
	const TSharedPtr<IPropertyHandle> MinValueProp = PropertyHandle->GetChildHandle(FName("MinValue"));
	const TSharedPtr<IPropertyHandle> MaxValueProp = PropertyHandle->GetChildHandle(FName("MaxValue"));
	TSharedPtr<IPropertyHandle> ValueProp = PropertyHandle->GetChildHandle(FName("Value"));

	if (MinValueProp.IsValid())
	{
		MinValueProp->MarkHiddenByCustomization();
		float MinValue;
		MinValueProp->GetValue(MinValue);

		// big numbers bugs out the ui
		if (FMath::IsNearlyEqual(MinValue, 0.f, MAX_int32/2))
		{
			ValueProp->SetInstanceMetaData(FName("UIMin"), FString::SanitizeFloat(MinValue));
			ValueProp->SetInstanceMetaData(FName("ClampMin"), FString::SanitizeFloat(MinValue));
			XProperty->SetInstanceMetaData(FName("UIMin"), FString::SanitizeFloat(MinValue));
			XProperty->SetInstanceMetaData(FName("ClampMin"), FString::SanitizeFloat(MinValue));
			YProperty->SetInstanceMetaData(FName("UIMin"), FString::SanitizeFloat(MinValue));
			YProperty->SetInstanceMetaData(FName("ClampMin"), FString::SanitizeFloat(MinValue));
			ZProperty->SetInstanceMetaData(FName("UIMin"), FString::SanitizeFloat(MinValue));
			ZProperty->SetInstanceMetaData(FName("ClampMin"), FString::SanitizeFloat(MinValue));
		}
	}

	if (MaxValueProp.IsValid())
	{
		MaxValueProp->MarkHiddenByCustomization();
		float MaxValue;
		MaxValueProp->GetValue(MaxValue);

		// big numbers bugs out the ui
		if (FMath::IsNearlyEqual(MaxValue, 0.f, MAX_int32/2))
		{
			ValueProp->SetInstanceMetaData(FName("UIMax"), FString::SanitizeFloat(MaxValue));
			ValueProp->SetInstanceMetaData(FName("ClampMax"), FString::SanitizeFloat(MaxValue));
			XProperty->SetInstanceMetaData(FName("UIMax"), FString::SanitizeFloat(MaxValue));
			XProperty->SetInstanceMetaData(FName("ClampMax"), FString::SanitizeFloat(MaxValue));
			YProperty->SetInstanceMetaData(FName("UIMax"), FString::SanitizeFloat(MaxValue));
			YProperty->SetInstanceMetaData(FName("ClampMax"), FString::SanitizeFloat(MaxValue));
			ZProperty->SetInstanceMetaData(FName("UIMax"), FString::SanitizeFloat(MaxValue));
			ZProperty->SetInstanceMetaData(FName("ClampMax"), FString::SanitizeFloat(MaxValue));
		}
	}
	
	ChildBuilder.AddProperty(PropertyHandle->GetChildHandle(FName("Value")).ToSharedRef())
	.OverrideResetToDefault(FResetToDefaultOverride::Create(
							FIsResetToDefaultVisible::CreateSP(this, &FAlterMeshVectorParamCustomization::IsResetVisible),
							FResetToDefaultHandler::CreateSP(this, &FAlterMeshVectorParamCustomization::OnReset)))
	.CustomWidget(true)
	.NameContent()
	[
		ValueProperty->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(300.f)
	[		
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.FillWidth(1.f)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()		
			[
				XProperty->CreatePropertyNameWidget()
			]
			+SHorizontalBox::Slot()
			.FillWidth(1.f)		
			[
				XProperty->CreatePropertyValueWidget()
			]
			+SHorizontalBox::Slot()
			.AutoWidth()		
			[
				YProperty->CreatePropertyNameWidget()
			]
			+SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				YProperty->CreatePropertyValueWidget()
			]
			+SHorizontalBox::Slot()
			.AutoWidth()		
			[
				ZProperty->CreatePropertyNameWidget()
			]
			+SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				ZProperty->CreatePropertyValueWidget()
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				CoordinateProperty->CreatePropertyNameWidget(FText::FromString("Convert"))
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				CoordinateProperty->CreatePropertyValueWidget()
			]
		]
	];	
	
	ValueProperty->GetChildHandle(FName("X"))->MarkResetToDefaultCustomized();
	ValueProperty->GetChildHandle(FName("Y"))->MarkResetToDefaultCustomized();
	ValueProperty->GetChildHandle(FName("Z"))->MarkResetToDefaultCustomized();

}

TSharedRef<IPropertyTypeCustomization> FAlterMeshVectorParamCustomization::MakeInstance()
{
	return MakeShareable(new FAlterMeshVectorParamCustomization);
}
