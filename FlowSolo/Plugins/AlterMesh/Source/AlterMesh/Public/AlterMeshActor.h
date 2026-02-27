// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshInstance.h"
#include "AlterMeshParam.h"
#include "CommonParams/AlterMeshGeometryParam.h"
#include "GameFramework/Actor.h"
#include "AlterMeshActor.generated.h"

class UAlterMeshComponent;
class UAlterMeshProvider;
class USplineComponent;
class UAlterMeshAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOutProcDelegate, FString, Output);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnImportDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBecomePreviewDelegate);
DECLARE_DELEGATE(FOnConvertClicked);

USTRUCT(BlueprintType)
struct FAlterMeshMaterial
{
	GENERATED_BODY()
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="")
	FName SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="")
	UMaterialInterface* Material;
};

UCLASS(BlueprintType, Blueprintable, hidecategories = (Mobile, HLOD, Physics, Input, Replication))
class ALTERMESH_API AAlterMeshActor : public AActor
{
	GENERATED_BODY()

public:
	AAlterMeshActor();

protected:

	virtual void Tick(float DeltaSeconds) override;
	virtual void Destroyed() override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true;};
	virtual void PostLoad() override;
	virtual void PostActorCreated() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
	virtual void PostEditUndo() override;
#endif

public:

	virtual void BeginPlay() override;

	UPROPERTY()
	USceneComponent* RootSceneComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UBillboardComponent* BillboardComponent;
#endif
	
	UPROPERTY(Transient)
	TArray<UAlterMeshComponent*> AlterMeshComponents;

	UPROPERTY(Transient)
	TArray<UActorComponent*> AssetInstanceComponents;
	
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, Category="")
	FOnImportDelegate OnImportDelegate;

	UPROPERTY(Transient, BlueprintReadOnly, Category="")
	UAlterMeshInstance* BlenderInstance;

	UFUNCTION(BlueprintImplementableEvent)
	void OnImportEvent();

	UPROPERTY(EditAnywhere, Category=Asset)
	UAlterMeshAssetInterface* Asset;

	// Override the default materials from the asset
	// These materials may change if the generated geometry has different material indices
	// If you want to automatically set materials based on name use the AlterMesh asset editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, Category="Materials")
	TArray<FAlterMeshMaterial> OverrideMaterials;

	// Total triangle count, including intances triangles
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="Statistics")
	int32 TriangleCount;

	// Whether background process is running
	// Max idle time can be changed on project configs
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="Statistics")
	EAlterMeshInstanceState InstanceState;

	// Last UE4 import time in seconds
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="Statistics")
	float ImportTime;

protected:
	// Wheter actor is in preview state, or baked into an asset
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Statistics")
	bool bPreview = false;

public:

	void SetPreview(bool bInPreview);
	bool GetPreview() const { return bPreview; };

	// Recalculate the nodetree and update the mesh
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Actions")
	void RefreshSync();
	
	UFUNCTION(BlueprintCallable, Category="")
	void RefreshAsync();
	
	// Kill blender.
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Actions")
	void StopInstance();

	// Convert this preview into a proper Mesh / Foliage / VatAnim, etc.
	// ** Tips:
	// ** Set a default converter class on your AlterMeshAsset
	// ** Saving the map will call Convert automatically
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Actions")
	void Convert();

	FOnConvertClicked OnConvertClicked;

	// Lock your parameters so this actor cannot be changed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	bool bLocked = false;
	
	// Export your parameters for blender and reimport the mesh upon dragging this actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	bool bExportOnMove = false;

	// Export your parameters for blender and reimport the mesh upon dragging and dropping this actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	bool bExportOnFinishMove = true;

	// Export your parameters for blender and reimport the mesh every frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	bool bExportEveryFrame = false;

	// By default we do a quicker low quality tangent calculation on previewing
	// When using the bake tool, the result will always be high quality.
	//
	// Enable if you still want high quality while previewing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	bool bHighQualityTangents = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Inputs, meta=(ShowOnlyInnerProperties))
	FAlterMeshInputParams InputParams;

	void InitializeParamsFromAsset();

	// Geometry params are UObjects that may spawn components such as splines or other tools
	// and must be properly cleaned/created every time the parameter or asset changes
	void CleanupGeometryParam(FAlterMeshGeometryParam& MeshParam);
	void InitializeGeometryParam(FAlterMeshGeometryParam& GeometryParam);
	void CleanupAllGeometryParams();
	void InitializeAllGeometryParams();

	// Replace the components materials with OverrideMaterials array
	void RefreshMaterials();

	UPROPERTY(BlueprintReadOnly, Category="Converter Settings")
	UObject* ConverterInstance;

	UPROPERTY()
	FOnBecomePreviewDelegate OnBecomePreview;

	void PlaceInstances(UObject* InObject, const TArray<FMatrix44f>& InInstances);
	void CreateComponents(const TSharedPtr<FAlterMeshPrimitive>& Mesh, bool bInstance);

	// Gets the refresh delegate, if blender is closed try to open it, if fails returns null
	FOnRefreshDelegate* GetOnRefreshDelegate();
	
private:
	
	void OnComponentMaterialChanged(const UAlterMeshComponent* Component, int32 ElementIndex, UMaterialInterface* Material);	
	void OnImport(TArray<TSharedPtr<FAlterMeshPrimitive>>);
	void StartInstance();
};

