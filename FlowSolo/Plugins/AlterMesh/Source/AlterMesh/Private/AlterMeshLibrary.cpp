// Copyright 2023 Aechmea

#include "AlterMeshLibrary.h"
#include "AlterMeshComponent.h"
#include "CommonParams/AlterMeshBoolParam.h"
#include "CommonParams/AlterMeshCollectionParam.h"
#include "CommonParams/AlterMeshFloatParam.h"
#include "CommonParams/AlterMeshIntParam.h"
#include "CommonParams/AlterMeshVectorParam.h"
#include "GeometryParams/AlterMeshGeometryAsset.h"
#include "Engine/StaticMesh.h"
#include "Engine/Blueprint.h"

FString UAlterMeshLibrary::ConvertFilenameToFull(const FString& InPath)
{
	if (FPaths::IsRelative(InPath))
	{
		// Path is relative to ProjectDir
		FString FullPath = FPaths::ProjectDir() + InPath;
		FullPath = FPaths::ConvertRelativePathToFull(FullPath);
		return FullPath;
	}

	return InPath;
}

FString UAlterMeshLibrary::ConvertFilenameToRelative(const FString& InPath)
{
	// if Filename is relative to engine dir, we want from project dir for source control purposes
	FString RelativePath = InPath;
	FPaths::MakePathRelativeTo(RelativePath, *FPaths::ProjectDir());
	return RelativePath;
}

void UAlterMeshLibrary::SetFrame(AAlterMeshActor* InActor, int32 InFrame, bool bRefresh)
{
	if (InActor)
	{
		InActor->InputParams.Frame = InFrame;

		if (bRefresh)
		{
			InActor->RefreshSync();
		}
	}
}

void UAlterMeshLibrary::SetTime(AAlterMeshActor* InActor, float InTime, bool bRefresh)
{
	if (InActor)
	{
		InActor->InputParams.Frame = InTime;

		if (bRefresh)
		{
			InActor->RefreshSync();
		}
	}
}

void UAlterMeshLibrary::SetVectorParam(AAlterMeshActor* InActor, FName ParamName, FVector Value, bool bConvertCoordinateSpace, bool bRefresh)
{
	if (InActor && ParamName != NAME_None)
	{
		for (FAlterMeshVectorParam* VectorParam : InActor->InputParams.GetTyped<FAlterMeshVectorParam>())
		{
			
			if (VectorParam->BaseData.Name == ParamName.ToString())
			{
				VectorParam->Data.Value = Value;
				VectorParam->Data.bConvertCoordinateSpace = bConvertCoordinateSpace;

				if (bRefresh)
				{
					InActor->RefreshSync();
				}
			}
		}
	}
}

void UAlterMeshLibrary::SetFloatParam(AAlterMeshActor* InActor, FName ParamName, float Value, bool bRefresh)
{
	if (InActor && ParamName != NAME_None)
	{
		for (FAlterMeshFloatParam* FloatParam : InActor->InputParams.GetTyped<FAlterMeshFloatParam>())
		{
			if (FloatParam->BaseData.Name == ParamName.ToString())
			{
				FloatParam->Data.Value = Value;

				if (bRefresh)
				{
					InActor->RefreshSync();
				}
			}
		}
	}
}

void UAlterMeshLibrary::SetIntParam(AAlterMeshActor* InActor, FName ParamName, int32 Value, bool bRefresh)
{
	if (InActor && ParamName != NAME_None)
	{
		for (FAlterMeshIntParam* IntParam : InActor->InputParams.GetTyped<FAlterMeshIntParam>())
		{
			if (IntParam->BaseData.Name == ParamName.ToString())
			{
				IntParam->Data.Value = Value;

				if (bRefresh)
				{
					InActor->RefreshSync();
				}
			}
		}
	}
}

void UAlterMeshLibrary::SetBoolParam(AAlterMeshActor* InActor, FName ParamName, bool Value, bool bRefresh)
{
	if (InActor && ParamName != NAME_None)
	{
		for (FAlterMeshBoolParam* BoolParam : InActor->InputParams.GetTyped<FAlterMeshBoolParam>())
		{
			if (BoolParam->BaseData.Name == ParamName.ToString())
			{
				BoolParam->Data.Value = Value;

				if (bRefresh)
				{
					InActor->RefreshSync();
				}
			}
		}
	}
}

