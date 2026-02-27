// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Components/SceneComponent.h"
#include "AlterMeshTransformComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTransformComponentChangeDelegate);

UENUM()
enum class EAlterMeshTransformDrawType : uint8
{
	Point,
	Box,
	Sphere,
	Axis,
};

// Component for transform widgets with custom visualizer
UCLASS()
class ALTERMESH_API UAlterMeshTransformComponent : public USceneComponent
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, Category="")
	FOnTransformComponentChangeDelegate OnChangeDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="")
	EAlterMeshTransformDrawType DrawAs;

#if WITH_EDITOR
	virtual void PostEditComponentMove(bool bFinished) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
