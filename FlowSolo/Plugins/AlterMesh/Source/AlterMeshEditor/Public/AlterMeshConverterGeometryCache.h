// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshConverter.h"
#include "AlterMeshConverterAnimationBase.h"
#include "GeometryCacheComponent.h"
#include "AlterMeshConverterGeometryCache.generated.h"

UCLASS(meta=(DisplayName="Convert to GeometryCache animation", AlterMeshPro))
class UAlterMeshConverterGeometryCache : public UAlterMeshConverterAnimationBase
{
	GENERATED_BODY()
public:

	UAlterMeshConverterGeometryCache();

	virtual void Convert(AAlterMeshActor* InActor) override;
	
	virtual void CleanUp() override;
	
};