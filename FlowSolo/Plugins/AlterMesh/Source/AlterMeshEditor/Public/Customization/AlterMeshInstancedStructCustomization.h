// Copyright 2023 Aechmea

#pragma once

#include "RHIDefinitions.h"
#include "IDetailChildrenBuilder.h"
#include "InstancedStructDetails.h"
#include "IPropertyTypeCustomization.h"
#include "Templates/SharedPointer.h"
#include "PropertyHandle.h"

class FAlterMeshInstancedStructPropertyTypeIdentifier : public IPropertyTypeIdentifier
{
public:
	FAlterMeshInstancedStructPropertyTypeIdentifier()
	{}

	virtual bool IsPropertyTypeCustomized(const IPropertyHandle& PropertyHandle) const override
	{
		return PropertyHandle.HasMetaData(TEXT("AlterMeshParamCustomDisplay"));
	}
};

struct FAlterMeshInstancedStructCustomization : public FInstancedStructDetails
{
	virtual bool IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle);
	virtual void OnReset(TSharedPtr<IPropertyHandle> PropertyHandle);
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
};

