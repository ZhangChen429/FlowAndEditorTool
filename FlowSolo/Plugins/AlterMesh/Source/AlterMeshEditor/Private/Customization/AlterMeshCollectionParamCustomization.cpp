// Copyright 2023 Aechmea

#include "Customization/AlterMeshCollectionParamCustomization.h"

#include "AlterMeshAsset.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "IPropertyUtilities.h"
#include "PropertyCustomizationHelpers.h"
#include "CommonParams/AlterMeshCollectionParam.h"
#include "Customization/AlterMeshParamCustomization.h"
#include "SDropTarget.h"
#include "GeometryParams/AlterMeshGeometryBase.h"
#include "UObject/UObjectIterator.h"

bool FAlterMeshCollectionParamCustomization::IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	// todo check for all entries
	if (FAlterMeshCollectionParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshCollectionParam>(PropertyHandle))
	{
		const int32 AssetNumEntries = Param->Data.Geometries.Num();

		uint32 ActualNumEntries;
		PropertyHandle->AsArray()->GetNumElements(ActualNumEntries);

		return PropertyHandle->GetNumOuterObjects() == 1 && AssetNumEntries != ActualNumEntries;
	}

	return false;
}

void FAlterMeshCollectionParamCustomization::OnReset(TSharedPtr<IPropertyHandle> PropertyHandle, IPropertyTypeCustomizationUtils* CustomizationUtils)
{
	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	if (FAlterMeshCollectionParam* Param = FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle<FAlterMeshCollectionParam>(PropertyHandle))
	{
		const TArray<FAlterMeshGeometryParam>& Geometries = Param->Data.Geometries;

		// Is there an array to formatted string?
		FString FormattedString = TEXT("(");
		bool bFirstEntry = true;

		// Create new instances
		for (const FAlterMeshGeometryParam& Instance : Geometries)
		{
			if (!bFirstEntry)
			{
				FormattedString += ",";
			}
			bFirstEntry = false;

			if (Instance.Data.InputType)
			{
				UAlterMeshGeometryBase* NewInstance = NewObject<UAlterMeshGeometryBase>(OuterObjects[0], Instance.Data.InputType->GetClass());
				FormattedString += FString("(") + GET_MEMBER_NAME_STRING_CHECKED(FAlterMeshGeometryParamData, InputType) + FString("=") + NewInstance->GetName() + FString(")");
			}
		}

		FormattedString += ")";

		PropertyHandle->SetValueFromFormattedString(FormattedString);
	}

	CustomizationUtils->GetPropertyUtilities()->ForceRefresh();
}

void FAlterMeshCollectionParamCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
																IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FAlterMeshParamCustomizationUtils::RenameProperty(PropertyHandle, GET_MEMBER_NAME_CHECKED(FAlterMeshCollectionParamData, Geometries));
	
	TSharedPtr<IPropertyHandle> HandlePtr = PropertyHandle;
	TSharedPtr<IPropertyHandle> GeometriesHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAlterMeshCollectionParamData, Geometries));
	ChildBuilder.AddProperty(GeometriesHandle.ToSharedRef())
	.ShouldAutoExpand(true)
	.ShowPropertyButtons(false)
	.OverrideResetToDefault(FResetToDefaultOverride::Create(
							FIsResetToDefaultVisible::CreateSP(this, &FAlterMeshCollectionParamCustomization::IsResetVisible),
							FResetToDefaultHandler::CreateSP(this, &FAlterMeshCollectionParamCustomization::OnReset, &CustomizationUtils)))
	.CustomWidget(true)
	.NameContent()
	[
		GeometriesHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SDropTarget)
		.OnDropped(this, &FAlterMeshCollectionParamCustomization::OnDragDropTarget, HandlePtr, &CustomizationUtils)
		.OnAllowDrop(this, &FAlterMeshCollectionParamCustomization::CanDragDropTarget, HandlePtr)
		.OnIsRecognized(this, &FAlterMeshCollectionParamCustomization::CanDragDropTarget, HandlePtr)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Padding(6, 0)
			[
				SNew(STextBlock)
				.Text_Lambda([GeometriesHandle]()
				{
					TSharedPtr<IPropertyHandleArray> Array = GeometriesHandle->AsArray();
					uint32 NumElements;
					Array->GetNumElements(NumElements);
					return FText::FromString(FString::FromInt(NumElements));
				})
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				PropertyCustomizationHelpers::MakeAddButton(FSimpleDelegate::CreateLambda([GeometriesHandle]()
				{
					TSharedPtr<IPropertyHandleArray> Array = GeometriesHandle->AsArray();
					if (Array.IsValid())
					{
						Array->AddItem();
					}
				}))
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				PropertyCustomizationHelpers::MakeEmptyButton(FSimpleDelegate::CreateLambda([GeometriesHandle]()
				{
					TSharedPtr<IPropertyHandleArray> Array = GeometriesHandle->AsArray();
					if (Array.IsValid())
					{
						// Custom empty to cleanup old params before emptying
						uint32 NumElements;
						Array->GetNumElements(NumElements);

						for (uint32 i = 0; i < NumElements; i++)
						{
							TSharedPtr<IPropertyHandle> InputType = Array->GetElement(i)->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAlterMeshGeometryParamData, InputType));
							InputType->SetValueFromFormattedString(FString("None"));
						}

						Array->EmptyArray();
					}
				}))
			]
		]
	];
}

