// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "CustomDetailsViewContent.generated.h"

/**
 * 
 */
UCLASS()
class TOUCHPROPERTY_API UCustomDetailsViewContent : public UObject
{
	GENERATED_BODY()
	public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Details)
	UStaticMesh* DetailsMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Details)
	UMaterial* DetailsMaterial;
};
UCLASS()
class TOUCHPROPERTY_API UTouchCameraPreviewObject : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ValueFloat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* StaticMesh;
};
