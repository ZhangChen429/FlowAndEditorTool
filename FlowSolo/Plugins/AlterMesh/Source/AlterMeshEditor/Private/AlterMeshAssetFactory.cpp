// Copyright 2023 Aechmea

#include "AlterMeshAssetFactory.h"

#include <Extern/AlterMesh.h>
#pragma comment(lib, "AlterMesh.lib")

#include "AlterMeshActor.h"
#include "AlterMeshAsset.h"
#include "AlterMeshAssetReimportFactory.h"
#include "AlterMeshConverterStaticMesh.h"
#include "AlterMeshImport.h"
#include "AlterMeshLibrary.h"
#include "AlterMeshSettings.h"
#include "ObjectTools.h"
#include "StructUtils/StructView.h"
#include "StructView.h"
#include "GeometryParams/AlterMeshGeometryBase.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/FeedbackContext.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/STileView.h"

DEFINE_LOG_CATEGORY(LogAlterMeshAssetFactory)

UAlterMeshAssetFactory::UAlterMeshAssetFactory()
{
	bEditorImport = true;
	Formats.Add(FString("blend;Blend File"));
	SupportedClass = UAlterMeshAsset::StaticClass();
	bEditAfterNew = false;
	ImportPriority = DefaultImportPriority + 1;
}

void UAlterMeshAssetFactory::ShowImportDialog()
{
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(FText::FromString("Importing"))
		.ClientSize(FVector2D(400.f, 400.f))
		.SupportsMinimize(false)
		.SupportsMaximize(false);


	TSharedRef<SBorder> Border = SNew(SBorder)
	.BorderImage(FAppStyle::GetBrush("Menu.Background"))
	.Padding(5.f)
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		.FillHeight(1.0f)
		[
			SAssignNew(ListView, SListView<TSharedPtr<FString> >)
				.OnGenerateRow_UObject(this, &UAlterMeshAssetFactory::GenerateRow)
				.ListItemsSource(&ListViewObjects)
				.SelectionMode(ESelectionMode::Multi)
				.OnSelectionChanged_Lambda([&] (TSharedPtr<FString> NewItem, ESelectInfo::Type SelectType)
				{
					SelectedObjects.Empty();
					for (auto Item : ListView->GetSelectedItems())
					{
						SelectedObjects.Add(*Item.Get());
					}
				})
		]
		+SVerticalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.FillHeight(0.1f)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.f)
			[
				SNew(SButton)
				.Text(FText::FromString("Import All"))
				.OnClicked_Lambda([&]()
				{
					SelectedObjects.Empty();
					for (const TSharedPtr<FString>& ObjectName : ListViewObjects)
					{
						SelectedObjects.Add(*ObjectName.Get());
					}

					bImport = true;
					Window->RequestDestroyWindow();
					return FReply::Handled();
				})
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.f)
			[
				SNew(SButton)
				.Text(FText::FromString("Import"))
				.OnClicked_Lambda([&]()
				{
					bImport = true;
					Window->RequestDestroyWindow();
					return FReply::Handled();
				})
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.f)
			[
				SNew(SButton)
				.Text(FText::FromString("Cancel"))
				.OnClicked_Lambda([&]()
				{
					SelectedObjects.Empty();
					Window->RequestDestroyWindow();
					return FReply::Handled();
				})
			]
		]
	];

	bImport = false;
	SelectedObjects.Empty();
	Window->SetContent(Border);
	GEditor->EditorAddModalWindow(Window);
}

TSharedRef<ITableRow> UAlterMeshAssetFactory::GenerateRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
	SNew(STableRow< TSharedRef<FText> >, OwnerTable)
	[
		SNew(STextBlock).Text(FText::FromString(*Item.Get()))
	];
}