FReply FAlterMeshCollectionParamCustomization::OnDragDropTarget(const FGeometry& Geometry, const FDragDropEvent& DragDropEvent, TSharedPtr<IPropertyHandle> PropertyHandle, IPropertyTypeCustomizationUtils* CustomizationUtils)
{
	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	TSharedPtr<IPropertyHandle> GeometryParams = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAlterMeshCollectionParamData, Geometries));

	bool bHandled = false;

	// Instances must be created with the selected object as outer
	// So we disallow dragging to multiple objects
	if (OuterObjects.Num() == 1
		&& DragDropEvent.GetOperation().IsValid())
	{
		// Checks if any geometry CDO allows dropping
		for (UAlterMeshGeometryBase* GeometryType : TObjectRange<UAlterMeshGeometryBase>(RF_NoFlags))
		{
			if (GeometryType->HasAnyFlags(RF_ClassDefaultObject))
			{
				int32 OperationIndex = 0;
				while (GeometryType->CanDropOperation(DragDropEvent.GetOperation(), OperationIndex))
				{
					UAlterMeshGeometryBase* NewInstance = NewObject<UAlterMeshGeometryBase>(OuterObjects[0], GeometryType->GetClass());
					GeometryParams->AsArray()->AddItem();
					uint32 ArrayNum;
					GeometryParams->AsArray()->GetNumElements(ArrayNum);

					TSharedPtr<IPropertyHandle> GeometryInput = GeometryParams->AsArray()->GetElement(ArrayNum-1)->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAlterMeshGeometryParamData, InputType));
					GeometryInput->SetValueFromFormattedString(NewInstance->GetName());
					NewInstance->OnDropOperation(DragDropEvent.GetOperation(), OperationIndex);

					bHandled = true;
					OperationIndex++;
				}
			}
		}
	}

	if (bHandled)
	{
		CustomizationUtils->GetPropertyUtilities()->ForceRefresh();
	}

	return bHandled ? FReply::Handled() : FReply::Unhandled();
}

bool FAlterMeshCollectionParamCustomization::CanDragDropTarget(TSharedPtr<FDragDropOperation> DragDropOperation, TSharedPtr<IPropertyHandle> PropertyHandle)
{
	bool bAnyGeometryAcceptsAsset = false;

	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	// Instances must be created with the selected object as outer
	// So we disallow dragging to multiple objects
	if ( OuterObjects.Num() == 1
		&& DragDropOperation.IsValid())
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

TSharedRef<IPropertyTypeCustomization> FAlterMeshCollectionParamCustomization::MakeInstance()
{
	return MakeShareable(new FAlterMeshCollectionParamCustomization);
}
