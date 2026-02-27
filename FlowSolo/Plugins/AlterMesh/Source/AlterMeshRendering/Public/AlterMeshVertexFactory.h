// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Rendering/StaticMeshVertexDataInterface.h"
#include "AlterMeshComponent.h"
#include "LocalVertexFactory.h"
#include "Containers/DynamicRHIResourceArray.h"
#include "Containers/ResourceArray.h"

class UAlterMeshComponent;

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FAlterMeshVertexFactoryUniformShaderParameters, )
	SHADER_PARAMETER_SRV(Buffer<float4>, VertexFetch_InstanceOriginBuffer)
	SHADER_PARAMETER_SRV(Buffer<float4>, VertexFetch_InstanceTransformBuffer)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

#define REQUIRES_COMMANDLIST ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3

struct FAlterMeshDataType
{
	/** The stream to read the mesh transform from. */
	FVertexStreamComponent InstanceOriginComponent;

	/** The stream to read the mesh transform from. */
	FVertexStreamComponent InstanceTransformComponent[3];

	FRHIShaderResourceView* InstanceOriginSRV = nullptr;
	FRHIShaderResourceView* InstanceTransformSRV = nullptr;
};
class FAlterMeshInstanceData;
/** A vertex buffer of positions. */
class FAlterMeshInstanceBuffer : public FRenderResource
{
public:

	/** Default constructor. */
	FAlterMeshInstanceBuffer();

	/** Destructor. */
	~FAlterMeshInstanceBuffer();

	/**
	 * Initializes the buffer with the component's data.
	 * @param Other - instance data, this call assumes the memory, so this will be empty after the call
	 */
	ALTERMESHRENDERING_API void InitFromPreallocatedData(FAlterMeshInstanceData& Other);

	/**
	 * Specialized assignment operator, only used when importing LOD's.
	 */
	void operator=(const FAlterMeshInstanceBuffer &Other);

	FORCEINLINE FAlterMeshInstanceData* GetInstanceData() const
	{
		return InstanceData.Get();
	}

	// FRenderResource interface.
#if REQUIRES_COMMANDLIST
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
	virtual void InitResource(FRHICommandListBase& RHICmdList) override;
#else
	virtual void InitRHI() override;
	virtual void InitResource() override;
#endif
	
	virtual void ReleaseRHI() override;
	virtual void ReleaseResource() override;
	virtual FString GetFriendlyName() const override { return TEXT("Static-mesh instances"); }
	SIZE_T GetResourceSize() const;

	void BindInstanceVertexBuffer(const class FVertexFactory* VertexFactory, struct FAlterMeshDataType& InstancedStaticMeshData) const;

public:
	/** The vertex data storage type */
	TSharedPtr<FAlterMeshInstanceData, ESPMode::ThreadSafe> InstanceData;

	/** Keep CPU copy of instance data*/
	bool RequireCPUAccess;

	FBufferRHIRef GetInstanceOriginBuffer()
	{
		return InstanceOriginBuffer.VertexBufferRHI;
	}

	FBufferRHIRef GetInstanceTransformBuffer()
	{
		return InstanceTransformBuffer.VertexBufferRHI;
	}

private:
	class FInstanceOriginBuffer : public FVertexBuffer
	{
		virtual FString GetFriendlyName() const override { return TEXT("FInstanceOriginBuffer"); }
	} InstanceOriginBuffer;
	FShaderResourceViewRHIRef InstanceOriginSRV;

	class FInstanceTransformBuffer : public FVertexBuffer
	{
		virtual FString GetFriendlyName() const override { return TEXT("FInstanceTransformBuffer"); }
	} InstanceTransformBuffer;
	FShaderResourceViewRHIRef InstanceTransformSRV;

#if REQUIRES_COMMANDLIST
	void CreateVertexBuffer(FRHICommandListBase& RHICmdList, FResourceArrayInterface* InResourceArray, EBufferUsageFlags InUsage, uint32 InStride, uint8 InFormat, FBufferRHIRef& OutVertexBufferRHI, FShaderResourceViewRHIRef& OutInstanceSRV);
#else
	void CreateVertexBuffer(FResourceArrayInterface* InResourceArray, EBufferUsageFlags InUsage, uint32 InStride, uint8 InFormat, FBufferRHIRef& OutVertexBufferRHI, FShaderResourceViewRHIRef& OutInstanceSRV);
#endif
};

