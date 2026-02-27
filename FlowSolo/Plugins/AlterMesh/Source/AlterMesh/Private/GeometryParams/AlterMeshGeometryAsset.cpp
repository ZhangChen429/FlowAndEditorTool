// Copyright 2023 Aechmea

#include "GeometryParams/AlterMeshGeometryAsset.h"
#include "Engine/World.h"

#if WITH_EDITOR
#include "DragAndDrop/AssetDragDropOp.h"
#endif

#include "Components/StaticMeshComponent.h"
#include "Engine/SCS_Node.h"
#include "Engine/StaticMesh.h"

void UAlterMeshGeometryAsset::Export(FAlterMeshExport& Exporter)
{
	Super::Export(Exporter);

	TArray<FVector3f> Vertices;
	TArray<FVector3f> Normals;
	TArray<int32> Indices;
	TArray<FVector2f> UVs;
	TArray<FVector3f> Tangents;
	TArray<int32> MaterialIndices;

	// Static Mesh
	if (UStaticMesh* MeshAsset = Cast<UStaticMesh>(Object))
	{
		int32 IndexOffset = 0;
		for (int32 i = 0; i < MeshAsset->GetNumSections(LOD); i++)
		{
			AppendSectionFromStaticMesh(MeshAsset, LOD, i, IndexOffset, FTransform3f::Identity, Vertices, Normals, Indices, UVs, Tangents, MaterialIndices, i);
		}
	}

	// BP
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Object))
	{
		UClass* ParentClass = Blueprint->GeneratedClass;
		if (ParentClass->IsChildOf(AActor::StaticClass()))
		{
			TArray<UStaticMeshComponent*> Components;

			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnInfo.bNoFail = true;
			SpawnInfo.ObjectFlags = RF_Transient;

			AActor* Actor = GetWorld()->SpawnActor<AActor>(ParentClass, SpawnInfo);
			Actor->GetComponents(Components);
			Actor->Destroy();

			int32 IndexOffset = 0;
			int32 MaterialIndexOffset = 0;
			TMap<UMaterialInterface*, int32> UniqueMaterials;
			for (const UStaticMeshComponent* Component : Components)
			{
				int32 NumSections = Component->GetStaticMesh()->GetNumSections(0);
				for (int32 i = 0; i < NumSections; i++)
				{
					int32* MaterialIndex = UniqueMaterials.Find(Component->GetMaterial(i));
					if (!MaterialIndex)
					{
						MaterialIndex = &UniqueMaterials.Add(Component->GetMaterial(i), MaterialIndexOffset);
						MaterialIndexOffset++;
					}
					AppendSectionFromStaticMesh(Component->GetStaticMesh(), LOD, i, IndexOffset, FTransform3f(Component->GetRelativeTransform()),
						Vertices, Normals, Indices, UVs, Tangents, MaterialIndices, *MaterialIndex);
				}
			}
		}
	}

	for (FVector3f& Vertex : Vertices)
	{
		Vertex = Exporter.ToBlenderMatrix.TransformPosition(Vertex);
	}

	for (FVector3f& Normal : Normals)
	{
		Normal = Exporter.ToBlenderMatrix.GetMatrixWithoutScale().Inverse().GetTransposed().TransformVector(Normal).GetSafeNormal();
	}

	Exporter.WriteArray(Vertices);
	Exporter.WriteArray(Normals);
	Exporter.WriteArray(Indices);

	if (bInvertUVs)
	{
		Exporter.InvertUVs(UVs);
	}
	
	Exporter.WriteArray(UVs);
	Exporter.WriteArray(MaterialIndices);
	
	FTransform Transform = TransformOverride;
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
}

#if WITH_EDITOR
bool UAlterMeshGeometryAsset::CanDropOperation(TSharedPtr<FDragDropOperation> DragDropOperation, int32 OperationIndex)
{
	if (DragDropOperation->IsOfType<FAssetDragDropOp>())
	{
		TSharedPtr<FAssetDragDropOp> AssetDrop = StaticCastSharedPtr<FAssetDragDropOp>(DragDropOperation);
		if (AssetDrop->GetAssets().IsValidIndex(OperationIndex))
		{
			FAssetData AssetData = AssetDrop->GetAssets()[OperationIndex];
			if ( AssetData.GetAsset()->IsA(UStaticMesh::StaticClass())
				|| AssetData.GetAsset()->IsA(UBlueprint::StaticClass()))
			{
				return true;
			}
		}
	}

	return false;
}

void UAlterMeshGeometryAsset::OnDropOperation(TSharedPtr<FDragDropOperation> DragDropOperation, int32 OperationIndex)
{
	Super::OnDropOperation(DragDropOperation, OperationIndex);

	if (DragDropOperation->IsOfType<FAssetDragDropOp>())
	{
		TSharedPtr<FAssetDragDropOp> AssetDrop = StaticCastSharedPtr<FAssetDragDropOp>(DragDropOperation);

		FAssetData AssetData = AssetDrop->GetAssets()[OperationIndex];
		if ( AssetData.GetAsset()->IsA(UStaticMesh::StaticClass())
			|| AssetData.GetAsset()->IsA(UBlueprint::StaticClass()))
		{
			Object = AssetData.GetAsset();
		}
	}
}
#endif

