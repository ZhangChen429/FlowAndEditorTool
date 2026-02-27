// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshGeometryBase.h"
#include "NiagaraDataInterfaceExport.h"
#include "AlterMeshGeometryParticleData.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

// Allows exporting a point cloud generated from a particle system
// Requires an Export Data Interface using the CallbackParameterName
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Geometry: Particle Data"))
class ALTERMESH_API UAlterMeshGeometryParticleData : public UAlterMeshGeometryBase, public INiagaraParticleCallbackHandler
{
	GENERATED_BODY()
public:

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Subclass and override to change behavior
	virtual void ReceiveParticleData_Implementation(const TArray<FBasicParticleData>& Data, UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset) override;
	
	virtual void Initialize(AActor* OwnerActor) override;
	virtual void Cleanup() override;
	virtual void PreExport(FAlterMeshExport& Exporter) override;
	virtual void Export(FAlterMeshExport& Exporter) override;
	virtual bool ShouldExport() override;

#if WITH_EDITOR
	virtual bool CanDropOperation(TSharedPtr<FDragDropOperation> DragDropOperation, int32 OperationIndex) override;
	virtual void OnDropOperation(TSharedPtr<FDragDropOperation> DragDropOperation, int32 OperationIndex) override;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	float WarmupTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	float TickDelta = 0.016f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	FName DataInterfaceParameterName = FName("User.ExportDI");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	FName CallbackObjectParameterName = FName("User.CallbackHandler");

	UPROPERTY(BlueprintReadOnly, Category="")
	UNiagaraComponent* NiagaraComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	UNiagaraSystem* ParticleSystem;

	UPROPERTY()
	TArray<FVector3f> ParticleLocations;

	// This will make sure your particle system asset has a data interface and callback handler
	// if you want to set them up manually, set this to false
	// You will need to: Create a Export Data Interface and a Callback Handler parameter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category="")
	bool bSetupAssetAutomatically = true;

	
private:

	// Makes sure the asset has a data interface and callback handler
	void SetupAsset(); 
};
