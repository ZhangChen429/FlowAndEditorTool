// Copyright 2023 Aechmea

#include "GeometryParams/AlterMeshGeometryLandscape.h"

#include "EngineUtils.h"
#include "Landscape.h"
#include "LandscapeDataAccess.h"

void UAlterMeshGeometryLandscape::Export(FAlterMeshExport& Exporter)
{
	Super::Export(Exporter);

#if WITH_EDITOR
	ALandscape* Landscape = nullptr;
	for (AActor* Actor : FActorRange(GetWorld()))
	{
		if (Cast<ALandscape>(Actor))
		{
			Landscape = Cast<ALandscape>(Actor);
		}
	}

	if (!Landscape)
	{
		return;
	}

	int32 MinX = MAX_int32, MinY = MAX_int32;
	int32 MaxX = MIN_int32, MaxY = MIN_int32;

	// Find range of entire landscape
	for (int32 ComponentIndex = 0; ComponentIndex < Landscape->LandscapeComponents.Num(); ComponentIndex++)
	{
		ULandscapeComponent* Component = Landscape->LandscapeComponents[ComponentIndex];
		Component->GetComponentExtent(MinX, MinY, MaxX, MaxY);
	}

	// Create and fill in the vertex position data source.
	const int32 ComponentSizeQuads = ((Landscape->ComponentSizeQuads + 1) >> Landscape->ExportLOD) - 1;
	const float ScaleFactor = (float)Landscape->ComponentSizeQuads / (float)ComponentSizeQuads;
	const int32 NumComponents = Landscape->LandscapeComponents.Num();
	const int32 VertexCountPerComponent = FMath::Square(ComponentSizeQuads + 1);
	const int32 VertexCount = NumComponents * VertexCountPerComponent;
	const int32 TriangleCount = NumComponents * FMath::Square(ComponentSizeQuads) * 2;

	TArray<uint8> VisibilityData;
	VisibilityData.Empty(VertexCount);
	VisibilityData.AddZeroed(VertexCount);

	TArray<uint32> Indices;
	TArray<FVector3f> Vertices;
	TArray<FVector3f> Normals;
	TArray<FVector2f> UVs;

	for (int32 ComponentIndex = 0, SelectedComponentIndex = 0; ComponentIndex < Landscape->LandscapeComponents.Num(); ComponentIndex++)
	{
		ULandscapeComponent* Component = Landscape->LandscapeComponents[ComponentIndex];

		if (MaxDistance > 0 && FVector::Distance(Component->Bounds.Origin, Exporter.Location) > MaxDistance)
		{
			continue;
		}
		
		FLandscapeComponentDataInterface CDI(Component, Landscape->ExportLOD);
		const int32 BaseVertIndex = SelectedComponentIndex++ * VertexCountPerComponent;

		TArray<FWeightmapLayerAllocationInfo>& ComponentWeightmapLayerAllocations = Component->GetWeightmapLayerAllocations();
		TArray<uint8> CompVisData;
		for (int32 AllocIdx = 0; AllocIdx < ComponentWeightmapLayerAllocations.Num(); AllocIdx++)
		{
			FWeightmapLayerAllocationInfo& AllocInfo = ComponentWeightmapLayerAllocations[AllocIdx];
			if (AllocInfo.LayerInfo == ALandscapeProxy::VisibilityLayer)
			{
				CDI.GetWeightmapTextureData(AllocInfo.LayerInfo, CompVisData);
			}
		}

		if (CompVisData.Num() > 0)
		{
			for (int32 i = 0; i < VertexCountPerComponent; ++i)
			{
				VisibilityData[BaseVertIndex + i] = CompVisData[CDI.VertexIndexToTexel(i)];
			}
		}

		for (int32 VertIndex = 0; VertIndex < VertexCountPerComponent; VertIndex++)
		{
			int32 VertX, VertY;
			CDI.VertexIndexToXY(VertIndex, VertX, VertY);
			
			FVector Vertex = CDI.GetLocalVertex(VertX, VertY);

			Vertex = Component->GetRelativeTransform().TransformPosition(Vertex);
			
			Vertices.Add(Exporter.ToBlenderMatrix.TransformPosition(FVector3f(Vertex)));

			FVector Normal, TangentX, TangentY;
			CDI.GetLocalTangentVectors(VertX, VertY, TangentX, TangentY, Normal);
			Normal /= Component->GetComponentTransform().GetScale3D();
			FVector3f TransformedNormal = Exporter.ToBlenderMatrix.GetMatrixWithoutScale().Inverse().GetTransposed().TransformVector(FVector3f(Normal)).GetSafeNormal();
			Normals.Add(TransformedNormal);

			FVector2f UV = FVector2f(VertX * ScaleFactor + Component->GetSectionBase().X, VertY * ScaleFactor + Component->GetSectionBase().Y);
			UVs.Add(UV);
		}

		const int32 VisThreshold = 170;

		for (int32 Y = 0; Y < ComponentSizeQuads; Y++)
		{
			for (int32 X = 0; X < ComponentSizeQuads; X++)
			{
				if (VisibilityData[BaseVertIndex + Y * (ComponentSizeQuads + 1) + X] < VisThreshold)
				{
					Indices.Add(BaseVertIndex + (X + 0) + (Y + 0)*(ComponentSizeQuads + 1));
					Indices.Add(BaseVertIndex + (X + 1) + (Y + 1)*(ComponentSizeQuads + 1));
					Indices.Add(BaseVertIndex + (X + 1) + (Y + 0)*(ComponentSizeQuads + 1));

					Indices.Add(BaseVertIndex + (X + 0) + (Y + 0)*(ComponentSizeQuads + 1));
					Indices.Add(BaseVertIndex + (X + 0) + (Y + 1)*(ComponentSizeQuads + 1));
					Indices.Add(BaseVertIndex + (X + 1) + (Y + 1)*(ComponentSizeQuads + 1));
				}
			}
		}
	}
	
	TArray<int32> MaterialIndices;
	MaterialIndices.AddZeroed(Indices.Num() / 3);	

	Exporter.WriteArray(Vertices);
	Exporter.WriteArray(Normals);
	Exporter.WriteArray(Indices);

	if (bInvertUVs)
	{
		Exporter.InvertUVs(UVs);
	}
	
	Exporter.WriteArray(UVs);
	Exporter.WriteArray(MaterialIndices);

	FTransform Transform = Landscape->GetActorTransform();
	Transform.AddToTranslation(-Exporter.Location);

	Transform.SetScale3D(FVector(Transform.GetScale3D().Y,
								Transform.GetScale3D().X,
								Transform.GetScale3D().Z));
	
	Transform.SetRotation(FQuat(Transform.GetRotation().Y,
								Transform.GetRotation().X,
								-Transform.GetRotation().Z,
								Transform.GetRotation().W));
	
	Transform.SetTranslation(FMatrix(Exporter.ToBlenderMatrix).TransformPosition(Transform.GetTranslation()));
	
	Exporter.WriteSingle(FMatrix44f(Transform.ToMatrixWithScale().GetTransposed()));
	
	Exporter.WriteSingle((GetAsset() ? FName(GetAsset()->GetPathName()) : NAME_None).ToUnstableInt());
#endif
}

bool UAlterMeshGeometryLandscape::ShouldExport()
{
	ALandscape* Landscape = nullptr;
	for (AActor* Actor : FActorRange(GetWorld()))
	{
		if (Cast<ALandscape>(Actor))
		{
			Landscape = Cast<ALandscape>(Actor);
		}
	}

	return !!Landscape;
}
