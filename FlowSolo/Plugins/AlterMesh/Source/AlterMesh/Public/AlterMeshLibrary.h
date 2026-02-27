// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshActor.h"
#include "AlterMeshLibrary.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ALTERMESH_API UAlterMeshLibrary : public UObject
{
	GENERATED_BODY()
public:

	static FString ConvertFilenameToFull(const FString& InPath);
	static FString ConvertFilenameToRelative(const FString& InPath);

	UFUNCTION(BlueprintCallable, Category="AlterMesh|Animation", meta = (DefaultToSelf=InActor))
	static void SetFrame(AAlterMeshActor* InActor, int32 InFrame, bool bRefresh = false);

	UFUNCTION(BlueprintCallable, Category="AlterMesh|Animation", meta = (DefaultToSelf=InActor))
	static void SetTime(AAlterMeshActor* InActor, float InTime, bool bRefresh = false);

	// Sets a param by name
	UFUNCTION(BlueprintCallable, Category="AlterMesh|Params", meta = (DefaultToSelf=InActor))
	static void SetVectorParam(AAlterMeshActor* InActor, FName ParamName, FVector Value, bool bConvertCoordinateSpace, bool bRefresh = false);

	// Sets a param by name
	UFUNCTION(BlueprintCallable, Category="AlterMesh|Params", meta = (DefaultToSelf=InActor))
	static void SetFloatParam(AAlterMeshActor* InActor, FName ParamName, float Value, bool bRefresh = false);

	// Sets a param by name
	UFUNCTION(BlueprintCallable, Category="AlterMesh|Params", meta = (DefaultToSelf=InActor))
	static void SetIntParam(AAlterMeshActor* InActor, FName ParamName, int32 Value, bool bRefresh = false);

	// Sets a param by name
	UFUNCTION(BlueprintCallable, Category="AlterMesh|Params", meta = (DefaultToSelf=InActor))
	static void SetBoolParam(AAlterMeshActor* InActor, FName ParamName, bool Value, bool bRefresh = false);

	// Sets a param by name
	UFUNCTION(BlueprintCallable, Category="AlterMesh|Params", meta = (DefaultToSelf=InActor))
	static void SetAssetParam(AAlterMeshActor* InActor, FName ParamName, UObject* Value, bool bRefresh = false);

	// Gets a param by name
	UFUNCTION(BlueprintCallable, Category="AlterMesh|Params", meta = (DefaultToSelf=InActor))
	static UAlterMeshGeometryBase* GetGeometryParam(AAlterMeshActor* InActor, FString ParamName);

	// Gets a param by Id
	UFUNCTION(BlueprintCallable, Category="AlterMesh|Params", meta = (DefaultToSelf=InActor))
	static UAlterMeshGeometryBase* GetGeometryParamById(AAlterMeshActor* InActor, FAlterMeshParamId ParamId);

	// Set a material on all components that match the slot name
	UFUNCTION(BlueprintCallable, Category="AlterMesh|Materials", meta = (DefaultToSelf=InActor))
	static void SetMaterial(AAlterMeshActor* InActor, FName InSlotName, UMaterialInterface* InMaterial);

	// Get material from all components that match the slot name
	// AlterMesh will create one component from each imported mesh
	// loop this output if you want to set a parameter for each one
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AlterMesh|Materials", meta = (DefaultToSelf=InActor))
	static void GetMaterials(AAlterMeshActor* InActor, FName InSlotName, TArray<UMaterialInterface*>& OutMaterials);

	// Get all materials from all components
	// there can be duplicates if meshes have repeating materials
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AlterMesh|Materials", meta = (DefaultToSelf=InActor))
	static void GetAllMaterials(AAlterMeshActor* InActor, TArray<FName>& OutMaterialNames, TArray<UMaterialInterface*>& OutMaterials);

	// Create a DMI for all components's materials that match slot name
	UFUNCTION(BlueprintCallable, Category="AlterMesh|Materials", meta = (DefaultToSelf=InActor))
	static void CreateDynamicMaterialInstance(AAlterMeshActor* InActor, FName InSlotName, UMaterialInterface* InMaterial);
};
