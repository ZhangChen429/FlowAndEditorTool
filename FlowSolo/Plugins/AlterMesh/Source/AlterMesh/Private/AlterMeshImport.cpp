// Copyright 2023 Aechmea

#include "AlterMeshImport.h"
#include "AlterMeshAsset.h"
#include "MaterialShared.h"
#include "Algo/MaxElement.h"
#include "AlterMesh/AlterMesh.h"
#include "Async/Async.h"
#include "Serialization/JsonReader.h"
#include "Async/ParallelFor.h"
#include "Materials/Material.h"
#include "Serialization/JsonSerializer.h"

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2
#include "MaterialDomain.h"
#endif

#if WITH_EDITOR
#include "MeshUtilities.h"
#endif

using TUniqueVert = TTuple<FVector3f, FVector3f, FLinearColor, FVector2f, FVector2f, FVector2f, FVector2f>;

DEFINE_LOG_CATEGORY(LogAlterMeshImport)

void FAlterMeshImport::ImportMeshes(TArray<TSharedPtr<FAlterMeshPrimitive>>& OutMeshes)
{
	if (AlterMeshHandle.IsValid() && ReadLock(AlterMeshHandle->Get()))
	{
		const double StartTime = FPlatformTime::Seconds();

		int32 NumMeshes = ReadValue<int32>();
		int32 NumInstances = ReadValue<int32>();

		if (NumMeshes < 1)
		{
			ReadUnlock(AlterMeshHandle->Get());
			return;
		}

		// Import meshes
		TArray<TFuture<TSharedPtr<FAlterMeshPrimitive>>> FutureMeshes;
		for (int32 MeshIndex = 0; MeshIndex < NumMeshes; MeshIndex++)
		{
	 		TArrayView<FVector3f> Vertices = ReadArray<FVector3f>();
			TArrayView<FVector3f> Normals = ReadArray<FVector3f>();
			TArrayView<int32> Loops = ReadArray<int32>();
			TArrayView<int32> TriangleCorners = ReadArray<int32>();
			TArrayView<int32> CornerToVertexIndex = ReadArray<int32>();
			TArrayView<int32> MaterialIndices = ReadArray<int32>();
			TArrayView<TCHAR> UsedMaterialsJson = ReadArray<TCHAR>();
			int64 Hash = ReadValue<int64>();
	 		TArrayView<int32> BlenderVersion = ReadArray<int32>();

			MajorVersion = BlenderVersion[0];
			MinorVersion = BlenderVersion[1];
			PatchVersion = BlenderVersion[2];
			
			TArray<TArrayView<FVector4f>> Attributes;
			for (const FAlterMeshAttributeMapping& AttributeMapping : Asset->Get()->AttributeMapping)
			{
				TArrayView<FVector4f> Attribute = ReadArray<FVector4f>();		
				Attributes.Add(Attribute);
			}

			TArrayView<bool> Indexing = ReadArray<bool>();
			TArray<EAlterMeshAttributeIndexing> AttributeIndexing;
			for (bool bVertexIndexed : Indexing)
			{
				AttributeIndexing.Add(bVertexIndexed ? EAlterMeshAttributeIndexing::Vertex : EAlterMeshAttributeIndexing::Corner);				
			}
			
			if (Vertices.Num())
			{
				FutureMeshes.Add(
					Async(
						EAsyncExecution::ThreadPool,
						[this, Vertices, Normals, Loops, TriangleCorners, CornerToVertexIndex, MaterialIndices, UsedMaterialsJson, Hash, Attributes, AttributeIndexing]()
						{
							return ProcessMesh(Vertices, Normals, Loops, TriangleCorners, CornerToVertexIndex, MaterialIndices, UsedMaterialsJson, Hash, Attributes, AttributeIndexing);
						}
					)
				);
			}
		}

		for (const auto& FutureMesh : FutureMeshes)
		{
			OutMeshes.Add(FutureMesh.Get());
		}

		// Import Instances
		for (int32 InstanceIndex = 0; InstanceIndex < NumInstances; InstanceIndex++)
		{
			uint64 PathId = ReadValue<uint64>();
			FName AssetPath = *reinterpret_cast<FName*>(&PathId);

			AssetPath = FName(AssetPath.GetComparisonIndex(), AssetPath.GetComparisonIndex(), AssetPath.GetNumber());
			
			FMatrix44f LocalMatrix = ReadValue<FMatrix44f>();
			FMatrix44f WorldMatrix = ReadValue<FMatrix44f>();

			FMatrix44f InstanceMatrix;

			// Don't use matrix directly, needs to convert to UE
			// Decompose to create new matrix
			FTransform3f Transform(Asset->Get()->CoordinateSpace == EAlterMeshCoordinateSpace::Local ? LocalMatrix.GetTransposed() : WorldMatrix.GetTransposed());

			FVector3f Location = ToUEMatrix.TransformVector(Transform.GetLocation());
			Transform.SetLocation(Location);

			FQuat4f Rotation = Transform.GetRotation();
			Rotation = FQuat4f(Rotation.Y, Rotation.X, -Rotation.Z, Rotation.W);
			Transform.SetRotation(Rotation);

			FVector3f Scale = Transform.GetScale3D();
			Scale = FVector3f(Scale.Y, Scale.X, Scale.Z);
			Transform.SetScale3D(Scale);

			InstanceMatrix = Transform.ToMatrixWithScale();

			int64 Hash = ReadValue<int64>();

			for (TSharedPtr<FAlterMeshPrimitive>& Mesh : OutMeshes)
			{
				if (Mesh->Hash == Hash)
				{
					if (AssetPath != NAME_None)
					{
						Mesh->AssetPath = AssetPath;
						Mesh->bIsInstance = true;
					}
					
					for (FAlterMeshSection& Section : Mesh->Sections)
					{
						Section.Instances.Add(InstanceMatrix);
					}
				}
			}
		}

		// Setup materials
		for (TSharedPtr<FAlterMeshPrimitive>& Mesh : OutMeshes)
		{
			for (FAlterMeshSection& Section : Mesh->Sections)
			{
				UMaterialInterface* const* ParamMaterial = Asset->Get()->Materials.Find(Section.MaterialName);
				if (ParamMaterial && *ParamMaterial)
				{
					Section.Material = *ParamMaterial;
				}
				else
				{
					Section.Material = UMaterial::GetDefaultMaterial(MD_Surface);
				}
			}
		}

		if (CVarAlterMeshDebugProcessOutput.GetValueOnAnyThread())
		{
			UE_LOG( LogAlterMeshImport, Log, TEXT( "Imported in %.4f sec(s)" ), FPlatformTime::Seconds() - StartTime);
		}

		ReadUnlock(AlterMeshHandle->Get());
	}

}

