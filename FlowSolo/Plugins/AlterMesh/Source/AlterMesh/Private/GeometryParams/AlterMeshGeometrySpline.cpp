// Copyright 2023 Aechmea

#include "GeometryParams/AlterMeshGeometrySpline.h"

#include "AlterMeshActor.h"
#include "AlterMeshSplineComponent.h"


void UAlterMeshGeometrySpline::Initialize(AActor* OwnerActor)
{
	Super::Initialize(OwnerActor);

	if (!SplineComponent)
	{
		SplineComponent = NewObject<UAlterMeshSplineComponent>(OwnerActor, NAME_None, RF_Transactional);
		SplineComponent->AttachToComponent(OwnerActor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		SplineComponent->RegisterComponent();
		SplineComponent->ClearSplinePoints();

		for (const FVector& Location : DefaultLocations)
		{
			SplineComponent->AddSplinePoint(Location, ESplineCoordinateSpace::Local, false);
			SplineComponent->SetSplinePointType(SplineComponent->GetNumberOfSplinePoints()-1, CurveType);
		}
	
		SplineComponent->UpdateSpline();
		OwnerActor->AddInstanceComponent(SplineComponent);
		SplineComponent->OnSplineChange.AddDynamic(Cast<AAlterMeshActor>(OwnerActor), &AAlterMeshActor::RefreshAsync);
	}
}

void UAlterMeshGeometrySpline::Cleanup()
{
	if (SplineComponent)
	{
		//GetTypedOuter<AActor>()->RemoveInstanceComponent(SplineComponent);
		SplineComponent->DestroyComponent();
		SplineComponent = nullptr;
	}

	Super::Cleanup();
}

void UAlterMeshGeometrySpline::Export(FAlterMeshExport& Exporter)
{
	Super::Export(Exporter);
	ExportSpline(Exporter, SplineComponent, Resolution);
}

enum class ECurveType : uint8
{
	Bezier = 0,
	Poly = 1,
};

void UAlterMeshGeometrySpline::ImportDefaults(FAlterMeshImport& Importer)
{
	Super::ImportDefaults(Importer);
	TArrayView<FVector3f> Locations = Importer.ReadArray<FVector3f>();

	for (FVector3f& Location : Locations)
	{
		Location = Importer.ToUEMatrix.TransformPosition(Location);
	}

	DefaultLocations = Locations;

	uint8 DefaultPointType = Importer.ReadValue<uint8>();
	CurveType = DefaultPointType == 1 /*Poly*/ ? ESplinePointType::Constant : ESplinePointType::Curve;
}

void UAlterMeshGeometrySpline::OnDuplicate(UObject* NewOuter)
{
	Super::OnDuplicate(NewOuter);
	
	SplineComponent = DuplicateObject(SplineComponent, NewOuter);
	ReattachComponent(SplineComponent, NewOuter);
}

void UAlterMeshGeometrySpline::ExportSpline(FAlterMeshExport& Exporter, USplineComponent* SplineComponent, int32 Resolution)
{
	TArray<FVector3f> Locations;
	TArray<FVector3f> Scales;
	TArray<float> Rolls;
	TArray<FVector3f> LeaveTangents;
	TArray<FVector3f> ArriveTangents;
	TArray<int32> PointTypes;

	const bool bCyclic = SplineComponent->IsClosedLoop();

	for (int32 i = 0; i < SplineComponent->GetNumberOfSplinePoints(); i++)
	{
		FVector RelativeLocation = Exporter.Location - SplineComponent->GetComponentLocation();
		Locations.Add(Exporter.ToBlenderMatrix.TransformVector(FVector3f(SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local) - RelativeLocation)));
		Scales.Add(FVector3f(SplineComponent->GetScaleAtSplinePoint(i)));
		Rolls.Add(SplineComponent->GetRollAtSplinePoint(i, ESplineCoordinateSpace::Local));
		LeaveTangents.Add(Exporter.ToBlenderMatrix.TransformVector(FVector3f(SplineComponent->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local)) / 3.f));
		ArriveTangents.Add(Exporter.ToBlenderMatrix.TransformVector(FVector3f(SplineComponent->GetArriveTangentAtSplinePoint(i, ESplineCoordinateSpace::Local)) / 3.f));
		PointTypes.Add(static_cast<uint32>(SplineComponent->GetSplinePointType(i)));
	}

	Exporter.WriteSingle(Resolution);
	Exporter.WriteSingle(bCyclic);
	Exporter.WriteArray(Locations);
	Exporter.WriteArray(Scales);
	Exporter.WriteArray(Rolls);
	Exporter.WriteArray(LeaveTangents);
	Exporter.WriteArray(ArriveTangents);
	Exporter.WriteArray(PointTypes);
}