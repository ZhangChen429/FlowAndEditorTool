// Copyright 2023 Aechmea

#pragma once

#include "IPropertyTypeCustomization.h"
#include "Templates/SharedPointer.h"
#include "PropertyHandle.h"

struct FAlterMeshTextureParamCustomization : public IPropertyTypeCustomization
{
	virtual bool IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle);
	virtual void OnReset(TSharedPtr<IPropertyHandle> PropertyHandle);
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override {};
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
};