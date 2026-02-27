// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "StaticMeshResources.h"
#include "GeometryParams/AlterMeshGeometryBase.h"
#include "AlterMeshGeometryAsset.generated.h"

// Export StaticMeshes or blueprints containing static meshes
// If the exported geometry is not realized (with realize instances node) it will instance it instead
// allowing for instanced particles, light, components etc.
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Geometry: StaticMesh or Blueprint"))
class ALTERMESH_API UAlterMeshGeometryAsset : public UAlterMeshGeometryBase
{
	GENERATED_BODY()
public:

	static void AppendSectionFromStaticMesh(UStaticMesh* MeshAsset, int32 LodIndex, int32 SectionIndex, int32& IndexOffset, FTransform3f Transform,
														TArray<FVector3f>& Vertices, TArray<FVector3f>& Normals, TArray<int32>& Indices,
														TArray<FVector2f>& UVs, TArray<FVector3f>& Tangents, TArray<int32>& MaterialIndices, int32 MaterialIndex);
	static int32 GetIndexInSection(int32 MeshVertIndex, TMap<int32, int32>& MeshToSectionVertMap, const FStaticMeshVertexBuffers& VertexBuffers, TArray<FVector3f>& Vertices, TArray<FVector3f>& Normals, TArray<FVector2f>& UVs, TArray<FVector3f>& Tangents);

	virtual bool ShouldExport() override { return !!Object; };

	// Replaces the generated instances with this asset upon conversion
	virtual UObject* GetAsset() override { return Object; };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowedClasses="/Script/Engine.StaticMesh, /Script/Engine.Blueprint"), Category="")
	UObject* Object = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	FTransform TransformOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	int32 LOD;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	bool bInvertUVs;

	virtual void Export(FAlterMeshExport& Exporter) override;

#if WITH_EDITOR
	virtual bool CanDropOperation(TSharedPtr<FDragDropOperation> DragDropOperation, int32 OperationIndex) override;
	virtual void OnDropOperation(TSharedPtr<FDragDropOperation> DragDropOperation, int32 OperationIndex) override;
#endif
};