/**
* A vertex factory for instanced static meshes
*/
struct FAlterMeshVertexFactory : public FLocalVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FAlterMeshVertexFactory);
public:
	FAlterMeshVertexFactory(ERHIFeatureLevel::Type InFeatureLevel)
		: FLocalVertexFactory(InFeatureLevel, "FAlterMeshVertexFactory")
	{
		//bSupportsManualVertexFetch = true;
	}

	struct FDataType : public FAlterMeshDataType, public FLocalVertexFactory::FDataType
	{
	};

	/**
	* Should we cache the material's shadertype on this platform with this vertex factory?
	*/
	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters);

	/**
	* Modify compile environment to enable instancing
	* @param OutEnvironment - shader compile environment to modify
	*/
	static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);

	/**
	* An implementation of the interface used by TSynchronizedResource to update the resource with new data from the game thread.
	*/
	void SetData(const FDataType& InData)
	{
		FLocalVertexFactory::Data = InData;
		Data = InData;
#if REQUIRES_COMMANDLIST
		UpdateRHI(FRHICommandListImmediate::Get());
#else
		UpdateRHI();
#endif
	}

	/**
	* Copy the data from another vertex factory
	* @param Other - factory to copy from
	*/
	void Copy(const FAlterMeshVertexFactory& Other);

	// FRenderResource interface.
#if REQUIRES_COMMANDLIST
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
#else
	virtual void InitRHI() override;	
#endif	

	/** Make sure we account for changes in the signature of GetStaticBatchElementVisibility() */
	static constexpr uint32 NumBitsForVisibilityMask()
	{
		return 8 * sizeof(uint64);
	}

	inline FRHIShaderResourceView* GetInstanceOriginSRV() const
	{
		return Data.InstanceOriginSRV;
	}

	inline FRHIShaderResourceView* GetInstanceTransformSRV() const
	{
		return Data.InstanceTransformSRV;
	}
	FRHIUniformBuffer* GetUniformBuffer() const
	{
		return UniformBuffer.GetReference();
	}


private:
	FDataType Data;
	TUniformBufferRef<FAlterMeshVertexFactoryUniformShaderParameters> UniformBuffer;
	FAlterMeshSceneProxy* SceneProxy;
};

class ALTERMESHRENDERING_API FAlterMeshVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
	DECLARE_TYPE_LAYOUT(FAlterMeshVertexFactoryShaderParameters, NonVirtual);
public:
	void Bind(const FShaderParameterMap& ParameterMap)
	{
		VertexFetch_InstanceOriginBufferParameter.Bind(ParameterMap, TEXT("VertexFetch_InstanceOriginBuffer"));
		VertexFetch_InstanceTransformBufferParameter.Bind(ParameterMap, TEXT("VertexFetch_InstanceTransformBuffer"));
		InstanceOffset.Bind(ParameterMap, TEXT("InstanceOffset"));
	}

	void GetElementShaderBindings(
		const class FSceneInterface* Scene,
		const FSceneView* View,
		const FMeshMaterialShader* Shader,
		const EVertexInputStreamType InputStreamType,
		ERHIFeatureLevel::Type FeatureLevel,
		const FVertexFactory* VertexFactory,
		const FMeshBatchElement& BatchElement,
		FMeshDrawSingleShaderBindings& ShaderBindings,
		FVertexInputStreamArray& VertexStreams
		) const;

private:

	LAYOUT_FIELD(FShaderResourceParameter, VertexFetch_InstanceOriginBufferParameter)
	LAYOUT_FIELD(FShaderResourceParameter, VertexFetch_InstanceTransformBufferParameter)
	LAYOUT_FIELD(FShaderParameter, InstanceOffset)
};

/** The implementation of the static mesh vertex data storage type. */
template<typename VertexDataType>
class TAlterMeshVertexData
{
	using FVertexResourceArray = TResourceArray<VertexDataType, VERTEXBUFFER_ALIGNMENT>;
	FVertexResourceArray Data;
public:

	/**
	* Constructor
	* @param InNeedsCPUAccess - true if resource array data should be CPU accessible
	*/
	TAlterMeshVertexData(bool InNeedsCPUAccess=false)
		: Data(InNeedsCPUAccess)
	{
	}

