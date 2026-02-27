// Copyright 2023 Aechmea

#include "GeometryParams/AlterMeshGeometryActor.h"
#include "CoreMinimal.h"
#include "AlterMeshSplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GeometryParams/AlterMeshGeometrySpline.h"

#if WITH_EDITOR
#include "DragAndDrop/ActorDragDropOp.h"
#endif


void UAlterMeshGeometryActor::Export(FAlterMeshExport& Exporter)
{
	TArray<USplineComponent*> SplineComponents;
	Actor->GetComponents(SplineComponents);

	Exporter.WriteSingle(SplineComponents.Num());
	
	// Splines	
	if (SplineComponents.Num() > 0)
	{
		for (auto* Spline : SplineComponents)
		{
			UAlterMeshGeometrySpline::ExportSpline(Exporter, Spline, 12);
		}	
		return;
	}

	// Static Meshes
	TArray<FVector3f> Vertices;
	TArray<FVector3f> Normals;
	TArray<int32> Indices;
	TArray<FVector2f> UVs;
	TArray<FVector3f> Tangents;
	TArray<int32> MaterialIndices;

	TArray<UStaticMeshComponent*> Components;
	Actor->GetComponents<UStaticMeshComponent>(Components);

	TArray<AActor*> AttachedActors;
	Actor->GetAttachedActors(AttachedActors, true, true);

	TSet<UStaticMeshComponent*> ComponentsSet;
	ComponentsSet.Append(Components);
	
	for (const auto* AttachedActor : AttachedActors)
	{
		TArray<UStaticMeshComponent*> AttachedActorComponents;
		AttachedActor->GetComponents<UStaticMeshComponent>(AttachedActorComponents);
		AttachedActorComponents.RemoveAll([&ComponentsSet](const auto* Item) { return ComponentsSet.Contains(Item); });
		ComponentsSet.Append(AttachedActorComponents);
		
		Components.Append(AttachedActorComponents);
	}
	
	Components.RemoveAll([](const auto* Item) { return !IsValid(Item->GetStaticMesh()); });

	TMap<UMaterialInterface*, int32> UniqueMaterials;
	int32 IndexOffset = 0;
	for (const UStaticMeshComponent* StaticMeshComponent : Components)
	{
		int32 ExportLod = LOD;
		if (StaticMeshComponent->GetStaticMesh())
		{
			ExportLod = FMath::Clamp(LOD, 0, StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources.Num() - 1);
		}
		
		const int NumSections = StaticMeshComponent->GetStaticMesh()->GetNumSections(ExportLod);
		for (int32 i = 0; i < NumSections; i++)
		{
			int32* MaterialIndex = UniqueMaterials.Find(StaticMeshComponent->GetMaterial(i));
			if (!MaterialIndex)
			{
				MaterialIndex = &UniqueMaterials.Add(StaticMeshComponent->GetMaterial(i), UniqueMaterials.Num());
			}
			FTransform RelativeTransform = Actor->GetRootComponent() == StaticMeshComponent ? FTransform::Identity : StaticMeshComponent->GetComponentTransform().GetRelativeTransform(Actor->GetTransform()); 
			UAlterMeshGeometryAsset::AppendSectionFromStaticMesh(StaticMeshComponent->GetStaticMesh(), ExportLod, i, IndexOffset, FTransform3f(RelativeTransform),
												Vertices, Normals, Indices, UVs, Tangents, MaterialIndices, *MaterialIndex);
		}
	}

	for (FVector3f& Vertex : Vertices)
	{
		// Convert to blender
		Vertex = Exporter.ToBlenderMatrix.TransformPosition(Vertex);
	}
	
	for (FVector3f& Normal : Normals)
	{
		// Convert to blender
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

	FTransform Transform = Actor->GetActorTransform();
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
}

UObject* UAlterMeshGeometryActor::GetAsset()
{
	// todo get used class or duplicate actor
	return nullptr;
}

void UAlterMeshGeometryActor::OnDuplicate(UObject* NewOuter)
{
	Super::OnDuplicate(NewOuter);

	Actor = nullptr;
}

#if WITH_EDITOR
bool UAlterMeshGeometryActor::CanDropOperation(TSharedPtr<FDragDropOperation> DragDropOperation, int32 OperationIndex)
{
	if (DragDropOperation->IsOfType<FActorDragDropOp>())
	{
		TSharedPtr<FActorDragDropOp> ActorDrop = StaticCastSharedPtr<FActorDragDropOp>(DragDropOperation);
		if (ActorDrop->Actors.IsValidIndex(OperationIndex) && ActorDrop->Actors[OperationIndex].IsValid())
		{
			return true;
		}
	}

	return false;
}


void UAlterMeshGeometryActor::OnDropOperation(TSharedPtr<FDragDropOperation> DragDropOperation, int32 OperationIndex)
{
	if (DragDropOperation->IsOfType<FActorDragDropOp>())
	{
		TSharedPtr<FActorDragDropOp> ActorDrop = StaticCastSharedPtr<FActorDragDropOp>(DragDropOperation);

		if (ActorDrop->Actors[OperationIndex].IsValid())
		{
			Actor = ActorDrop->Actors[OperationIndex].Get();
		}
	}
}
#endif