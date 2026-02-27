// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshGeometryBase.h"
#include "AlterMeshTransformComponent.h"
#include "AlterMeshGeometryTransform.generated.h"

// Sends only transform information to geometry nodes
// Can be set through code by editing the TransformComponent
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Geometry: Transform only"))
class ALTERMESH_API UAlterMeshGeometryTransform : public UAlterMeshGeometryBase
{
	GENERATED_BODY()
public:

	virtual void OnDuplicate(UObject* NewOuter) override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="")
	EAlterMeshTransformDrawType DrawAs;

	virtual bool ShouldExport() override { return !!TransformComponent; };

	virtual void Initialize(AActor* OwnerActor) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="")
	UAlterMeshTransformComponent* TransformComponent = nullptr;

	virtual void Cleanup() override;

	virtual void Export(FAlterMeshExport& Exporter) override;
};