	/**
	* Resizes the vertex data buffer, discarding any data which no longer fits.
	*
	* @param NumVertices - The number of vertices to allocate the buffer for.
	* @param BufferFlags - Flags to define the expected behavior of the buffer
	*/
	void ResizeBuffer(uint32 NumVertices, EResizeBufferFlags BufferFlags = EResizeBufferFlags::None)
	{
		if ((uint32)Data.Num() < NumVertices)
		{
			// Enlarge the array.
			if (!EnumHasAnyFlags(BufferFlags, EResizeBufferFlags::AllowSlackOnGrow))
			{
				Data.Reserve(NumVertices);
			}

			Data.AddUninitialized(NumVertices - Data.Num());
		}
		else if ((uint32)Data.Num() > NumVertices)
		{
			// Shrink the array.
			bool AllowShinking = !EnumHasAnyFlags(BufferFlags, EResizeBufferFlags::AllowSlackOnReduce);
			Data.RemoveAt(NumVertices, Data.Num() - NumVertices, AllowShinking);
		}
	}

	void Empty(uint32 NumVertices)
	{
		Data.Empty(NumVertices);
	}

	bool IsValidIndex(uint32 Index)
	{
		return Data.IsValidIndex(Index);
	}

	/**
	* @return stride of the vertex type stored in the resource data array
	*/
	uint32 GetStride() const
	{
		return sizeof(VertexDataType);
	}
	/**
	* @return uint8 pointer to the resource data array
	*/
	uint8* GetDataPointer()
	{
		return (uint8*)Data.GetData();
	}

	/**
	* @return resource array interface access
	*/
	FResourceArrayInterface* GetResourceArray()
	{
		return &Data;
	}

	const FResourceArrayInterface* GetResourceArray() const
	{
		return &Data;
	}

	/**
	* Assignment. This is currently the only method which allows for
	* modifying an existing resource array
	*/
	void Assign(const TArray<VertexDataType>& Other)
	{
		ResizeBuffer(Other.Num());
		if (Other.Num())
		{
			memcpy(GetDataPointer(), &Other[0], Other.Num() * sizeof(VertexDataType));
		}
	}

	/**
	* Helper function to return the amount of memory allocated by this
	* container.
	*
	* @returns Number of bytes allocated by this container.
	*/
	SIZE_T GetResourceSize() const
	{
		return Data.GetAllocatedSize();
	}

	/**
	* Helper function to return the number of elements by this
	* container.
	*
	* @returns Number of elements allocated by this container.
	*/
	int32 Num() const
	{
		return Data.Num();
	}

	bool GetAllowCPUAccess() const
	{
		return Data.GetAllowCPUAccess();
	}

	void OverrideFreezeSizeAndAlignment(int64& Size, int32& Alignment) const
	{
		Size = sizeof(*this);
	}
};

/** The implementation of the static mesh instance data storage type. */
class FAlterMeshInstanceData
{
	template<typename F>
	struct FInstanceTransformMatrix
	{
		F InstanceTransform1[4];
		F InstanceTransform2[4];
		F InstanceTransform3[4];
	};

public:

	/**
	 * Constructor
	 */
	FAlterMeshInstanceData()
	{
		AllocateBuffers(0);
	}

	~FAlterMeshInstanceData()
	{
		delete InstanceOriginData;
		delete InstanceTransformData;
	}

	void AllocateInstances(int32 InNumInstances, EResizeBufferFlags BufferFlags, bool DestroyExistingInstances)
	{
		NumInstances = InNumInstances;

		if (DestroyExistingInstances)
		{
			InstanceOriginData->Empty(NumInstances);
			InstanceTransformData->Empty(NumInstances);
		}

		// We cannot write directly to the data on all platforms,
		// so we make a TArray of the right type, then assign it
		InstanceOriginData->ResizeBuffer(NumInstances, BufferFlags);
		InstanceOriginDataPtr = InstanceOriginData->GetDataPointer();
		InstanceTransformData->ResizeBuffer(NumInstances, BufferFlags);
		InstanceTransformDataPtr = InstanceTransformData->GetDataPointer();
	}

