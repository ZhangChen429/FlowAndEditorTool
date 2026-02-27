// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshStructArrayView.h"
#include "InstancedStruct.h"
#include "StructUtils/StructView.h"
#include "CommonParams/AlterMeshParamBase.h"
#include "AlterMeshParam.generated.h"

/*
 * Blender param types definitions
 *
 * Customization:
 * AlterMeshParamCustomization.h
 * AlterMeshActorDetailCustomization.h
 * AlterMeshAssetDetailCustomization.h
 *
 * Add functionality on:
 * AlterMeshExport.h
 * AlterMeshImport.h
 *
 * Geometry / Object type parameters can be customized.
 * See UAlterMeshGeometryParamBase to extend geometry parameters
*/

// Available input types
USTRUCT(BlueprintType)
struct ALTERMESH_API FAlterMeshInputParams
{
	GENERATED_BODY()
public:

	template<typename T>
	T* FindParamById(FAlterMeshParamId Id)
	{
		for (FAlterMeshStructView Param : FAlterMeshStructArrayView(Params))
		{
			if (Param.GetScriptStruct()
				&& Param.GetPtr<FInstancedStruct>()->GetMutablePtr<T>()
				&& Param.Get<FInstancedStruct>().GetMutable<T>().BaseData.Id == Id)
			{
				return &Param.Get<FInstancedStruct>().GetMutable<T>();
			}
		}
		
		return nullptr;
	}

	template<typename T>
	TArray<T*> GetTyped()
	{
		TArray<T*> OutArray;
		for (FAlterMeshStructView Param : FAlterMeshStructArrayView(Params))
		{
			if (Param.Get<FInstancedStruct>().GetScriptStruct() 
				&& Param.Get<FInstancedStruct>().GetScriptStruct()->IsChildOf(T::StaticStruct()))
			{
				OutArray.Add(Param.Get<FInstancedStruct>().GetMutablePtr<T>());
			}
		}

		return OutArray;
	}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
	template<typename T>
	TArray<const T*> GetTyped() const
	{
		TArray<const T*> OutArray;
		for (FAlterMeshConstStructView Param : FAlterMeshConstStructArrayView(Params))
		{
			if (Param.Get<const FInstancedStruct>().GetScriptStruct() 
				&& Param.Get<const FInstancedStruct>().GetScriptStruct()->IsChildOf(T::StaticStruct()))
			{
				OutArray.Add(Param.Get<const FInstancedStruct>().GetPtr<T>());
			}
		}

		return OutArray;
	}
#endif

	void Add(FInstancedStruct& NewParam)
	{
		Params.Add(NewParam);
	}
	
	// The frame that will be imported from blender
	// values higher than the timeline will wrap to the beggining
	// @see SetFrame
	// @see SetTime
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Target)
	int32 Frame = 0;

	//FAlterMeshInputParams& operator=(const FAlterMeshInputParams& Other);

	// Function needed to pass params from one asset/actor to the other
	// this takes care of duplicating UObject components
	FAlterMeshInputParams& Copy(const FAlterMeshInputParams& Other, UObject* NewOuter);
	
	void Clear();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, NoClear, Meta = (AlterMeshParamCustomDisplay, ExcludeBaseStruct, BaseStruct = "/Script/AlterMesh.AlterMeshParamBase", AllowPrivateAccess = "true"), Category="")
	TArray<FInstancedStruct> Params;

};