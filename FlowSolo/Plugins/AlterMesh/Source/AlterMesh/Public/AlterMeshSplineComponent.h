// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "AlterMeshSplineComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSplineChangeDelegate);

UCLASS(BlueprintType, Blueprintable)
class ALTERMESH_API UAlterMeshSplineComponent : public USplineComponent
{
	GENERATED_BODY()
public:

#if WITH_EDITOR
	virtual void OnComponentCreated() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
#endif

	UPROPERTY(BlueprintReadOnly, Category="")
	FOnSplineChangeDelegate OnSplineChange;
};
