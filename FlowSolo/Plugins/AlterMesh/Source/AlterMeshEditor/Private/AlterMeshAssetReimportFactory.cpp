// Copyright 2023 Aechmea

#include "AlterMeshAssetReimportFactory.h"
#include <Extern/AlterMesh.h>
#include "AlterMeshAsset.h"
#include "AlterMeshLibrary.h"
#include "AlterMeshImport.h"
#include "Utils.h"
#include "CommonParams/AlterMeshMenuParam.h"
#include "HAL/FileManager.h"

#include "Interfaces/IPluginManager.h"

UAlterMeshAssetReimportFactory::UAlterMeshAssetReimportFactory()
{
	SupportedClass = UAlterMeshAsset::StaticClass();
	Formats.Add(FString("blend;Blend File"));

	bCreateNew = false;
	ImportPriority = DefaultImportPriority - 1;
}

bool UAlterMeshAssetReimportFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	if (UAlterMeshAsset* AlterMeshAsset = Cast<UAlterMeshAsset>(Obj))
	{
		OutFilenames.Add(UAlterMeshLibrary::ConvertFilenameToFull(AlterMeshAsset->Filename.FilePath));
		return true;
	}

	return false;
}

void UAlterMeshAssetReimportFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UAlterMeshAsset* AlterMeshAsset = Cast<UAlterMeshAsset>(Obj);
	if (AlterMeshAsset && ensure(NewReimportPaths.Num() == 1))
	{
		AlterMeshAsset->Filename.FilePath =NewReimportPaths[0];
	}
}