UObject* UAlterMeshAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms,
													FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, InClass, InParent, InName, Parms);
	AdditionalImportedObjects.Empty();

	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("AlterMesh"))->GetBaseDir();
	const FString ScriptPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(PluginDir, FString("\\Source\\ThirdParty\\GetParams.py")));
	const FString FilePath = FPaths::ConvertRelativePathToFull(Filename);

	UAlterMeshInstance* Instance = NewObject<UAlterMeshInstance>(this);
	if (!Instance->Initialize(FilePath, ScriptPath))
	{
		return nullptr;
	}

	if (ReadLock(Instance->AlterMeshHandle->Get()))
	{
		ListViewObjects.Empty();
		SelectedObjects.Empty();

		FAlterMeshImport Importer(Instance->AlterMeshHandle, nullptr);

		TArray<FString> ObjectNames;
		TArray<FString> ObjectParams;
		Importer.ImportParams(ObjectNames, ObjectParams);

		for (const FString& ObjectName : ObjectNames)
		{
			ListViewObjects.Add(MakeShared<FString>(ObjectName));
		}

		bImport = false;
				
		if (!bReimporting)
		{
			if (ListViewObjects.Num() > 1)
			{
				ShowImportDialog();
			}
			else
			{
				bImport = true;

				for (const TSharedPtr<FString>& ObjectName : ListViewObjects)
				{
					SelectedObjects.Add(*ObjectName.Get());
				}
			}
		}

		if ( bImport || bReimporting )
		{
			for (int32 i = 0; i < ObjectNames.Num(); i++)
			{
				if (SelectedObjects.Contains(ObjectNames[i])
					|| (bReimporting && ObjectNames[i] == ReimportedObject->ObjectName)
					|| (bReimporting && ReimportedObject->ObjectName.IsEmpty()))
				{
					Warn->Log(TEXT("Importing:") + FilePath);

					FString ObjectName;
					FString PackageName;
					
					if (bReimporting)
					{
						ObjectName = ReimportedObject->GetName();
						PackageName = InParent->GetPathName();
					}
					else
					{
						FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
						AssetToolsModule.Get().CreateUniqueAssetName(InParent->GetPathName(), TEXT("_")+ObjectNames[i], PackageName, ObjectName);		
					}
					
					UPackage* Package = CreatePackage(*PackageName);
					UAlterMeshAsset* AlterMeshAsset = NewObject<UAlterMeshAsset>(Package, InClass, FName(ObjectName), Flags);
					AlterMeshAsset->CreateParamsFromJson(ObjectParams[i]);
					AlterMeshAsset->ObjectName = ObjectNames[i];

					for (FAlterMeshGeometryParam* GeometryParam : AlterMeshAsset->GetParams().GetTyped<FAlterMeshGeometryParam>())
					{
						TArrayView<TCHAR> DefaultGeometryTypeNameArray = Importer.ReadArray<TCHAR>();
						FName DefaultGeometryTypeName = NAME_None;
						if (DefaultGeometryTypeNameArray.Num() > 0)
						{
							DefaultGeometryTypeName = FName(FString(DefaultGeometryTypeNameArray).Replace(TEXT("GeometryTypes."), TEXT("Default__")));

							for (UAlterMeshGeometryBase* GeometryType : TObjectRange<UAlterMeshGeometryBase>(RF_NoFlags))
							{
								if (GeometryType->HasAnyFlags(RF_ClassDefaultObject))
								{
									if (GeometryType->GetFName() == DefaultGeometryTypeName)
									{
										GeometryParam->Data.InputType = NewObject<UAlterMeshGeometryBase>(AlterMeshAsset, GeometryType->GetClass());
										GeometryParam->Data.InputType->ImportDefaults(Importer);
									}
								}
							}
						}
					}

					if (!AlterMeshAsset->ConverterClass.LoadSynchronous() && !bReimporting)
					{
						AlterMeshAsset->ConverterClass = UAlterMeshConverterStaticMesh::StaticClass();	
					}
					
					AlterMeshAsset->Filename.FilePath = UAlterMeshLibrary::ConvertFilenameToRelative(FilePath);
					GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, AlterMeshAsset);
					AdditionalImportedObjects.Add(AlterMeshAsset);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("No files imported."));
		}

		ReadUnlock(Instance->AlterMeshHandle->Get());
		Instance->CleanupProcess();

		return AdditionalImportedObjects.Num() ? AdditionalImportedObjects[0] : nullptr;
	}

	/*
	if (FMessageDialog::Open(EAppMsgType::OkCancel, EAppReturnType::Ok, NSLOCTEXT("AlterMeshImport", "AlterMeshImport_Unresponsive", "Blender is taking longer than expected to open. You can cancel or continue importing."))
		== EAppReturnType::Cancel)
	{
		// Failed to import file			
		return nullptr;
	}*/

	return nullptr;
}
