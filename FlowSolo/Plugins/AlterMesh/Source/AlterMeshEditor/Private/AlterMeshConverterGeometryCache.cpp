// Copyright 2023 Aechmea

#include "AlterMeshConverterGeometryCache.h"

UAlterMeshConverterGeometryCache::UAlterMeshConverterGeometryCache()
{
}

void UAlterMeshConverterGeometryCache::Convert(AAlterMeshActor* InActor)
{
#if !ALTERMESH_FREE
	UGeometryCacheComponent* NewComponent = NewObject<UGeometryCacheComponent>(GetOuter());
	NewComponent->RegisterComponent();
	NewComponent->AttachToComponent(GetTypedOuter<AActor>()->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
	GetTypedOuter<AActor>()->AddInstanceComponent(NewComponent);
	SpawnedComponents.Add(NewComponent);
		
	Super::Convert(InActor);
#endif
}

void UAlterMeshConverterGeometryCache::CleanUp()
{
	Super::CleanUp();

	for (UActorComponent* Component : SpawnedComponents)
	{
		Component->DestroyComponent();
	}

	SpawnedComponents.Empty();
}
