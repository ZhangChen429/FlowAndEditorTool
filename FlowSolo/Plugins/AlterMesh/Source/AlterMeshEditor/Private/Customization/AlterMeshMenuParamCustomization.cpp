// Copyright 2023 Aechmea

#include "Customization/AlterMeshMenuParamCustomization.h"
#include "IDetailChildrenBuilder.h"
#include "CommonParams/AlterMeshMenuParam.h"
#include "Customization/AlterMeshAssetDetailCustomization.h"
#include "DetailWidgetRow.h"
#include "IPropertyUtilities.h"
#include "Customization/AlterMeshParamCustomization.h"
#include "PropertyHandle.h"
#include "Widgets/Input/STextComboBox.h"

bool FAlterMeshMenuParamCustomization::IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (FAlterMeshMenuParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshMenuParam>(PropertyHandle))
	{
		const FString AssetValue = Param->Data.Value;

		FString ActualValue;
		PropertyHandle->GetValue(ActualValue);

		return PropertyHandle->GetNumOuterObjects() == 1 && AssetValue != ActualValue;
	}

	return false;
}

void FAlterMeshMenuParamCustomization::OnReset(TSharedPtr<IPropertyHandle> PropertyHandle, IPropertyTypeCustomizationUtils* CustomizationUtils)
{
	if (FAlterMeshMenuParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshMenuParam>(PropertyHandle))
	{
		const FString Value = Param->Data.Value;
		PropertyHandle->SetValue(Value);
		if (CustomizationUtils)
		{
			CustomizationUtils->GetPropertyUtilities()->ForceRefresh();
		}
	}
}

void FAlterMeshMenuParamCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
                                                         IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FAlterMeshParamCustomizationUtils::RenameProperty(PropertyHandle, "Value");

	auto EntriesHandle = PropertyHandle->GetChildHandle(FName("Entries"))->AsArray();
	auto ValuesHandle = PropertyHandle->GetChildHandle(FName("Values"))->AsArray();

	uint32 NumEntries = 0;
	EntriesHandle->GetNumElements(NumEntries);

	uint32 NumValues = 0;
	ValuesHandle->GetNumElements(NumValues);

	ensure(NumEntries == NumValues);

	ComboBoxEntries.Empty();
	for (uint32 i = 0; i < NumEntries; i++)
	{
		FString Entry = "";
		EntriesHandle->GetElement(i)->GetValue(Entry);

		int32 Value = 0;
		ValuesHandle->GetElement(i)->GetValue(Value);

		ComboBoxEntries.Add(MakeShared<FString>(Entry));
	}
	
	auto ValueHandle = PropertyHandle->GetChildHandle(FName("Value"));
	FString CurrentValue;
	ValueHandle->GetValue(CurrentValue);
	auto CurrentItem = ComboBoxEntries.FindByPredicate([CurrentValue](const TSharedPtr<FString>& Item)
	{
		return *Item.Get() == CurrentValue;
	});
	
	if (ComboBoxEntries.Num() > 0)
	{
		ChildBuilder.AddProperty(ValueHandle.ToSharedRef())
		.OverrideResetToDefault(FResetToDefaultOverride::Create(
							FIsResetToDefaultVisible::CreateSP(this, &FAlterMeshMenuParamCustomization::IsResetVisible),
							FResetToDefaultHandler::CreateSP(this, &FAlterMeshMenuParamCustomization::OnReset, &CustomizationUtils)))
	
		.CustomWidget()
		.NameContent()	
		[
			ValueHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(STextComboBox)
			.OptionsSource(&ComboBoxEntries)
			.InitiallySelectedItem(CurrentItem ? *CurrentItem : ComboBoxEntries[0])
			.OnSelectionChanged_Lambda([&, ValueHandle](TSharedPtr<FString> NewValue, ESelectInfo::Type InSelectType)
			{
				ValueHandle->SetValue(*NewValue.Get());
			})
		];
	}
}

TSharedRef<IPropertyTypeCustomization> FAlterMeshMenuParamCustomization::MakeInstance()
{
	return MakeShareable(new FAlterMeshMenuParamCustomization);
}
