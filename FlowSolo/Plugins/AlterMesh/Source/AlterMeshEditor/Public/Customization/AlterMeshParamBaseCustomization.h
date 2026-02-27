// Copyright 2023 Aechmea

#pragma once

#include "IPropertyTypeCustomization.h"
#include "Templates/SharedPointer.h"
#include "PropertyHandle.h"

struct FAlterMeshParamBaseCustomization : public IPropertyTypeCustomization
{
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override {};
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override {};

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
};
