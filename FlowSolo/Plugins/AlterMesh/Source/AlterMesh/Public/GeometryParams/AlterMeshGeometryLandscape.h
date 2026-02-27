// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "GeometryParams/AlterMeshGeometryBase.h"
#include "AlterMeshGeometryLandscape.generated.h"

// Landscape geometry interface
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Geometry: Landscape"))
class ALTERMESH_API UAlterMeshGeometryLandscape : public UAlterMeshGeometryBase
{
	GENERATED_BODY()
public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="")
	bool bInvertUVs;

	// Max distance from component to actor that will be exported
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="")
	float MaxDistance = 8000.0f;
	
	virtual void Export(FAlterMeshExport& Exporter) override;

	virtual bool ShouldExport() override;
};
