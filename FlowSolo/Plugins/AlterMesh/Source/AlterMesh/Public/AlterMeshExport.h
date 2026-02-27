// Copyright 2023 Aechmea

#pragma once
#include "CoreMinimal.h"
#include "AlterMeshParam.h"
#include "GameFramework/Actor.h"

#include <Extern/AlterMesh.h>

#include "AlterMeshHandle.h"

#pragma comment(lib, "AlterMesh.lib")

class UAlterMeshAssetInterface;

DECLARE_LOG_CATEGORY_EXTERN(LogAlterMeshExport, Log, All);

struct ALTERMESH_API FAlterMeshExport
{
	FAlterMeshExport(const TSharedPtr<FAlterMeshHandle, ESPMode::ThreadSafe>& AlterMeshHandle, const FAlterMeshInputParams& Params, TWeakObjectPtr<UAlterMeshAssetInterface> Asset, TWeakObjectPtr<AActor> Actor)
		: 
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
		Params(Params),
#else
		Params(const_cast<FAlterMeshInputParams&>(Params)),	
#endif
		AlterMeshHandle(AlterMeshHandle),
		Asset(Asset)
	{
		Location = Actor.IsValid() ? Actor->GetActorLocation() : FVector::ZeroVector;
	}

public:
	// Does the thing
	void PreExport();
	void Export();

	// Helper
	FMatrix44f ToBlenderMatrix = FTransform3f(FRotator3f(0,90,0), FVector3f::ZeroVector, FVector3f(-0.01,0.01,0.01)).ToMatrixWithScale();

	// Helper function to write an array of values
	template<typename T>
	void WriteArray(TArray<T>& InArray)
	{
		Write(AlterMeshHandle->Get(), reinterpret_cast<const char*>(InArray.GetData()), InArray.Num() * sizeof(T));
	}

	// Helper function to write a single value
	template<typename T>
	void WriteSingle(const T& InValue)
	{
		Write(AlterMeshHandle->Get(), reinterpret_cast<const char*>(&InValue), sizeof(T));
	}

	void InvertUVs(TArray<FVector2f>& OutUVs);

	// Location of the exporter in world
	// Used if you need to export relative to this
	FVector Location;
	
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
	const FAlterMeshInputParams& Params;
#else
	FAlterMeshInputParams& Params;
#endif	
	
private:
	// Handle to the AlterMesh library
	TSharedPtr<FAlterMeshHandle, ESPMode::ThreadSafe> AlterMeshHandle = nullptr;

	const TWeakObjectPtr<UAlterMeshAssetInterface> Asset;
	
};