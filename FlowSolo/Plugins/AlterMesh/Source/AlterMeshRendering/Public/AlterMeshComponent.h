// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshSection.h"
#include "EngineUtils.h"
#include "Components/MeshComponent.h"
#include "GenericPlatform/ICursor.h"
#include "AlterMeshComponent.generated.h"

class FPrimitiveSceneProxy;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnMaterialChanged,const class UAlterMeshComponent*, int32, UMaterialInterface*)

struct ALTERMESHRENDERING_API HAlterMeshComponentProxy : public HActor
{
	DECLARE_HIT_PROXY();

	HAlterMeshComponentProxy(AActor* InActor, const UPrimitiveComponent* InComponent, const int32 InSectionIndex, const int32 InMaterialIndex)
	: HActor(InActor, InComponent, InSectionIndex, InMaterialIndex)
	{}

	virtual EMouseCursor::Type GetMouseCursor() override
	{
		return  EMouseCursor::Default;
	}
};

UCLASS(hidecategories=(Object, Physics, Activation, "Components|Activation"), editinlinenew, meta=(BlueprintSpawnableComponent), ClassGroup=Rendering)
class ALTERMESHRENDERING_API UAlterMeshComponent : public UMeshComponent
{
	GENERATED_BODY()
public:

	UAlterMeshComponent();

	TArray<FAlterMeshSection> Sections;

	FBox BoundingBox;
	FOnMaterialChanged OnMaterialChanged;

	virtual void OnRegister() override;

	//~ Begin UActorComponent Interface.
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	//~ End UActorComponent Interface.

	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual void UpdateBounds() override;
	//~ End USceneComponent Interface.

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UMeshComponent Interface.
	virtual int32 GetNumMaterials() const override;
	virtual TArray<FName> GetMaterialSlotNames() const override;
	virtual int32 GetMaterialIndex(FName MaterialSlotName) const override;
	virtual void SetMaterial(int32 ElementIndex, UMaterialInterface* Material) override;
	//~ End UMeshComponent Interface.

private:

	friend class FAlterMeshSceneProxy;
};


