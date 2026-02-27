// Copyright 2023 Aechmea

#include "AlterMeshSection.h"

class UAlterMeshConverterBase;

void FAlterMeshSection::Reserve(int32 Num)
{
	Vertices.Reserve(Num);
	Normals.Reserve(Num);
	Colors.Reserve(Num);
	UV0.Reserve(Num);
	UV1.Reserve(Num);
	UV2.Reserve(Num);
	UV3.Reserve(Num);
	Tangents.Reserve(Num);
	Bitangents.Reserve(Num);
	Indices.Reserve(Num*3);
}

FAlterMeshSection& FAlterMeshSection::operator+=(const FAlterMeshSection& Right)
{
	checkf(!Material || Material == Right.Material, TEXT("Cannot merge sections with different materials"));
	Material = Right.Material;

	Reserve(Vertices.Max() + Right.Vertices.Num() * Right.Instances.Num());
	 
	for (const FMatrix44f& Instance : Right.Instances)
	{
		int32 IndexOffset = Vertices.Num();

		// Convert instances to real geo
		TArray<FVector3f> NewVertices = Right.Vertices;

		for (FVector3f& Vertex : NewVertices)
		{
			Vertex = Instance.TransformPosition(Vertex);
		}

		Vertices += NewVertices;

		TArray<FVector3f> NewNormals = Right.Normals;

		for (FVector3f& Normal : NewNormals)
		{
			Normal = Instance.TransformVector(Normal);
			Normal.Normalize();
		}

		Normals += NewNormals;

		Colors += Right.Colors;
		UV0 += Right.UV0;
		UV1 += Right.UV1;
		UV2 += Right.UV2;
		UV3 += Right.UV3;
		Tangents += Right.Tangents;
		Bitangents += Right.Bitangents;

		TArray<uint32> NewIndices = Right.Indices;

		for (uint32& Index : NewIndices)
		{
			Index += IndexOffset;
		}

		Indices += NewIndices;
	}

	return *this;
}

FAlterMeshSection::FAlterMeshSection(const FAlterMeshSection& Other)
{
	MaterialName = Other.MaterialName;
	Material = Other.Material;
	Hash = Other.Hash;

	Material = Other.Material;
	Vertices = Other.Vertices;
	Normals = Other.Normals;
	Colors = Other.Colors;
	UV0 = Other.UV0;
	UV1 = Other.UV1;
	UV2 = Other.UV2;
	UV3 = Other.UV3;
	Tangents = Other.Tangents;
	Bitangents = Other.Bitangents;
	Indices = Other.Indices;
	Instances = Other.Instances;
}

FAlterMeshSection::FAlterMeshSection(FAlterMeshSection&& Other)
{
	MaterialName = Other.MaterialName;
	Material = Other.Material;
	Hash = Other.Hash;

	Vertices = MoveTemp(Other.Vertices);
	Normals = MoveTemp(Other.Normals);
	Colors = MoveTemp(Other.Colors);
	UV0 = MoveTemp(Other.UV0);
	UV1 = MoveTemp(Other.UV1);
	UV2 = MoveTemp(Other.UV2);
	UV3 = MoveTemp(Other.UV3);
	Tangents = MoveTemp(Other.Tangents);
	Bitangents = MoveTemp(Other.Bitangents);
	Indices = MoveTemp(Other.Indices);
	Instances = MoveTemp(Other.Instances);
}

FAlterMeshSection FAlterMeshSection::Copy()
{
	return *this;
}

FAlterMeshPrimitive::FAlterMeshPrimitive(FAlterMeshPrimitive&& Other)
{
	Sections = MoveTemp(Other.Sections);
}

const FAlterMeshPrimitive& FAlterMeshPrimitive::operator=(FAlterMeshPrimitive&& Other)
{
	Sections = MoveTemp(Other.Sections);
	return *this;
}
