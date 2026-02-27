// Copyright 2023 Aechmea

#pragma once

#include "AlterMeshSection.h"
#include "AlterMeshAsset.h"
#include "Async/ParallelFor.h"

#include <Extern/AlterMesh.h>

#include "AlterMeshHandle.h"
#pragma comment(lib, "AlterMesh.lib")

DECLARE_LOG_CATEGORY_EXTERN(LogAlterMeshImport, Log, All);

enum class EAlterMeshAttributeIndexing : uint8
{
	Vertex,
	Corner,
};

struct ALTERMESH_API FAlterMeshImport
{
	FAlterMeshImport(const TSharedPtr<FAlterMeshHandle, ESPMode::ThreadSafe>& AlterMeshHandle, TWeakObjectPtr<UAlterMeshAssetInterface> Asset)
		: AlterMeshHandle(AlterMeshHandle), Asset(Asset)
	{
		
	}
public:
	void ImportMeshes(TArray<TSharedPtr<FAlterMeshPrimitive>>& OutMeshes);
	void ImportParams(TArray<FString>& OutObjects, TArray<FString>& OutObjectParams);

	template <typename T1, typename T2>
	static void ContainerSwizzle(uint8 NumComponents, T1& FromContainer, uint8 FromBitmask, T2& ToContainer, uint8 ToBitmask);
	
	TSharedPtr<FAlterMeshPrimitive> ProcessMesh(TArrayView<FVector3f> Vertices, TArrayView<FVector3f> Normals,
								TArrayView<int32> Loops, TArrayView<int32> TriangleCorners,
								TArrayView<int32> CornerToVertexIndex, TArrayView<int32> MaterialIndices, TArrayView<TCHAR> UsedMaterialsJson,
								int64 Hash, TArray<TArrayView<FVector4f>> Attributes, TArray<EAlterMeshAttributeIndexing> AttributeIndexing);

	void CalculateTangents(FAlterMeshSection& Section);

	inline static FMatrix44f ToUEMatrix = FTransform3f(FRotator3f(0,90,0), FVector3f::ZeroVector, FVector3f(-100,100,100)).ToMatrixWithScale();

	template<typename T>
	TArrayView<T> ReadArray()
	{
		void* Address;
		uint64 Length;
		Read(AlterMeshHandle->Get(), &Address, &Length);
		//check(Length % sizeof(T) == 0)
		return TArrayView<T, int32>((T*)Address, Length/sizeof(T));
	}

	template<typename T>
	T ReadValue()
	{
		void* Address;
		uint64 Length;
		Read(AlterMeshHandle->Get(), &Address, &Length);
		//check(Length == sizeof(T))
		T Out{};
		FPlatformMemory::Memcpy(&Out, Address, Length);
		return Out;
	}

private:

	// Handle to the AlterMesh library
	TSharedPtr<FAlterMeshHandle, ESPMode::ThreadSafe> AlterMeshHandle = nullptr;

	// Asset contains data about the object being imported
	TWeakObjectPtr<const UAlterMeshAssetInterface> Asset;

	// Version
	int32 MajorVersion = 0;
	int32 MinorVersion = 0;
	int32 PatchVersion = 0;
};

template <typename T1, typename T2>
void FAlterMeshImport::ContainerSwizzle(uint8 NumComponents, T1& FromContainer, uint8 FromBitmask, T2& ToContainer, uint8 ToBitmask)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < NumComponents; j++)
		{
			if ((FromBitmask >> i) & 1
				&& (ToBitmask >> j) & 1)
			{
				ToContainer.Component(j) = FromContainer.Component(i);
				FromBitmask = FromBitmask ^ (1 << i);
				ToBitmask = ToBitmask ^ (1 << j);
			}
		}
	}
}