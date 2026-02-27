// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshConverter.h"
#include "AlterMeshConverterAnimationBase.h"
#include "GeometryCacheComponent.h"
#include "AlterMeshConverterVertexAnimation.generated.h"

UCLASS(meta=(DisplayName="Convert to Vertex animation texture (VAT)", AlterMeshPro))
class UAlterMeshConverterVertexAnimation : public UAlterMeshConverterAnimationBase
{
	GENERATED_BODY()
public:

	UPROPERTY()
	UTexture2D* GeneratedTexture;
	
	UAlterMeshConverterVertexAnimation();

	virtual void Convert(AAlterMeshActor* InActor) override;
	
	virtual void CleanUp() override;
	
};