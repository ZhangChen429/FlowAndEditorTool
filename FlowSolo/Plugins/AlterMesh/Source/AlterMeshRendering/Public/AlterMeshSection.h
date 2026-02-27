// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;

struct ALTERMESHRENDERING_API FAlterMeshSection
{
	void Reserve(int32 Num);

	FAlterMeshSection() = default;

	// Merge a whole section to another one, takes care of indexing
	FAlterMeshSection& operator+=(const FAlterMeshSection& Right);
	FAlterMeshSection(FAlterMeshSection&& Other);

	// Explicit copy
	FAlterMeshSection Copy();

	TArray<FVector3f> Vertices;
	TArray<FVector3f> Normals;
	TArray<FVector3f> Tangents;
	TArray<FVector3f> Bitangents;
	TArray<uint32> Indices;
	TArray<FVector2f> UV0;
	TArray<FVector2f> UV1;
	TArray<FVector2f> UV2;
	TArray<FVector2f> UV3;
	TArray<FColor> Colors;
	FName MaterialName;
	UMaterialInterface* Material = nullptr;
	int64 Hash;

	// Instance transforms
	TArray<FMatrix44f> Instances;

private:

	// Don't copy meshes around, use MoveTemp
	FAlterMeshSection(const FAlterMeshSection& Other) ;
	FAlterMeshSection& operator=(const FAlterMeshSection& Other) = delete;
};

struct ALTERMESHRENDERING_API FAlterMeshPrimitive
{
	FAlterMeshPrimitive() = default;

	const FAlterMeshPrimitive& operator=(FAlterMeshPrimitive&& Other);
	FAlterMeshPrimitive(FAlterMeshPrimitive&& Other);

	// Is an instance inside of blender, doesn't necessarily means theres an equivalent asset in unreal 
	bool bIsInstance = false;

	// If its an instance, this maps to the original asset in unreal
	FName AssetPath;

	int64 Hash;

	TArray<FAlterMeshSection> Sections;

private:

	// Don't copy meshes around, use MoveTemp
	const FAlterMeshPrimitive& operator=(const FAlterMeshPrimitive& Other) = delete;
	FAlterMeshPrimitive(const FAlterMeshPrimitive& Other) = delete;
};