	FORCEINLINE_DEBUGGABLE void SetInstance(int32 InstanceIndex, const FMatrix44f& Transform, float RandomInstanceID)
	{
		FVector4f Origin(Transform.M[3][0], Transform.M[3][1], Transform.M[3][2], RandomInstanceID);
		SetInstanceOriginInternal(InstanceIndex, Origin);

		FVector4f InstanceTransform[3];
		InstanceTransform[0] = FVector4f(Transform.M[0][0], Transform.M[0][1], Transform.M[0][2], 0.0f);
		InstanceTransform[1] = FVector4f(Transform.M[1][0], Transform.M[1][1], Transform.M[1][2], 0.0f);
		InstanceTransform[2] = FVector4f(Transform.M[2][0], Transform.M[2][1], Transform.M[2][2], 0.0f);

		SetInstanceTransformInternal<float>(InstanceIndex, InstanceTransform);
	}

	FORCEINLINE_DEBUGGABLE int32 GetNumInstances() const
	{
		return NumInstances;
	}

	FORCEINLINE_DEBUGGABLE void SetAllowCPUAccess(bool InNeedsCPUAccess)
	{
		if (InstanceOriginData)
		{
			InstanceOriginData->GetResourceArray()->SetAllowCPUAccess(InNeedsCPUAccess);
		}
		if (InstanceTransformData)
		{
			InstanceTransformData->GetResourceArray()->SetAllowCPUAccess(InNeedsCPUAccess);
		}
	}

	FORCEINLINE_DEBUGGABLE FResourceArrayInterface* GetOriginResourceArray()
	{
		return InstanceOriginData->GetResourceArray();
	}

	FORCEINLINE_DEBUGGABLE FResourceArrayInterface* GetTransformResourceArray()
	{
		return InstanceTransformData->GetResourceArray();
	}

	FORCEINLINE_DEBUGGABLE uint32 GetOriginStride()
	{
		return InstanceOriginData->GetStride();
	}

	FORCEINLINE_DEBUGGABLE uint32 GetTransformStride()
	{
		return InstanceTransformData->GetStride();
	}

	FORCEINLINE_DEBUGGABLE SIZE_T GetResourceSize() const
	{
		return	InstanceOriginData->GetResourceSize() +
				InstanceTransformData->GetResourceSize();
	}

private:
	template<typename T>
	FORCEINLINE_DEBUGGABLE void GetInstanceTransformInternal(int32 InstanceIndex, FVector4 (&Transform)[3]) const
	{
		FInstanceTransformMatrix<T>* ElementData = reinterpret_cast<FInstanceTransformMatrix<T>*>(InstanceTransformDataPtr);
		uint32 CurrentSize = InstanceTransformData->Num() * InstanceTransformData->GetStride();
		check((void*)((&ElementData[InstanceIndex]) + 1) <= (void*)(InstanceTransformDataPtr + CurrentSize));
		check((void*)((&ElementData[InstanceIndex]) + 0) >= (void*)(InstanceTransformDataPtr));

		Transform[0][0] = ElementData[InstanceIndex].InstanceTransform1[0];
		Transform[0][1] = ElementData[InstanceIndex].InstanceTransform1[1];
		Transform[0][2] = ElementData[InstanceIndex].InstanceTransform1[2];
		Transform[0][3] = ElementData[InstanceIndex].InstanceTransform1[3];

		Transform[1][0] = ElementData[InstanceIndex].InstanceTransform2[0];
		Transform[1][1] = ElementData[InstanceIndex].InstanceTransform2[1];
		Transform[1][2] = ElementData[InstanceIndex].InstanceTransform2[2];
		Transform[1][3] = ElementData[InstanceIndex].InstanceTransform2[3];

		Transform[2][0] = ElementData[InstanceIndex].InstanceTransform3[0];
		Transform[2][1] = ElementData[InstanceIndex].InstanceTransform3[1];
		Transform[2][2] = ElementData[InstanceIndex].InstanceTransform3[2];
		Transform[2][3] = ElementData[InstanceIndex].InstanceTransform3[3];
	}

