// Copyright 2023 Aechmea

#pragma once

#include "IPropertyTypeCustomization.h"
#include "Templates/SharedPointer.h"
#include "PropertyHandle.h"

struct FAlterMeshGeometryParamCustomization : public IPropertyTypeCustomization
{
	virtual bool IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle);
	virtual void OnReset(TSharedPtr<IPropertyHandle> PropertyHandle);
	virtual void OnDelete(TSharedPtr<IPropertyHandle> PropertyHandle);
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	FReply OnDragDropTarget(const FGeometry& Geometry, const FDragDropEvent& DragDropEvent, TSharedPtr<IPropertyHandle> PropertyHandle);
	bool CanDragDropTarget(TSharedPtr<FDragDropOperation> DragDropOperation);
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	void OnClearProperty(TSharedPtr<IPropertyHandle> PropertyHandle);
	void OnUseAsset(TSharedPtr<IPropertyHandle> PropertyHandle);

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
};
