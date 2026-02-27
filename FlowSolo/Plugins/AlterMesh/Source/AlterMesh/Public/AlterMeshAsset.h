// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshParam.h"
#include "UObject/Object.h"
#include "AlterMeshAsset.generated.h"

class UAlterMeshConverterBase;
class UMaterialInterface;

UENUM()
enum class EAlterMeshCoordinateSpace : uint8
{
	Local,
	World
};

UENUM(BlueprintType)
enum class EAlterMeshAttribute : uint8
{
	UV0,
	UV1,
	UV2,
	UV3,
	Color
};

UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EAlterMeshAttributeMask : uint8
{
	None = 0 UMETA(Hidden),
	R = 1 << 0,
	G = 1 << 1,
	B = 1 << 2,
	A = 1 << 3,
	All = R | G | B | A UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FAlterMeshAttributeMapping
{
	GENERATED_BODY()
public:
	// Attribute name inside blender
	UPROPERTY(EditDefaultsOnly, Category="")
	FName From;

	// In case its a Vector, which channel it will grab from
	UPROPERTY(EditDefaultsOnly, meta = (Bitmask, BitmaskEnum = "/Script/AlterMesh.EAlterMeshAttributeMask"), Category="")
	uint8 FromChannel = (uint8)EAlterMeshAttributeMask::All;

	// Which mesh attribute in unreal to write to
	UPROPERTY(EditDefaultsOnly, Category="")
	EAlterMeshAttribute To;

	// Which channel to write to
	UPROPERTY(EditDefaultsOnly, meta = (Bitmask, BitmaskEnum = "/Script/AlterMesh.EAlterMeshAttributeMask"), Category="")
	uint8 ToChannel = (uint8)EAlterMeshAttributeMask::All;	
};

UCLASS(BlueprintType, AutoExpandCategories=("Inputs"))
class ALTERMESH_API UAlterMeshAssetInterface : public UObject
{
	GENERATED_BODY()
public:
	virtual UAlterMeshAsset* Get() { return nullptr; };
	virtual const UAlterMeshAsset* Get() const { return nullptr; };
	virtual FAlterMeshInputParams& GetParams() { return EmptyParams; };

private:

	FAlterMeshInputParams EmptyParams;	
};

UCLASS(BlueprintType, AutoExpandCategories=("Config", "Inputs", "Params"))
class ALTERMESH_API UAlterMeshAsset : public UAlterMeshAssetInterface
{
	GENERATED_BODY()
	public:

	virtual UAlterMeshAsset* Get() override { return this; };
	virtual const UAlterMeshAsset* Get() const override { return this; };
	virtual FAlterMeshInputParams& GetParams() override { return InputParams; };
	
	UAlterMeshAsset();

	// Path to your file, relative to your project 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Config)
	FFilePath Filename;

	// Object that this asset will try to import
	// Only objects with GN modifiers will work
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Config)
	FString ObjectName;

	// If a bool parameter is found with this name, it will use as a replacement for the collision mesh
	// Otherwise collision will be created from hull decomposition
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Config)
	FString CollisionSwitchParameter = FString("Proxy");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Config)
	EAlterMeshCoordinateSpace CoordinateSpace = EAlterMeshCoordinateSpace::Local;

#if WITH_EDITORONLY_DATA
	// Whether this asset will convert into a mesh, a VAT, etc...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Config, meta=(ShowDisplayNames, AllowedClasses="/Script/AlterMeshEditor.AlterMeshConverterBase"))
	TSoftClassPtr<UObject> ConverterClass;
#endif

	// Maps Out Attributes (blender) to Mesh attributes (Unreal)
	// For example, you may map an Out Float attribute named "Wetness" to Color.R and use on a material to drive roughness
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Attributes)
	TArray<FAlterMeshAttributeMapping> AttributeMapping;

	// Materials slots that were found in the .blend file
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Materials)
	TMap<FName, UMaterialInterface*> Materials;

	void CreateParamsFromJson(FString Json);

	UPROPERTY(Transient)
	bool bReimportOnlyParams;
	
private:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Inputs, meta=(ShowOnlyInnerProperties, AllowPrivateAccess = "true"))
	FAlterMeshInputParams InputParams;

};

UCLASS(BlueprintType, AutoExpandCategories=("Inputs"))
class ALTERMESH_API UAlterMeshAssetInstance : public UAlterMeshAssetInterface
{
	GENERATED_BODY()
	
public:

	static UAlterMeshAssetInstance* CreateInstance(UAlterMeshAsset* Parent, const FAlterMeshInputParams& Params);
	
	virtual UAlterMeshAsset* Get() override { return Parent; };
	virtual const UAlterMeshAsset* Get() const override { return Parent; };
	virtual FAlterMeshInputParams& GetParams() override { return OverrideParams; };
	
	UPROPERTY(VisibleAnywhere, Category="")
	UAlterMeshAsset* Parent;
	
private:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Inputs, meta=(ShowOnlyInnerProperties, AllowPrivateAccess = "true"))
	FAlterMeshInputParams OverrideParams;
};
