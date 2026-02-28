// Copyright 2023 Aechmea

#include "GeometryParams/AlterMeshGeometryTransform.h"
#include "AlterMeshActor.h"

#if WITH_EDITOR
void UAlterMeshGeometryTransform::OnDuplicate(UObject* NewOuter)
{
	Super::OnDuplicate(NewOuter);

	TransformComponent = DuplicateObject(TransformComponent, NewOuter);
}

void UAlterMeshGeometryTransform::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property
		&& PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UAlterMeshGeometryTransform, DrawAs))
	{
		if (TransformComponent)
		{
			TransformComponent->DrawAs = DrawAs;
		}
	}
}
#endif


void UAlterMeshGeometryTransform::Initialize(AActor* OwnerActor)
{
	Super::Initialize(OwnerActor);

	if (!TransformComponent)
	{
		TransformComponent = NewObject<UAlterMeshTransformComponent>(OwnerActor, NAME_None, RF_Transactional);
		TransformComponent->AttachToComponent(OwnerActor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		TransformComponent->OnChangeDelegate.AddDynamic(Cast<AAlterMeshActor>(OwnerActor), &AAlterMeshActor::RefreshAsync);
		TransformComponent->RegisterComponent();
		OwnerActor->AddInstanceComponent(TransformComponent);
	}
}

void UAlterMeshGeometryTransform::Cleanup()
{
	Super::Cleanup();

	if (TransformComponent)
	{
		TransformComponent->DestroyComponent();
		TransformComponent = nullptr;
	}
}

void UAlterMeshGeometryTransform::Export(FAlterMeshExport& Exporter)
{
	Super::Export(Exporter);

	FTransform Transform = TransformComponent->GetComponentTransform();
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
}
