// Copyright 2023 Aechmea

#include "AlterMeshAsset.h"
#include "AlterMeshActor.h"
#include "AlterMeshImport.h"
#include "CommonParams/AlterMeshIntParam.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/UObjectIterator.h"

#if WITH_EDITOR
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Factories/MaterialImportHelpers.h"
#endif

UAlterMeshAsset::UAlterMeshAsset()
{
	FAlterMeshAttributeMapping UVs;
	UVs.From = FName("UVMap");
	UVs.To = EAlterMeshAttribute::UV0;
	UVs.FromChannel = (uint8)EAlterMeshAttributeMask::R | (uint8)EAlterMeshAttributeMask::G; 
	UVs.ToChannel = (uint8)EAlterMeshAttributeMask::R | (uint8)EAlterMeshAttributeMask::G; 
	
	AttributeMapping.Add(UVs);
	
	FAlterMeshAttributeMapping Col;
	Col.From = FName("Color");
	Col.To = EAlterMeshAttribute::Color;
	Col.FromChannel = (uint8)EAlterMeshAttributeMask::All; 
	Col.ToChannel = (uint8)EAlterMeshAttributeMask::All;
	
	AttributeMapping.Add(Col);
}

void UAlterMeshAsset::CreateParamsFromJson(FString Json)
{
	// Convert to json object
	TSharedPtr<FJsonObject> JsonParams = MakeShareable(new FJsonObject);
	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Json);
	if (FJsonSerializer::Deserialize(JsonReader, JsonParams))
	{
		for (const TSharedPtr<FJsonValue> Mat : JsonParams->GetArrayField(TEXT("Materials")))
		{
			UMaterialInterface* Material = nullptr;
#if WITH_EDITOR
			FText MaterialError;
			Material = UMaterialImportHelpers::FindExistingMaterial(TEXT("/Game"), Mat->AsString(), true, MaterialError);
#endif
			Materials.Add( FName(*Mat->AsString()), Material);
		}
		
		
		for (const TSharedPtr<FJsonValue> Param : JsonParams->GetArrayField(TEXT("Params")))
		{
			FName NodeGroup = FName(Param->AsObject()->GetStringField(TEXT("NodeGroup")));
			FName Id = FName(Param->AsObject()->GetStringField(TEXT("Id")));
			FString Name = Param->AsObject()->GetStringField(TEXT("Name"));
			FString Type = Param->AsObject()->GetStringField(TEXT("Type"));
			FString Panel = Param->AsObject()->GetStringField(TEXT("Panel"));
			FString Tooltip = Param->AsObject()->GetStringField(TEXT("Tooltip"));

			for (TObjectIterator<UScriptStruct> StructIterator; StructIterator; ++StructIterator)
			{
				if (StructIterator->IsChildOf(FAlterMeshParamBase::StaticStruct()))
				{
					if(FInstancedStruct(*StructIterator).Get<FAlterMeshParamBase>().UsedForType(Type))
					{						
						FInstancedStruct NewParam;
						NewParam.InitializeAs(FInstancedStruct(*StructIterator).GetScriptStruct());
						NewParam.GetMutable<FAlterMeshParamBase>().BaseData.Id.NodeGroup = NodeGroup;
						NewParam.GetMutable<FAlterMeshParamBase>().BaseData.Id.Input = Id;
						NewParam.GetMutable<FAlterMeshParamBase>().BaseData.Name = Name;
						NewParam.GetMutable<FAlterMeshParamBase>().BaseData.Panel = Panel;
						NewParam.GetMutable<FAlterMeshParamBase>().BaseData.Tooltip = Tooltip;
						NewParam.GetMutable<FAlterMeshParamBase>().DeserializeJson(Param);
						InputParams.Add(NewParam);
					}					
				}
			}
		}
	}
	else
		
	{
		UE_LOG(LogAlterMeshImport, Warning, TEXT("Could not parse json: %s"), *Json)
	}
}

UAlterMeshAssetInstance* UAlterMeshAssetInstance::CreateInstance(UAlterMeshAsset* Parent, const FAlterMeshInputParams& Params)
{
#if WITH_EDITOR
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	
	FString PackageName;
	FString ObjectName;
	AssetToolsModule.Get().CreateUniqueAssetName(Parent->GetOutermost()->GetName(), TEXT("_Inst"), PackageName, ObjectName);		

	UPackage* Package = CreatePackage(*PackageName);
	UAlterMeshAssetInstance* AlterMeshAssetInstance = NewObject<UAlterMeshAssetInstance>(Package, UAlterMeshAssetInstance::StaticClass(), FName(ObjectName), RF_Public | RF_Standalone | RF_Transactional);

	AlterMeshAssetInstance->Parent = Cast<UAlterMeshAsset>(Parent);
	AlterMeshAssetInstance->GetParams().Copy(Params, AlterMeshAssetInstance);
	
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<UObject*> Objects = {AlterMeshAssetInstance};
	ContentBrowserModule.Get().SyncBrowserToAssets(Objects);

	return AlterMeshAssetInstance;
#endif
	return nullptr;
}
