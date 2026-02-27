// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshConverter.h"
#include "AlterMeshConverterNone.generated.h"

/* Used for assets that shouldn't be converted into a Mesh/etc
 * Useful if you just want it to be shown during runtime/cinematics
 * WARNING: Runtime requires shipping blender with your project
 * make sure you understand the implications
 * http://altermesh.com/docs/runtime */
UCLASS(meta=(DisplayName="Do not convert (Runtime only)"))
class UAlterMeshConverterNone : public UAlterMeshConverterBase
{
	GENERATED_BODY()
public:
	
	UAlterMeshConverterNone();

	virtual void Convert(AAlterMeshActor* InActor) override;	
	
	virtual void CleanUp() override;
	
};