// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshConverter.h"
#include "AlterMeshConverterStaticMesh.h"
#include "GeometryCacheComponent.h"
#include "AlterMeshConverterAnimationBase.generated.h"

UCLASS(Abstract)
class UAlterMeshConverterAnimationBase : public UAlterMeshConverterStaticMesh
{
	GENERATED_BODY()
public:
	
	UAlterMeshConverterAnimationBase();

	// Inclusive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	int32 InitialFrame = 1;

	// Inclusive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	int32 LastFrame = 60;

	virtual bool ProcessFrames(TFunctionRef<bool(TArray<TSharedPtr<FAlterMeshPrimitive>>, int32)> InFunc);
	
	virtual void Convert(AAlterMeshActor* InActor) override;	
	
	virtual void CleanUp() override;
	
};