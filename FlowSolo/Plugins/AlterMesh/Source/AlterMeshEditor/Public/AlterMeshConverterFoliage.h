// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshConverter.h"
#include "AlterMeshConverterStaticMesh.h"
#include "InstancedFoliage.h"
#include "AlterMeshConverterFoliage.generated.h"

USTRUCT()
struct FFoliageIndicesWrapper
{
	GENERATED_BODY()
public:
	TArray<int32> Indices;
};

// Stamp GN result into foliage system
// Converts to static mesh before converting to foliage
UCLASS(meta=(DisplayName="Convert to Foliage instances", AlterMeshPro))
class UAlterMeshConverterFoliage : public UAlterMeshConverterStaticMesh
{
	GENERATED_BODY()
public:

	UAlterMeshConverterFoliage();

	UPROPERTY()
	FGuid Guid;
	
	UPROPERTY()
	TMap<UObject*, FFoliageIndicesWrapper> FoliageInstances;
	
	virtual void Convert(AAlterMeshActor* InActor) override;

	/* ~ Begin UAlterMeshConverterStaticMesh interface */
	virtual void PlaceAssets(AAlterMeshActor* InActor) override;
	/* ~ End UAlterMeshConverterStaticMesh interface */
	
	virtual void CleanUp() override;

};