// Copyright 2023 Aechmea

#include "Customization/AlterMeshGeometryParamCustomization.h"

#include "AlterMeshAsset.h"
#include "AssetSelection.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Customization/AlterMeshAssetDetailCustomization.h"
#include "Customization/AlterMeshParamCustomization.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "GeometryParams/AlterMeshGeometryBase.h"
#include "SDropTarget.h"
#include "PropertyCustomizationHelpers.h"
#include "CommonParams/AlterMeshGeometryParam.h"
#include "UObject/UObjectIterator.h"

void FAlterMeshGeometryParamCustomization::OnReset(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	FName ParamId;
	TSharedPtr<IPropertyHandle> StructProperty = PropertyHandle->GetParentHandle();
	StructProperty->GetChildHandle(FName("Id"))->GetValue(ParamId);

	TArray<UObject*> OuterObjects;
	StructProperty->GetOuterObjects(OuterObjects);

	if (!StructProperty->GetBoolMetaData(FName("Collection")))
	{
		if (FAlterMeshGeometryParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshGeometryParam>(PropertyHandle))
		{
			UAlterMeshGeometryBase* InputType = Param->Data.InputType;
			if (InputType)
			{
				UAlterMeshGeometryBase* NewInstance = NewObject<UAlterMeshGeometryBase>(OuterObjects[0], InputType->GetClass());
				PropertyHandle->SetValueFromFormattedString(NewInstance->GetName());
			}
			else
			{
				PropertyHandle->SetValueFromFormattedString("None");
			}
		}
	}
}

void FAlterMeshGeometryParamCustomization::OnDelete(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	const TSharedPtr<IPropertyHandle> GeometryParam = PropertyHandle->GetParentHandle();
	const TSharedPtr<IPropertyHandle> GeometryParamsArray = GeometryParam->GetParentHandle();

	// Inside the collection we can delete, otherwise we just set to None
	if (GeometryParamsArray->GetBoolMetaData(FName("Collection")))
	{
		const int32 Index = GeometryParam->GetIndexInArray();
		if (Index != INDEX_NONE)
		{
			// Clean old param
			PropertyHandle->SetValueFromFormattedString(FString("None"));

			GeometryParamsArray->AsArray()->DeleteItem(Index);
		}
	}
	else
	{
		PropertyHandle->SetValueFromFormattedString(FString("None"));
	}
}

void FAlterMeshGeometryParamCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
															IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

void FAlterMeshGeometryParamCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
															IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FAlterMeshParamCustomizationUtils::RenameProperty(PropertyHandle, GET_MEMBER_NAME_CHECKED(FAlterMeshGeometryParamData, InputType));
	
	TSharedPtr<IPropertyHandle> GeometryInputHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAlterMeshGeometryParamData, InputType));
	TSharedPtr<IPropertyHandle> HandlePtr = PropertyHandle;	
	PropertyHandle->MarkResetToDefaultCustomized();

	ChildBuilder.AddProperty(GeometryInputHandle.ToSharedRef())
	.OverrideResetToDefault(FResetToDefaultOverride::Create(
							FIsResetToDefaultVisible::CreateSP(this, &FAlterMeshGeometryParamCustomization::IsResetVisible),
							FResetToDefaultHandler::CreateSP(this, &FAlterMeshGeometryParamCustomization::OnReset)))
	.CustomWidget(true)
	.NameContent()
	[
		GeometryInputHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(200)
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SDropTarget)
			.OnDropped(this, &FAlterMeshGeometryParamCustomization::OnDragDropTarget, HandlePtr)
			.OnAllowDrop(this, &FAlterMeshGeometryParamCustomization::CanDragDropTarget)
			.OnIsRecognized(this, &FAlterMeshGeometryParamCustomization::CanDragDropTarget)
			[
				GeometryInputHandle->CreatePropertyValueWidget()
			]
		]
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			PropertyCustomizationHelpers::MakeDeleteButton(FSimpleDelegate::CreateSP(this, &FAlterMeshGeometryParamCustomization::OnDelete, GeometryInputHandle))
		]
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			PropertyCustomizationHelpers::MakeUseSelectedButton(FSimpleDelegate::CreateSP(this, &FAlterMeshGeometryParamCustomization::OnUseAsset, GeometryInputHandle))
		]
	];
}