EReimportResult::Type UAlterMeshAssetReimportFactory::Reimport(UObject* Obj)
{
	UAlterMeshAsset* AlterMeshAsset = Cast<UAlterMeshAsset>(Obj);
	if (!AlterMeshAsset)
	{
		UE_LOG(LogAlterMeshAssetFactory, Warning, TEXT("Reimport failed"));
		return EReimportResult::Failed;
	}

	const FString Filename = UAlterMeshLibrary::ConvertFilenameToFull(AlterMeshAsset->Filename.FilePath);
	const FString FileExtension = FPaths::GetExtension(Filename);

	const bool bIsSupportedExtension = FileExtension == FString("Blend");

	if (!bIsSupportedExtension)
	{
		UE_LOG(LogAlterMeshAssetFactory, Warning, TEXT("Reimport failed - Wrong file extension."));
		return EReimportResult::Failed;
	}

	UE_LOG(LogAlterMeshAssetFactory, Log, TEXT("Reimporting [%s]"), *Filename);

	// Ensure that the file provided by the path exists
	if (IFileManager::Get().FileSize(*Filename) == INDEX_NONE)
	{
		UE_LOG(LogAlterMeshAssetFactory, Warning, TEXT("Reimport failed - file can't be found"));
		return EReimportResult::Failed;
	}

	bReimporting = true;
	ReimportedObject = AlterMeshAsset;
	
	if (AlterMeshAsset->bReimportOnlyParams)
	{
		const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("AlterMesh"))->GetBaseDir();
		const FString ScriptPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(PluginDir, FString("\\Source\\ThirdParty\\GetParams.py")));
		const FString FilePath = FPaths::ConvertRelativePathToFull(Filename);

		UAlterMeshInstance* Instance = NewObject<UAlterMeshInstance>(this);
		if (!Instance->Initialize(FilePath, ScriptPath))
		{
			UE_LOG(LogAlterMeshAssetFactory, Warning, TEXT("Reimport failed - Couldn't open file"));
			return EReimportResult::Failed;
		}

		if (ReadLock(Instance->AlterMeshHandle->Get()))
		{
			ListViewObjects.Empty();
			SelectedObjects.Empty();

			FAlterMeshImport Importer(Instance->AlterMeshHandle, nullptr);

			TArray<FString> ObjectNames;
			TArray<FString> ObjectParams;
			Importer.ImportParams(ObjectNames, ObjectParams);
			
			for (FString& ObjectName : ObjectNames)
			{
				if(AlterMeshAsset->ObjectName == ObjectName)
				{
					UAlterMeshAsset* NewAsset = NewObject<UAlterMeshAsset>(GetTransientPackage(), AlterMeshAsset->GetClass());
					NewAsset->CreateParamsFromJson(ObjectParams[0]);

					// Added Params
					{
						FAlterMeshStructArrayView View = FAlterMeshStructArrayView(NewAsset->GetParams().Params);
						for (int32 i = 0; i < View.Num(); i++)
						{
							FAlterMeshParamId NewParamId = View[i].Get<FInstancedStruct>().Get<FAlterMeshParamBase>().BaseData.Id;
							const UScriptStruct* NewParamStruct = View[i].Get<FInstancedStruct>().GetScriptStruct();

							// Check if old asset does not contain this param
							auto bIsNewParam = !AlterMeshAsset->GetParams().Params.ContainsByPredicate([NewParamId, NewParamStruct](const FInstancedStruct& Item)
							{							
								return Item.Get<FAlterMeshParamBase>().BaseData.Id == NewParamId
								&& Item.GetScriptStruct() == NewParamStruct;
							});
						
							// if it doesn't, add
							if (bIsNewParam)
							{
								AlterMeshAsset->GetParams().Params.Insert(View[i].Get<FInstancedStruct>(), i);
							}
						}
					}
					
					// Removed Params
					{
						FAlterMeshStructArrayView View = FAlterMeshStructArrayView(AlterMeshAsset->GetParams().Params);
						for (int32 i = View.Num() -1; i >= 0; i--)
						{
							FAlterMeshParamId ParamId = View[i].Get<FInstancedStruct>().Get<FAlterMeshParamBase>().BaseData.Id;
							const UScriptStruct* ParamStruct = View[i].Get<FInstancedStruct>().GetScriptStruct();

							// Check if new asset doesnt contain this param
							auto bIsRemovedParam = !NewAsset->GetParams().Params.ContainsByPredicate([ParamId, ParamStruct](const FInstancedStruct& Item)
							{							
								return Item.Get<FAlterMeshParamBase>().BaseData.Id == ParamId
								&& Item.GetScriptStruct() == ParamStruct;
							});

							// if it doesn't, remove
							if (bIsRemovedParam)
							{
								AlterMeshAsset->GetParams().Params.RemoveAt(i);
							}
						}
					}

					// Update Panels and display name
					{						
						FAlterMeshStructArrayView NewParamsView = FAlterMeshStructArrayView(NewAsset->GetParams().Params);
						FAlterMeshStructArrayView OldParamsView = FAlterMeshStructArrayView(AlterMeshAsset->GetParams().Params);
						for (int32 i = OldParamsView.Num() -1; i >= 0; i--)
						{
							if (OldParamsView[i].Get<FInstancedStruct>().GetMutable<FAlterMeshParamBase>().BaseData.Id ==
								NewParamsView[i].Get<FInstancedStruct>().GetMutable<FAlterMeshParamBase>().BaseData.Id)
							{
								OldParamsView[i].Get<FInstancedStruct>().GetMutable<FAlterMeshParamBase>().BaseData = NewParamsView[i].Get<FInstancedStruct>().GetMutable<FAlterMeshParamBase>().BaseData;
							}
						}
					}
					
					// Update menu switch params
					{						
						FAlterMeshStructArrayView NewParamsView = FAlterMeshStructArrayView(NewAsset->GetParams().Params);
						FAlterMeshStructArrayView OldParamsView = FAlterMeshStructArrayView(AlterMeshAsset->GetParams().Params);
						for (int32 i = OldParamsView.Num() -1; i >= 0; i--)
						{
							if (OldParamsView[i].Get<FInstancedStruct>().GetMutable<FAlterMeshParamBase>().BaseData.Id ==
								NewParamsView[i].Get<FInstancedStruct>().GetMutable<FAlterMeshParamBase>().BaseData.Id
								&& NewParamsView[i].Get<FInstancedStruct>().GetPtr<FAlterMeshMenuParam>())
								
							{
								OldParamsView[i].Get<FInstancedStruct>().GetMutable<FAlterMeshMenuParam>().Data.Entries = NewParamsView[i].Get<FInstancedStruct>().GetMutable<FAlterMeshMenuParam>().Data.Entries;
								OldParamsView[i].Get<FInstancedStruct>().GetMutable<FAlterMeshMenuParam>().Data.Values = NewParamsView[i].Get<FInstancedStruct>().GetMutable<FAlterMeshMenuParam>().Data.Values;
							}
						}
					}
				}
			}
			
			UE_LOG(LogAlterMeshAssetFactory, Log, TEXT("Params Reimported."));
			ReadUnlock(Instance->AlterMeshHandle->Get());
			Instance->CleanupProcess();
		}		
	}
	else
	{
		bool OutCanceled = false;
		UObject* NewAsset = ImportObject(AlterMeshAsset->GetClass(), AlterMeshAsset->GetOuter(), *AlterMeshAsset->GetName(), RF_Public | RF_Standalone, Filename, nullptr, OutCanceled);
		if (!NewAsset)
		{
			if (OutCanceled)
			{
				UE_LOG(LogAlterMeshAssetFactory, Warning, TEXT("Reimport failed - canceled"));
				return EReimportResult::Cancelled;
			}

			UE_LOG(LogAlterMeshAssetFactory, Warning, TEXT("Reimport failed"));
			return EReimportResult::Failed;
		}
		
		UE_LOG(LogAlterMeshAssetFactory, Log, TEXT("file reimported."));
	}


	AlterMeshAsset->MarkPackageDirty();
	
	return EReimportResult::Succeeded;
}

int32 UAlterMeshAssetReimportFactory::GetPriority() const
{
	return ImportPriority;
}

void UAlterMeshAssetReimportFactory::CleanUp()
{

}