void FAlterMeshImport::ImportParams(TArray<FString>& OutObjects, TArray<FString>& OutObjectParams)
{
	int32 NumObjects = ReadValue<int32>();

	for (int32 i = 0; i < NumObjects; i++)
	{
		TArrayView<TCHAR> ObjectNameCharArray = ReadArray<TCHAR>();
		OutObjects.Add(FString(ObjectNameCharArray));

		TArrayView<TCHAR> ParamCharArray = ReadArray<TCHAR>();
		OutObjectParams.Add(FString(ParamCharArray));
	}
}

void FAlterMeshImport::CalculateTangents(FAlterMeshSection& Section)
{
	// todo is this even correct
	// Low quality tangents
	Section.Tangents.AddZeroed(Section.Vertices.Num());
	Section.Bitangents.AddZeroed(Section.Vertices.Num());

	ParallelFor( Section.Indices.Num() / 3, [&Section] (int32 i)
	{
		const FVector3f V0 = Section.Vertices[Section.Indices[i+0]];
		const FVector3f V1 = Section.Vertices[Section.Indices[i+1]];
		const FVector3f V2 = Section.Vertices[Section.Indices[i+2]];

		const FVector2f UV0 = Section.UV0[Section.Indices[i+0]];
		const FVector2f UV1 = Section.UV0[Section.Indices[i+1]];
		const FVector2f UV2 = Section.UV0[Section.Indices[i+2]];

		const FVector3f DeltaPos1 = V1-V0;
		const FVector3f DeltaPos2 = V2-V0;

		const FVector2f DeltaUV1 = UV1-UV0;
		const FVector2f DeltaUV2 = UV2-UV0;

		float Denominator = DeltaUV1.X * DeltaUV2.Y - DeltaUV1.Y * DeltaUV2.X;
		
		float r = 0.f;
		if (!FMath::IsNearlyZero(Denominator))
		{
			r = 1.0f / Denominator ;
		}
		
		const FVector3f Tangent = (DeltaPos1 * DeltaUV2.Y - DeltaPos2 * DeltaUV1.Y)*r;
		const FVector3f Bitangent = (DeltaPos2 * DeltaUV1.X - DeltaPos1 * DeltaUV2.X)*r;

		Section.Tangents[Section.Indices[i]] = Tangent;
		Section.Bitangents[Section.Indices[i]] = Bitangent;
		Section.Tangents[Section.Indices[i+1]] = Tangent;
		Section.Bitangents[Section.Indices[i+1]] = Bitangent;
		Section.Tangents[Section.Indices[i+2]] = Tangent;
		Section.Bitangents[Section.Indices[i+2]] = Bitangent;
	});

	return;
}