	FORCEINLINE_DEBUGGABLE void GetInstanceOriginInternal(int32 InstanceIndex, FVector4 &Origin) const
	{
		FVector4* ElementData = reinterpret_cast<FVector4*>(InstanceOriginDataPtr);
		uint32 CurrentSize = InstanceOriginData->Num() * InstanceOriginData->GetStride();
		check((void*)((&ElementData[InstanceIndex]) + 1) <= (void*)(InstanceOriginDataPtr + CurrentSize));
		check((void*)((&ElementData[InstanceIndex]) + 0) >= (void*)(InstanceOriginDataPtr));

		Origin = ElementData[InstanceIndex];
	}

	template<typename T>
	FORCEINLINE_DEBUGGABLE void SetInstanceTransformInternal(int32 InstanceIndex, FVector4f(Transform)[3]) const
	{
		FInstanceTransformMatrix<T>* ElementData = reinterpret_cast<FInstanceTransformMatrix<T>*>(InstanceTransformDataPtr);
		uint32 CurrentSize = InstanceTransformData->Num() * InstanceTransformData->GetStride();
		check((void*)((&ElementData[InstanceIndex]) + 1) <= (void*)(InstanceTransformDataPtr + CurrentSize));
		check((void*)((&ElementData[InstanceIndex]) + 0) >= (void*)(InstanceTransformDataPtr));

		ElementData[InstanceIndex].InstanceTransform1[0] = Transform[0][0];
		ElementData[InstanceIndex].InstanceTransform1[1] = Transform[0][1];
		ElementData[InstanceIndex].InstanceTransform1[2] = Transform[0][2];
		ElementData[InstanceIndex].InstanceTransform1[3] = Transform[0][3];

		ElementData[InstanceIndex].InstanceTransform2[0] = Transform[1][0];
		ElementData[InstanceIndex].InstanceTransform2[1] = Transform[1][1];
		ElementData[InstanceIndex].InstanceTransform2[2] = Transform[1][2];
		ElementData[InstanceIndex].InstanceTransform2[3] = Transform[1][3];

		ElementData[InstanceIndex].InstanceTransform3[0] = Transform[2][0];
		ElementData[InstanceIndex].InstanceTransform3[1] = Transform[2][1];
		ElementData[InstanceIndex].InstanceTransform3[2] = Transform[2][2];
		ElementData[InstanceIndex].InstanceTransform3[3] = Transform[2][3];
	}

	FORCEINLINE_DEBUGGABLE void SetInstanceOriginInternal(int32 InstanceIndex, const FVector4f& Origin) const
	{
		FVector4f* ElementData = reinterpret_cast<FVector4f*>(InstanceOriginDataPtr);
		uint32 CurrentSize = InstanceOriginData->Num() * InstanceOriginData->GetStride();
		checkf((void*)((&ElementData[InstanceIndex]) + 1) <= (void*)(InstanceOriginDataPtr + CurrentSize), TEXT("OOB Instance Set Under: %i, %u, %p, %p"), InstanceIndex, CurrentSize, &ElementData, InstanceOriginDataPtr);
		checkf((void*)((&ElementData[InstanceIndex]) + 0) >= (void*)(InstanceOriginDataPtr), TEXT("OOB Instance Set: %i, %u, %p, %p"), InstanceIndex, CurrentSize, &ElementData, InstanceOriginDataPtr);

		ElementData[InstanceIndex] = Origin;
	}

	void AllocateBuffers(int32 InNumInstances, EResizeBufferFlags BufferFlags = EResizeBufferFlags::None)
	{
		delete InstanceOriginData;
		InstanceOriginDataPtr = nullptr;

		delete InstanceTransformData;
		InstanceTransformDataPtr = nullptr;

		InstanceOriginData = new TAlterMeshVertexData<FVector4>();
		InstanceOriginData->ResizeBuffer(InNumInstances, BufferFlags);
		InstanceTransformData = new TAlterMeshVertexData<FInstanceTransformMatrix<float>>();
		InstanceTransformData->ResizeBuffer(InNumInstances, BufferFlags);
	}

	TAlterMeshVertexData<FVector4>* InstanceOriginData = nullptr;
	uint8* InstanceOriginDataPtr = nullptr;

	TAlterMeshVertexData<FInstanceTransformMatrix<float>>* InstanceTransformData = nullptr;
	uint8* InstanceTransformDataPtr = nullptr;

	int32 NumInstances = 0;
};

