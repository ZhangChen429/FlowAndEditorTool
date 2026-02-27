// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshGeometryBase.h"
#include "Components/SplineComponent.h"
#include "AlterMeshGeometrySpline.generated.h"

class UAlterMeshSplineComponent;

// Use a spline to control a curve object
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Geometry: Spline"))
class ALTERMESH_API UAlterMeshGeometrySpline : public UAlterMeshGeometryBase
{
	GENERATED_BODY()
public:

	virtual bool ShouldExport() override { return !!SplineComponent; };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="")
	UAlterMeshSplineComponent* SplineComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spline", meta=(ClampMin=0, ClampMax=64))
	int32 Resolution = 12;

	virtual void Initialize(AActor* OwnerActor) override;

	virtual void Cleanup() override;

	virtual void Export(FAlterMeshExport& Exporter) override;
	virtual void ImportDefaults(FAlterMeshImport& Importer) override;

	virtual void OnDuplicate(UObject* NewOuter) override;

	UPROPERTY()
	TArray<FVector> DefaultLocations = {FVector::ZeroVector, FVector(0,0,100)};
	
	UPROPERTY()
	TEnumAsByte<ESplinePointType::Type> CurveType = ESplinePointType::Curve;

	static void ExportSpline(FAlterMeshExport& Exporter, USplineComponent* SplineComponent, int32 Resolution);
};