TSharedPtr<FAlterMeshPrimitive> FAlterMeshImport::ProcessMesh(TArrayView<FVector3f> Vertices, TArrayView<FVector3f> Normals,
								TArrayView<int32> Loops, TArrayView<int32> TriangleCorners,
								TArrayView<int32> CornerToVertexIndex, TArrayView<int32> MaterialIndices, TArrayView<TCHAR> UsedMaterialsJson,
								int64 Hash, TArray<TArrayView<FVector4f>> Attributes, TArray<EAlterMeshAttributeIndexing> AttributeIndexing)
{
	TSharedPtr<FJsonObject> JsonParams = MakeShareable(new FJsonObject);
	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FString(UsedMaterialsJson));
	check(FJsonSerializer::Deserialize(JsonReader, JsonParams))

	TSharedPtr<FAlterMeshPrimitive> OutMesh = MakeShareable(new FAlterMeshPrimitive);
	OutMesh->Hash = Hash;

	// Convert scene
	ParallelFor(Vertices.Num(), [&](int32 i)
	{
		Vertices[i] = ToUEMatrix.TransformPosition(Vertices[i]);
	});

	ParallelFor(Normals.Num(), [&](int32 i)
	{
		Normals[i] = ToUEMatrix.TransformVector(Normals[i]);
		Normals[i].Normalize();
	});
	
	TArray<FVector3f> SplitVertices;
	SplitVertices.Reserve(Vertices.Num());
	TArray<FVector3f> SplitNormals;
	SplitNormals.Reserve(Vertices.Num());
	TArray<FVector2f> SplitUV0;
	SplitUV0.Reserve(Vertices.Num());
	TArray<FVector2f> SplitUV1;
	SplitUV1.Reserve(Vertices.Num());
	TArray<FVector2f> SplitUV2;
	SplitUV2.Reserve(Vertices.Num());
	TArray<FVector2f> SplitUV3;
	SplitUV3.Reserve(Vertices.Num());
	TArray<FColor> SplitColors;
	SplitColors.Reserve(Vertices.Num());

	TMultiMap<TUniqueVert, int32> UniqueVertices;
	UniqueVertices.Reserve(Vertices.Num());
	TArray<uint32> HashValues;
	HashValues.AddUninitialized(Loops.Num());

	{
		TArray<FLinearColor> Colors;
		Colors.AddZeroed(Loops.Num());	
		TArray<FVector2f> UV0;
		UV0.AddZeroed(Loops.Num());	
		TArray<FVector2f> UV1;
		UV1.AddZeroed(Loops.Num());
		TArray<FVector2f> UV2;
		UV2.AddZeroed(Loops.Num());
		TArray<FVector2f> UV3;
		UV3.AddZeroed(Loops.Num());

		TArray<TArray<FVector2f>*> UVSets{&UV0, &UV1, &UV2, &UV3};

		ParallelFor(Loops.Num(), [&](const int32 i)
		{
			int32 j = 0;
			for (const FAlterMeshAttributeMapping& Mapping : Asset->Get()->AttributeMapping)
			{
				if (Mapping.To == EAlterMeshAttribute::Color)
				{
					FVector4f& Attribute = AttributeIndexing[j] == EAlterMeshAttributeIndexing::Vertex ? Attributes[j][Loops[i]] : Attributes[j][TriangleCorners[i]];		
					ContainerSwizzle(4, Attribute, Mapping.FromChannel, Colors[i], Mapping.ToChannel);
				}
				else
				{
					FVector4f& Attribute = AttributeIndexing[j] == EAlterMeshAttributeIndexing::Vertex ? Attributes[j][Loops[i]] : Attributes[j][TriangleCorners[i]];
					ContainerSwizzle(2, Attribute, Mapping.FromChannel, (*UVSets[(uint8)Mapping.To])[i], Mapping.ToChannel);				
				}
			
				j++;
				
			}
		});
	
		TArray<TUniqueVert> HashedItems;
		HashedItems.AddUninitialized(Loops.Num());		
		
		bool bCornerIndexedNormals = MajorVersion >= 4 && MinorVersion >= 1;
		
		// Compute the hash values
		ParallelFor(Loops.Num(), [&](const int32 i)
		{
			const TUniqueVert Vert = TUniqueVert(
			Vertices[Loops[i]],
			Normals[bCornerIndexedNormals ? TriangleCorners[i] : i],
			Colors[i],
			UV0[i],
			UV1[i],
			UV2[i],
			UV3[i]);

			HashedItems[i] = Vert;
			HashValues[i] = GetTypeHash(Vert);
		});

		// Split verts
		for (int32 LoopIndex = 0; LoopIndex < Loops.Num(); LoopIndex++)
		{			
			if (const int32* SplitVertLoopIndex = UniqueVertices.FindByHash(HashValues[LoopIndex], HashedItems[LoopIndex]))
			{
				Loops[LoopIndex] = Loops[*SplitVertLoopIndex];
				CornerToVertexIndex[TriangleCorners[LoopIndex]] = CornerToVertexIndex[TriangleCorners[*SplitVertLoopIndex]];
			}
			else
			{
				const int32 NewVertIndex = SplitVertices.Add(Vertices[Loops[LoopIndex]]);

				CornerToVertexIndex[TriangleCorners[LoopIndex]] = NewVertIndex;
				Loops[LoopIndex] = NewVertIndex;
				SplitNormals.Add(bCornerIndexedNormals ? Normals[TriangleCorners[LoopIndex]] : Normals[LoopIndex]);
				SplitColors.Add(Colors[LoopIndex].ToFColor(false));
				SplitUV0.Add(UV0[LoopIndex]);
				SplitUV1.Add(UV1[LoopIndex]);
				SplitUV2.Add(UV2[LoopIndex]);
				SplitUV3.Add(UV3[LoopIndex]);
				
				UniqueVertices.AddByHash(HashValues[LoopIndex], HashedItems[LoopIndex], LoopIndex);
			}
		}
	}
	
	const int32 NumMaterials = MaterialIndices.Num() ?  *Algo::MaxElement(MaterialIndices) + 1 : 0;

	for (int32 MaterialIndex = 0; MaterialIndex < NumMaterials; MaterialIndex++)
	{
		FAlterMeshSection MeshSection;
		MeshSection.Reserve(SplitVertices.Num());
		MeshSection.Vertices = SplitVertices;

		// Always have at least one UV map
		MeshSection.UV0 = SplitUV0;
		
		if (Asset->Get()->AttributeMapping.ContainsByPredicate([](const FAlterMeshAttributeMapping& Item)
		{
			return Item.To == EAlterMeshAttribute::UV1;
		}))
		{
			MeshSection.UV1 = SplitUV1;
		}
		
		if (Asset->Get()->AttributeMapping.ContainsByPredicate([](const FAlterMeshAttributeMapping& Item)
		{
			return Item.To == EAlterMeshAttribute::UV2;
		}))
		{
			MeshSection.UV2 = SplitUV2;
		}
		
		if (Asset->Get()->AttributeMapping.ContainsByPredicate([](const FAlterMeshAttributeMapping& Item)
		{
			return Item.To == EAlterMeshAttribute::UV3;
		}))
		{
			MeshSection.UV3 = SplitUV3;
		}
		
		if (Asset->Get()->AttributeMapping.ContainsByPredicate([](const FAlterMeshAttributeMapping& Item)
		{
			return Item.To == EAlterMeshAttribute::Color;
		}))
		{
			MeshSection.Colors = SplitColors;
		}
		
		MeshSection.Normals = SplitNormals;

		TArray<uint32> SectionIndices;
		SectionIndices.Reserve(Loops.Num());

		for (int32 i = 0; i < Loops.Num(); i+=3)
		{
			const int32 FaceIndex = i/3;
			if (MaterialIndices[FaceIndex] == MaterialIndex)
			{
				SectionIndices.Add(Loops[i]);
				SectionIndices.Add(Loops[i+1]);
				SectionIndices.Add(Loops[i+2]);
			}
		}

		MeshSection.Indices = SectionIndices;

		if (MeshSection.Indices.Num())
		{
			FName MaterialName = FName("Slot_" + FString::FromInt(MaterialIndex));

			if (JsonParams->GetArrayField(TEXT("Materials")).IsValidIndex(MaterialIndex))
			{
				TSharedPtr<FJsonValue> ImportedMaterialName = JsonParams->GetArrayField(TEXT("Materials"))[MaterialIndex];
				if (ImportedMaterialName.IsValid())
				{
					MaterialName = FName(*ImportedMaterialName->AsString());
				}
			}

			MeshSection.MaterialName = MaterialName;
		}

		if (MeshSection.Indices.Num())
		{
			CalculateTangents(MeshSection);
			MeshSection.Hash = Hash;
			OutMesh->Sections.Add(MoveTemp(MeshSection));
		}
	}

	return OutMesh;
}
