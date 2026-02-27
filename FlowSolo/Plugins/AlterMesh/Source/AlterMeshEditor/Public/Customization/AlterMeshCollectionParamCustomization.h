// Copyright 2023 Aechmea

#pragma once

#include "IPropertyTypeCustomization.h"
#include "Templates/SharedPointer.h"
#include "PropertyHandle.h"

struct FAlterMeshCollectionParamCustomization : public IPropertyTypeCustomization
{
	virtual bool IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle);
	virtual void OnReset(TSharedPtr<IPropertyHandle> PropertyHandle, IPropertyTypeCustomizationUtils* CustomizationUtils);
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override {};
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	FReply OnDragDropTarget(const FGeometry& Geometry, const FDragDropEvent& DragDropEvent, TSharedPtr<IPropertyHandle> PropertyHandle, IPropertyTypeCustomizationUtils* CustomizationUtils);
	bool CanDragDropTarget(TSharedPtr<FDragDropOperation> DragDropOperation, TSharedPtr<IPropertyHandle> PropertyHandle);

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
};