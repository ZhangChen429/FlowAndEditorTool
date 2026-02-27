// Copyright 2023 Aechmea

#include "Customization/AlterMeshRotationParamCustomization.h"

#include "AlterMeshAsset.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "CommonParams/AlterMeshRotationParam.h"
#include "Customization/AlterMeshAssetDetailCustomization.h"
#include "Customization/AlterMeshParamCustomization.h"

bool FAlterMeshRotationParamCustomization::IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshRotationParam* AssetParam = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshRotationParam>(PropertyHandle))
	{
		const FRotator AssetValue = AssetParam->Data.Value;

		void* Data;
		PropertyHandle->GetValueData(Data);
		FRotator* ActualValue = reinterpret_cast<FRotator*>(Data);

		return PropertyHandle->GetNumOuterObjects() == 1 && ActualValue && AssetValue != *ActualValue;
	}

	return false;
}

void FAlterMeshRotationParamCustomization::OnReset(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshRotationParam* AssetParam = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshRotationParam>(PropertyHandle))
	{
		const FRotator Value = AssetParam->Data.Value;
	
		void* Data;
		PropertyHandle->GetValueData(Data);
		FRotator* PropertyValue = reinterpret_cast<FRotator*>(Data);
	
		PropertyHandle->NotifyPreChange();
		if (PropertyValue)
		{
			*PropertyValue = Value;
		}
		PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	}
}

void FAlterMeshRotationParamCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
															IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FAlterMeshParamCustomizationUtils::RenameProperty(PropertyHandle);

	auto ValueProperty = PropertyHandle->GetChildHandle(FName("Value")).ToSharedRef();
	auto CoordinateProperty = PropertyHandle->GetChildHandle(FName("bConvertCoordinateSpace")).ToSharedRef();
	auto XProperty = ValueProperty->GetChildHandle(FName("Roll")).ToSharedRef();
	auto YProperty = ValueProperty->GetChildHandle(FName("Pitch")).ToSharedRef();
	auto ZProperty = ValueProperty->GetChildHandle(FName("Yaw")).ToSharedRef();

	
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
							FIsResetToDefaultVisible::CreateSP(this, &FAlterMeshRotationParamCustomization::IsResetVisible),
							FResetToDefaultHandler::CreateSP(this, &FAlterMeshRotationParamCustomization::OnReset)))
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
			.Padding(6.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString("|"))
				.ColorAndOpacity(FLinearColor::Red)
			]
			+SHorizontalBox::Slot()
			.FillWidth(1.f)		
			[
				XProperty->CreatePropertyValueWidget()
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(6.f)
			[
				SNew(STextBlock)	
				.Text(FText::FromString("|"))
				.ColorAndOpacity(FLinearColor::Green)
			]
			+SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				YProperty->CreatePropertyValueWidget()
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(6.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString("|"))
				.ColorAndOpacity(FLinearColor(0.0f, 0.6f, 1.f))		
			]
			+SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				ZProperty->CreatePropertyValueWidget()
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f)
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
	
	ValueProperty->GetChildHandle(FName("Roll"))->MarkResetToDefaultCustomized();
	ValueProperty->GetChildHandle(FName("Pitch"))->MarkResetToDefaultCustomized();
	ValueProperty->GetChildHandle(FName("Yaw"))->MarkResetToDefaultCustomized();

}

TSharedRef<IPropertyTypeCustomization> FAlterMeshRotationParamCustomization::MakeInstance()
{
	return MakeShareable(new FAlterMeshRotationParamCustomization);
}
