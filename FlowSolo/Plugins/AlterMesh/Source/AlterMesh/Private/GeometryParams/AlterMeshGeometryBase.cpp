// Copyright 2023 Aechmea

#include "GeometryParams/AlterMeshGeometryBase.h"

UAlterMeshGeometryBase::UAlterMeshGeometryBase()
{
}

void UAlterMeshGeometryBase::Initialize(AActor* OwnerActor)
{

}

void UAlterMeshGeometryBase::Cleanup()
{

}

void UAlterMeshGeometryBase::OnDuplicate(UObject* NewOuter)
{
}

void UAlterMeshGeometryBase::ReattachComponent(USceneComponent* Component, UObject* NewOuter)
{
	if (Component)
	{
		Component->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		if (AActor* OuterActor = Cast<AActor>(NewOuter))
		{
			Component->AttachToComponent(OuterActor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		}
	}
}
