// Copyright 2023 Aechmea

#include "Customization/AlterMeshActorDetailCustomization.h"

#include "AlterMeshActor.h"
#include "AlterMeshAsset.h"
#include "AlterMeshConverter.h"
#include "ClassViewerFilter.h"
#include "DetailCategoryBuilder.h"
#include "Editor.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "PropertyCustomizationHelpers.h"
#include "Kismet2/SClassPickerDialog.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "AlterMeshActorParamDetails"

class FAlterMeshConverterClassFilter : public IClassViewerFilter
{
	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< class FClassViewerFilterFuncs > InFilterFuncs ) override
	{
		return InClass->IsChildOf(UAlterMeshConverterBase::StaticClass()) && !InClass->HasAnyClassFlags(CLASS_Abstract);
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const class IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< class FClassViewerFilterFuncs > InFilterFuncs) override
	{
		return InUnloadedClassData->IsChildOf(UAlterMeshConverterBase::StaticClass()) &&  !InUnloadedClassData->HasAnyClassFlags(CLASS_Abstract);
	}
};


TSharedRef<IDetailCustomization> FAlterMeshActorDetailCustomization::MakeInstance()
{
	return MakeShareable(new FAlterMeshActorDetailCustomization);
}

void FAlterMeshActorDetailCustomization::GenerateParamWidget(TSharedRef<IPropertyHandle> PropertyHandle, int32 ArrayIndex, IDetailChildrenBuilder& ChildrenBuilder, bool bEnabled)
{
	FString Panel = "";

	TArray<void*> RawDatas;
	PropertyHandle->AccessRawData(RawDatas);

	FInstancedStruct* Struct = reinterpret_cast<FInstancedStruct*>(RawDatas[0]);
	if (Struct && Struct->GetPtr<FAlterMeshParamBase>())
	{
		Panel = Struct->GetMutable<FAlterMeshParamBase>().BaseData.Panel;
	}
	
	if (Panel.IsEmpty())
	{
		ChildrenBuilder.AddProperty(PropertyHandle)
		.IsEnabled(bEnabled);
	}
	else
	{
		FName CategoryName = FName("Parameters -> " + Panel);
		Detail->EditCategory(CategoryName, FText::GetEmpty(), ECategoryPriority::TypeSpecific).AddProperty(PropertyHandle);
		AlterMeshSection->AddCategory(CategoryName);
	}
}

void FAlterMeshActorDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	Detail = &DetailBuilder;

	bool bAnyActorLocked = false;
	
	for (TWeakObjectPtr<UObject> Object : Objects)
	{
		if (Object.IsValid())
		{
			if (AAlterMeshActor* AlterMeshActor = Cast<AAlterMeshActor>(Object))
			{
				bAnyActorLocked = bAnyActorLocked || AlterMeshActor->bLocked;
			}
		}
	}
	
	if (!AlterMeshSection.IsValid())
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>( "PropertyEditor" );
		AlterMeshSection = PropertyEditorModule.FindOrCreateSection(Objects[0]->GetClass()->GetFName(), FName("AlterMesh"), FText::FromString("AlterMesh"));
	}
	
	AlterMeshSection->AddCategory("Parameters");
	
	IDetailCategoryBuilder& AssetCategory = DetailBuilder.EditCategory("Asset", FText::GetEmpty(), ECategoryPriority::Important);
	IDetailCategoryBuilder& ParamsCategory = DetailBuilder.EditCategory("Parameters", FText::GetEmpty(), ECategoryPriority::Important);
	
	TSharedPtr<IPropertyHandle> Property = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(AAlterMeshActor, InputParams))->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAlterMeshInputParams, Params));
	TSharedRef<FDetailArrayBuilder> PropertyBuilder = MakeShareable(new FDetailArrayBuilder(Property.ToSharedRef(), false, false, false));
	PropertyBuilder->OnGenerateArrayElementWidget(FOnGenerateArrayElementWidget::CreateSP(this, &FAlterMeshActorDetailCustomization::GenerateParamWidget, !bAnyActorLocked));
	ParamsCategory.AddCustomBuilder(PropertyBuilder);

	// Refresh actor when a property changes
	FSimpleDelegate OnChange;
	OnChange.BindLambda([Objects]()
	{
		for (TWeakObjectPtr<UObject> Object : Objects)
		{
			if (Object.IsValid())
			{
				AAlterMeshActor* AlterMeshActor = Cast<AAlterMeshActor>(Object);
				if (AlterMeshActor)
				{
					AlterMeshActor->RefreshAsync();
				}
			}
		}
	});

	Property->SetOnChildPropertyValueChanged(OnChange);

	// Calls convert function, if class was not selected, show a picker
	for (TWeakObjectPtr<UObject> Object : Objects)
	{
		if (AAlterMeshActor* AlterMeshActor = Cast<AAlterMeshActor>(Object))
		{
			AlterMeshActor->OnConvertClicked.BindWeakLambda(AlterMeshActor, [AlterMeshActor]()
			{
				if (AlterMeshActor->AlterMeshComponents.Num())
				{
					// No class selected, show picker
					if (!AlterMeshActor->ConverterInstance)
					{
						if (AlterMeshActor->Asset->Get()->ConverterClass.LoadSynchronous())
						{
							AlterMeshActor->ConverterInstance = NewObject<UAlterMeshConverterBase>(AlterMeshActor, AlterMeshActor->Asset->Get()->ConverterClass.LoadSynchronous());
							AlterMeshActor->OnBecomePreview.AddDynamic(Cast<UAlterMeshConverterBase>(AlterMeshActor->ConverterInstance), &UAlterMeshConverterBase::CleanUp);
						}
						else
						{
							FClassViewerInitializationOptions Options;
							Options.DisplayMode = EClassViewerDisplayMode::ListView;
							Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;

							const TSharedRef<FAlterMeshConverterClassFilter> ClassFilter = MakeShareable(new FAlterMeshConverterClassFilter);
							Options.ClassFilters.Add(ClassFilter);

							UClass* ChosenClass;
							SClassPickerDialog::PickClass(FText::FromString("Converter class"), Options, ChosenClass, UAlterMeshConverterBase::StaticClass());

							if (ChosenClass)
							{
								AlterMeshActor->ConverterInstance = NewObject<UAlterMeshConverterBase>(AlterMeshActor, ChosenClass);
								AlterMeshActor->OnBecomePreview.AddDynamic(Cast<UAlterMeshConverterBase>(AlterMeshActor->ConverterInstance), &UAlterMeshConverterBase::CleanUp);
							}
						}
					}

					// Failed to pick class
					if (!AlterMeshActor->ConverterInstance)
					{
						return;
					}

					if (UAlterMeshConverterBase* Converter = Cast<UAlterMeshConverterBase>(AlterMeshActor->ConverterInstance))
					{
						Converter->Convert(AlterMeshActor);
						Converter->K2_Convert(AlterMeshActor);
					}

					AlterMeshActor->SetPreview(false);
		
					GEditor->SelectNone( false, true );
					GEditor->SelectActor( AlterMeshActor, true, true );	
				}
			});
		}
	}

	DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(AAlterMeshActor, bLocked))->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda(
		[this]()
		{
			if (Detail)
			{
				Detail->ForceRefreshDetails();
			}
		}));
	
	// Converter object properties custom row
	{
		TArray<UObject*> Converters;
		IDetailCategoryBuilder& ConverterCategory = DetailBuilder.EditCategory("Converter Settings", FText::GetEmpty(), ECategoryPriority::TypeSpecific);		
		AlterMeshSection->AddCategory("Converter Settings");
	
		for (TWeakObjectPtr<UObject> Object : Objects)
		{
			if (AAlterMeshActor* AlterMeshActor = Cast<AAlterMeshActor>(Object))
			{			
				if (AlterMeshActor->ConverterInstance)
				{
					Converters.Add(AlterMeshActor->ConverterInstance);			
				}
			}
		}

		if (Converters.Num())
		{
			FAddPropertyParams Params;
			Params.CreateCategoryNodes(false);
			auto PropertyRow = ConverterCategory.AddExternalObjects(Converters, EPropertyLocation::Default, Params);
			PropertyRow->CustomWidget(true)
			.NameWidget
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				[
					SNew(STextBlock)
					.Text(Converters.Num() > 1 ?
						FText::Format(FText::FromString(TEXT("{0} Converters")), {Converters.Num()})
						: FText::Format(FText::FromString("Type: {0}"), {Converters[0]->GetClass()->GetDisplayNameText()}))
				]
			];
		}
		else
		{
			ConverterCategory.AddCustomRow(LOCTEXT("AlterMeshConverterWarning", "Missing Converter Class"), false)
			.WholeRowWidget
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign( VAlign_Center )
				[
					SNew(STextBlock)
					.Text(FText::FromString("Asset is missing default converter class"))
				]
			];
		}
	}
}

#undef LOCTEXT_NAMESPACE