void UAlterMeshLibrary::SetAssetParam(AAlterMeshActor* InActor, FName ParamName, UObject* Value, bool bRefresh)
{
	if (InActor && ParamName != NAME_None && Value)
	{
		if (Cast<UStaticMesh>(Value) || Cast<UBlueprint>(Value))
		{
			for (FAlterMeshGeometryParam* GeometryParam : InActor->InputParams.GetTyped<FAlterMeshGeometryParam>())
			{
				if (GeometryParam->BaseData.Name == ParamName.ToString())
				{
					if (UAlterMeshGeometryAsset* GeometryParamAsset = Cast<UAlterMeshGeometryAsset>(GeometryParam->Data.InputType))
					{
						GeometryParamAsset->Object = Value;

						if (bRefresh)
						{
							InActor->RefreshSync();
						}
					}
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("SetObjectParam only works with static meshes or blueprints for now"));
		}
	}
}

UAlterMeshGeometryBase* UAlterMeshLibrary::GetGeometryParam(AAlterMeshActor* InActor, FString ParamName)
{
	if (InActor && !ParamName.IsEmpty())
	{
		for (FAlterMeshGeometryParam* GeometryParam : InActor->InputParams.GetTyped<FAlterMeshGeometryParam>())
		{
			if (GeometryParam->BaseData.Name == ParamName)
			{
				return GeometryParam->Data.InputType;
			}
		}

		for (FAlterMeshCollectionParam* CollectionParam : InActor->InputParams.GetTyped<FAlterMeshCollectionParam>())
		{
			for (FAlterMeshGeometryParam& GeometryParam : CollectionParam->Data.Geometries)
			{
				if (GeometryParam.BaseData.Name == ParamName)
				{
					return GeometryParam.Data.InputType;
				}
			}
		}
	}

	return nullptr;
}

UAlterMeshGeometryBase* UAlterMeshLibrary::GetGeometryParamById(AAlterMeshActor* InActor, FAlterMeshParamId ParamId)
{
	if (InActor && ParamId.Input != NAME_None)
	{
		for (FAlterMeshGeometryParam* GeometryParam : InActor->InputParams.GetTyped<FAlterMeshGeometryParam>())
		{
			if (GeometryParam->BaseData.Id == ParamId)
			{
				return GeometryParam->Data.InputType;
			}
		}

		for (FAlterMeshCollectionParam* CollectionParam : InActor->InputParams.GetTyped<FAlterMeshCollectionParam>())
		{
			for (FAlterMeshGeometryParam& GeometryParam : CollectionParam->Data.Geometries)
			{
				if (GeometryParam.BaseData.Id == ParamId)
				{
					return GeometryParam.Data.InputType;
				}
			}
		}
	}

	return nullptr;
}

void UAlterMeshLibrary::SetMaterial(AAlterMeshActor* InActor, FName InSlotName, UMaterialInterface* InMaterial)
{
	if (InActor)
	{
		FAlterMeshMaterial* Override = InActor->OverrideMaterials.FindByPredicate([InSlotName](const FAlterMeshMaterial& Material)
		{
			 return Material.SlotName == InSlotName;
		});

		if (Override)
		{
			Override->Material = InMaterial;
		}
		InActor->RefreshMaterials();
	}
}

void UAlterMeshLibrary::GetMaterials(AAlterMeshActor* InActor, FName InSlotName, TArray<UMaterialInterface*>& OutMaterials)
{
	if (InActor)
	{
		for (UAlterMeshComponent* Component : InActor->AlterMeshComponents)
		{
			if (Component)
			{
				for (int32 i = 0; i < Component->GetNumMaterials(); i++)
				{
					// Materials from all components that match InSlotName
					if (Component->GetMaterialSlotNames()[i] == InSlotName)
					{
						OutMaterials.Add(Component->GetMaterial(i));
					}
				}
			}
		}
	}
}

void UAlterMeshLibrary::GetAllMaterials(AAlterMeshActor* InActor, TArray<FName>& OutMaterialNames, TArray<UMaterialInterface*>& OutMaterials)
{
	if (InActor)
	{
		for (UAlterMeshComponent* Component : InActor->AlterMeshComponents)
		{
			if (Component)
			{
				// All materials from all components
				for (int32 i = 0; i < Component->GetNumMaterials(); i++)
				{
					OutMaterialNames.Add(Component->GetMaterialSlotNames()[i]);
					OutMaterials.Add(Component->GetMaterial(i));
				}
			}
		}
	}
}

void UAlterMeshLibrary::CreateDynamicMaterialInstance(AAlterMeshActor* InActor, FName InSlotName, UMaterialInterface* InMaterial)
{
	if (InActor)
	{
		for (UAlterMeshComponent* Component : InActor->AlterMeshComponents)
		{
			if (Component)
			{
				// Create DMI for All components matching slot name
				for (int32 i = 0; i < Component->GetNumMaterials(); i++)
				{
					if (Component->GetMaterialSlotNames()[i] == InSlotName)
					{
						Component->CreateDynamicMaterialInstance(i, InMaterial);
					}
				}
			}
		}
	}
}
