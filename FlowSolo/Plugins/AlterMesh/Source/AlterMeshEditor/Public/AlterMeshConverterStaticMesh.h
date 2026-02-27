// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshConverter.h"
#include "AlterMeshSection.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "AlterMeshConverterStaticMesh.generated.h"

class UAlterMeshComponent;
UENUM()
enum class EAlterMeshConverterAction : uint8
{
	CreateNewMesh,
	UseExistingMesh,
	DoNotImport
};

USTRUCT()
struct FAlterMeshConverterStep
{
	GENERATED_BODY()
public:

	EAlterMeshConverterAction Action = EAlterMeshConverterAction::CreateNewMesh;

	// Asset that this action will use, in case of CreateNewMesh, this will be replaced with generated mesh
	TSoftObjectPtr<UObject> AssetToUse;

	// Only valid if Action == UseExistingMesh
	uint32 Checksum;
};

USTRUCT()
struct FHullDecomposeSettings
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, Category="")
	int32 NumHulls = 8;
	
	UPROPERTY(EditAnywhere, Category="")
	int32 MaxHullVertices = 12;
	
	UPROPERTY(EditAnywhere, Category="")
	int32 Resolution = 10000;
};

UCLASS(BlueprintType, meta=(DisplayName="Convert to StaticMesh"))
class UAlterMeshConverterStaticMesh : public UAlterMeshConverterBase
{
	GENERATED_BODY()
	
public:
	UAlterMeshConverterStaticMesh();

	FName ChecksumTagName = FName("AlterMeshStaticMeshConverterChecksum");

	// Whether to combine everything in a single mesh or as many objects as the GN outputs
	// Ignored on foliage, make sure you output one object per foliage on the GN
	UPROPERTY(EditAnywhere, Category="")
	bool bCombineMeshes;

	// Merge every slot with same material into one
	// If enabled, remember to apply different materials to the slots,
	// otherwise everything is gonna merge into a single material slot
	UPROPERTY(EditAnywhere, Category="")
	bool bCombineMaterials;

	// Build collision, either from the Collision Param name or a simple hull decomposition if not specified/found
	UPROPERTY(EditAnywhere, Category="")
	bool bBuildCollision;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bBuildCollision"), Category="")
	FHullDecomposeSettings BuildCollisionSettings;
	
	UPROPERTY(EditAnywhere, Category="")
	FMeshBuildSettings BuildSettings;

	UPROPERTY(EditAnywhere, Category="")
	FMeshNaniteSettings NaniteSettings;
	
	// Class to use for spawned components
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="")
	TSubclassOf<UInstancedStaticMeshComponent> ComponentClass = UHierarchicalInstancedStaticMeshComponent::StaticClass();
	
	// Components that were created from the GN
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, AdvancedDisplay, Category="")
	TArray<UActorComponent*> SpawnedComponents;
	
	// Actors that were created from the GN
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, AdvancedDisplay, Category="")
	TArray<AActor*> SpawnedActors;
	
	UPROPERTY()
	TArray<FAlterMeshConverterStep> ConverterSteps;

	void BuildMeshDescription(const TArray<FAlterMeshSection>& Sections, FMeshDescription& OutMeshDescription);

	TWeakObjectPtr<AAlterMeshActor> WeakActor;
	virtual void Initialize(AAlterMeshActor* InActor);

	/* ~Begin UAlterMeshConverterBase interface */	
	virtual void Convert(AAlterMeshActor* InActor) override;
	/* ~End UAlterMeshConverterBase interface */
	
	virtual void CreateAssets(AAlterMeshActor* InActor);	
	virtual void PlaceAssets(AAlterMeshActor* InActor);

	uint32 CalculateMeshHash(const TArray<FAlterMeshSection>& Sections) const;

	virtual void CleanUp() override;
};