void FAlterMeshGeometryParamCustomization::OnClearProperty(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	PropertyHandle->SetValueFromFormattedString(FString("None"));
}

void FAlterMeshGeometryParamCustomization::OnUseAsset(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	TArray<FAssetData> SelectedAssets;
	AssetSelectionUtils::GetSelectedAssets(SelectedAssets);

	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	if (SelectedAssets.Num())
	{
		for (UObject* Outer : OuterObjects)
		{
			// Creating a drag op here so it uses same mechanism to check for allowed classes
			TSharedRef<FAssetDragDropOp> DragDropOperation = FAssetDragDropOp::New(SelectedAssets[0]);

			for (UAlterMeshGeometryBase* GeometryType : TObjectRange<UAlterMeshGeometryBase>(RF_NoFlags))
			{
				if (GeometryType->HasAnyFlags(RF_ClassDefaultObject))
				{
					if (GeometryType->CanDropOperation(DragDropOperation, 0))
					{
						UAlterMeshGeometryBase* NewInstance = NewObject<UAlterMeshGeometryBase>(Outer, GeometryType->GetClass());
						PropertyHandle->SetValueFromFormattedString(NewInstance->GetName());
						NewInstance->OnDropOperation(DragDropOperation, 0);
					}
				}
			}
		}
	}
}

FReply FAlterMeshGeometryParamCustomization::OnDragDropTarget(const FGeometry& Geometry, const FDragDropEvent& DragDropEvent, TSharedPtr<IPropertyHandle> PropertyHandle)
{
	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	TSharedPtr<IPropertyHandle> GeometryInputHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAlterMeshGeometryParamData, InputType));

	for (UObject* Outer : OuterObjects)
	{
		if (DragDropEvent.GetOperation().IsValid())
		{
			for (UAlterMeshGeometryBase* GeometryType : TObjectRange<UAlterMeshGeometryBase>(RF_NoFlags))
			{
				if (GeometryType->HasAnyFlags(RF_ClassDefaultObject))
				{
					if (GeometryType->CanDropOperation(DragDropEvent.GetOperation(), 0))
					{
						UAlterMeshGeometryBase* NewInstance = NewObject<UAlterMeshGeometryBase>(Outer, GeometryType->GetClass());
						GeometryInputHandle->SetValueFromFormattedString(NewInstance->GetName());
						NewInstance->OnDropOperation(DragDropEvent.GetOperation(), 0);
						return FReply::Handled();
					}
				}
			}
		}
	}

	return FReply::Unhandled();
}

bool FAlterMeshGeometryParamCustomization::CanDragDropTarget(TSharedPtr<FDragDropOperation> DragDropOperation)
{
	bool bAnyGeometryAcceptsAsset = false;

	if (DragDropOperation.IsValid())
	{
		for (UAlterMeshGeometryBase* GeometryType : TObjectRange<UAlterMeshGeometryBase>(RF_NoFlags))
		{
			if (GeometryType->HasAnyFlags(RF_ClassDefaultObject))
			{
				bAnyGeometryAcceptsAsset |= GeometryType->CanDropOperation(DragDropOperation, 0);
			}
		}
	}

	return bAnyGeometryAcceptsAsset;
}

TSharedRef<IPropertyTypeCustomization> FAlterMeshGeometryParamCustomization::MakeInstance()
{
	return MakeShareable(new FAlterMeshGeometryParamCustomization);
}

bool FAlterMeshGeometryParamCustomization::IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	TSharedPtr<IPropertyHandle> StructProperty = PropertyHandle->GetParentHandle();

	// Resetting single instances from a collection is complicated...
	if (StructProperty->GetBoolMetaData(FName("Collection")) || PropertyHandle->GetNumOuterObjects() > 1)
	{
		return false;
	}

	if (FAlterMeshGeometryParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshGeometryParam>(PropertyHandle))
	{
		UObject* AssetValue = Param->Data.InputType;

		UObject* ActualValue = nullptr;
		PropertyHandle->GetValue(ActualValue);

		const bool bEqual = AssetValue == ActualValue
							|| (AssetValue && ActualValue
								&& AssetValue->GetClass()->GetFName() == ActualValue->GetClass()->GetFName());
		return !bEqual;
	}

	return false;
}

