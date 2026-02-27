// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AlterMeshConverter.h"
#include "AlterMeshEditorLibrary.generated.h"

class AAlterMeshActor;
/**
 *
 */
UCLASS()
class ALTERMESHEDITOR_API UAlterMeshEditorLibrary : public UObject
{
	GENERATED_BODY()
public:

	// Convert using specified class, or shows a picker if class was null
	UFUNCTION(BlueprintCallable, Category="AlterMesh", meta = (DefaultToSelf=InActor))
	static void SetConverterClass(AAlterMeshActor* InActor, TSubclassOf<UAlterMeshConverterBase> InClass);
};
