// Copyright 2023 Aechmea

#include "Customization/AlterMeshInstancedStructCustomization.h"

#include "AlterMeshActor.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "InstancedStructDetails.h"

bool FAlterMeshInstancedStructCustomization::IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	return false;
}

void FAlterMeshInstancedStructCustomization::OnReset(TSharedPtr<IPropertyHandle> PropertyHandle)
{
}

void FAlterMeshInstancedStructCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FDetailWidgetRow Dummy;
	FInstancedStructDetails::CustomizeHeader(PropertyHandle, Dummy, CustomizationUtils);
}

void FAlterMeshInstancedStructCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
                                                               IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FInstancedStructDetails::CustomizeChildren(PropertyHandle, ChildBuilder, CustomizationUtils);
}

TSharedRef<IPropertyTypeCustomization> FAlterMeshInstancedStructCustomization::MakeInstance()
{
	return MakeShareable(new FAlterMeshInstancedStructCustomization);
}