void UAlterMeshGeometryAsset::AppendSectionFromStaticMesh(UStaticMesh* MeshAsset, int32 LodIndex, int32 SectionIndex, int32& IndexOffset, FTransform3f Transform,
	TArray<FVector3f>& Vertices, TArray<FVector3f>& Normals, TArray<int32>& Indices, TArray<FVector2f>& UVs, TArray<FVector3f>& Tangents, TArray<int32>& MaterialIndices, int32 MaterialIndex)
{
	TArray<FVector3f> SectionVertices;
	TArray<FVector3f> SectionNormals;
	TArray<int32> SectionIndices;
	TArray<FVector2f> SectionUVs;
	TArray<FVector3f> SectionTangents;

	if (MeshAsset)
	{
		LodIndex = FMath::Clamp(LodIndex, 0, MeshAsset->GetRenderData()->LODResources.Num()-1);
		if (MeshAsset->GetRenderData() != nullptr && MeshAsset->GetRenderData()->LODResources.IsValidIndex(LodIndex))
		{
			const FStaticMeshLODResources& LOD = MeshAsset->GetRenderData()->LODResources[LodIndex];
			if (LOD.Sections.IsValidIndex(SectionIndex))
			{
				// Empty output buffers
				SectionVertices.Reset();
				SectionIndices.Reset();
				SectionNormals.Reset();
				SectionUVs.Reset();
				SectionTangents.Reset();

				// Map from vert buffer for whole mesh to vert buffer for section of interest
				TMap<int32, int32> MeshToSectionVertMap;

				const FStaticMeshSection& Section = LOD.Sections[SectionIndex];
				const uint32 OnePastLastIndex = Section.FirstIndex + Section.NumTriangles * 3;
				FIndexArrayView IndexArrayView = LOD.IndexBuffer.GetArrayView();

				// Iterate over section index buffer, copying verts as needed
				for (uint32 i = Section.FirstIndex; i < OnePastLastIndex; i++)
				{
					uint32 MeshVertIndex = IndexArrayView[i];

					// See if we have this vert already in our section vert buffer, and copy vert in if not
					int32 SectionVertIndex = GetIndexInSection(MeshVertIndex, MeshToSectionVertMap, LOD.VertexBuffers, SectionVertices, SectionNormals, SectionUVs, SectionTangents);

					// Add to index buffer
					SectionIndices.Add(SectionVertIndex);
				}
			}
		}
	}

	if (!Transform.Equals(FTransform3f::Identity))
	{
		for (FVector3f& Vertex : SectionVertices)
		{
			Vertex = Transform.TransformPosition(Vertex);
		}

		for (FVector3f& Normal : SectionNormals)
		{
			Normal = Transform.TransformVector(Normal);
			Normal.Normalize();
		}
	}

	MaterialIndices.Reserve(MaterialIndices.Num() + SectionIndices.Num()/3);
	for (int32 i = 0; i < SectionIndices.Num()/3; i++)
	{
		MaterialIndices.Add(MaterialIndex);
	}

	Vertices.Append(SectionVertices);
	Normals.Append(SectionNormals);

	if (IndexOffset > 0)
	{
		for (int32& Index : SectionIndices)
		{
			Index += IndexOffset;
		}
	}

	Indices.Append(SectionIndices);
	UVs.Append(SectionUVs);
	Tangents.Append(SectionTangents);
	IndexOffset += SectionVertices.Num();
}

int32 UAlterMeshGeometryAsset::GetIndexInSection(int32 MeshVertIndex, TMap<int32, int32>& MeshToSectionVertMap, const FStaticMeshVertexBuffers& VertexBuffers, TArray<FVector3f>& Vertices, TArray<FVector3f>& Normals, TArray<FVector2f>& UVs, TArray<FVector3f>& Tangents)
{
	int32* NewIndexPtr = MeshToSectionVertMap.Find(MeshVertIndex);
	if (NewIndexPtr != nullptr)
	{
		return *NewIndexPtr;
	}
	else
	{
		// Copy position
		int32 SectionVertIndex = Vertices.Add(VertexBuffers.PositionVertexBuffer.VertexPosition(MeshVertIndex));

		// Copy normal
		Normals.Add(VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(MeshVertIndex));
		check(Normals.Num() == Vertices.Num());

		// Copy UVs
		UVs.Add(VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(MeshVertIndex, 0));
		check(UVs.Num() == Vertices.Num());

		// Copy tangents
		FVector4f TangentX = VertexBuffers.StaticMeshVertexBuffer.VertexTangentX(MeshVertIndex);

		if (TangentX.W < 0.f)
		{
			TangentX.Y *= -1;
		}

		Tangents.Add(TangentX);
		check(Tangents.Num() == Vertices.Num());

		MeshToSectionVertMap.Add(MeshVertIndex, SectionVertIndex);

		return SectionVertIndex;
	}
}