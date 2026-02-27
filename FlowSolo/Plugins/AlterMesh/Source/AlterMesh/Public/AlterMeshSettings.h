// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "Engine/DeveloperSettings.h"
#include "AlterMeshSettings.generated.h"

class UAlterMeshAssetInterface;
class UAlterMeshAsset;

UCLASS(BlueprintType, Blueprintable, config = AlterMesh, meta=(DisplayName="AlterMesh"))
class ALTERMESH_API UAlterMeshSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:

	UAlterMeshSettings();

#if WITH_EDITOR
    void OnChangedPath(UObject* Object, FPropertyChangedEvent& Event);
#endif

	virtual FName GetCategoryName() const override { return FName("Plugins"); };

	// Custom colors and visualizers
	UPROPERTY(EditAnywhere, config, Category = "AlterMesh")
	bool bCustomStyle = true;

	// Each AlterMeshActor will spawn a background process
	// and will be automatically closed if stays idle
	UPROPERTY(EditAnywhere, config, Category = "AlterMesh")
	float MaxIdleTime = 15.f;

	// Max amount of memory allowed to be allocated by ALL processes
	// if this amount is reached, older processes will need to finish before new ones spawn/
	// if value is too low, some meshes may not be able to be imported
	// Value in MiB
	UPROPERTY(EditAnywhere, config, Category = "AlterMesh")
	float MaxMemory = 4000.f;

	// Select a path to blender.exe here
	UPROPERTY(EditAnywhere, config, Category = "AlterMesh")
	FFilePath ExecutablePath;

	// When enabled, actors that get converted will automatically be locked and no further edits will be done
	// Enable if you want to avoid accidental changes by another users
	// @see AAlterMeshActor::bLocked 
	UPROPERTY(EditAnywhere, config, Category = "AlterMesh")
	bool bLockOnConvert = false;
};

UCLASS(config = AlterMesh, defaultconfig)
class ALTERMESH_API UAlterMeshEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, config, Category = UnitTests, meta=(AdvancedDisplay))
	TArray<TSoftObjectPtr<UAlterMeshAssetInterface>> UnitTestAssets;
	
	UPROPERTY(EditAnywhere, config, Category = UnitTests, meta=(AdvancedDisplay))
	TArray<TSoftObjectPtr<UObject>> UnitTestHelperAssets;
	
	UPROPERTY(EditAnywhere, config, Category = DefaultAssets, meta=(AdvancedDisplay))
	TSoftObjectPtr<UMaterialInterface> DefaultVATMaterial;
	
	UPROPERTY(EditAnywhere, config, Category = DefaultAssets, meta=(AdvancedDisplay))
	TSoftObjectPtr<UMaterialInterface> MeshPreviewVertexColors;
	
	UPROPERTY(EditAnywhere, config, Category = DefaultAssets, meta=(AdvancedDisplay))
	TSoftObjectPtr<UMaterialInterface> MeshPreviewUVs;
	
	UPROPERTY(EditAnywhere, config, Category = DefaultAssets, meta=(AdvancedDisplay))
	TSoftObjectPtr<UTexture2D> BillboardTexture;
	
	UPROPERTY(EditAnywhere, config, Category = DefaultAssets, meta=(AdvancedDisplay))
	TSoftObjectPtr<UNiagaraScript> ExportToBlueprintScript;
};