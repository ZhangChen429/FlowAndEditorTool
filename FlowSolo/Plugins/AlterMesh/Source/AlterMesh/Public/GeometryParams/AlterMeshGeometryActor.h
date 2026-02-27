// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshGeometryAsset.h"
#include "GeometryParams/AlterMeshGeometryBase.h"
#include "AlterMeshGeometryActor.generated.h"

// Actor picker geometry interface, including relative transform
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Geometry: Actor"))
class ALTERMESH_API UAlterMeshGeometryActor : public UAlterMeshGeometryBase
{
	GENERATED_BODY()
public:

	virtual void Export(FAlterMeshExport& Exporter) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	AActor* Actor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	int32 LOD;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	bool bInvertUVs;

	virtual bool ShouldExport() override { return !!Actor; }
	virtual UObject* GetAsset() override;

	virtual void OnDuplicate(UObject* NewOuter) override;

#if WITH_EDITOR
	virtual bool CanDropOperation(TSharedPtr<FDragDropOperation> DragDropOperation, int32 OperationIndex) override;
	virtual void OnDropOperation(TSharedPtr<FDragDropOperation> DragDropOperation, int32 OperationIndex) override;
#endif
};
