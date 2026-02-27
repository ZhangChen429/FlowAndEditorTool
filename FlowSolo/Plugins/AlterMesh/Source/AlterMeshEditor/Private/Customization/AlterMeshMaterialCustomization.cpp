// Copyright 2023 Aechmea

#include "Customization/AlterMeshMaterialCustomization.h"

#include "AlterMeshActor.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Text/STextBlock.h"

bool FAlterMeshMaterialCustomization::IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	// todo implement
	return false;
}

void FAlterMeshMaterialCustomization::OnReset(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	// todo implement
}

void FAlterMeshMaterialCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FName SlotName;
	PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAlterMeshMaterial, SlotName))->GetValue(SlotName);

	ChildBuilder.AddCustomRow(FText::FromName(SlotName))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(FText::FromName(SlotName))
	]
	.ValueContent()
	[
		PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAlterMeshMaterial, Material))->CreatePropertyValueWidget()
	];
}

TSharedRef<IPropertyTypeCustomization> FAlterMeshMaterialCustomization::MakeInstance()
{
	return MakeShareable(new FAlterMeshMaterialCustomization);
}
