// Copyright 2023 Aechmea

#include "AlterMeshVertexFactory.h"

#include "ProfilingDebugging/LoadTimeTracker.h"
#include "MeshMaterialShader.h"
#include "Engine/InstancedStaticMesh.h"
#include "LocalVertexFactory.h"
#include "MeshDrawShaderBindings.h"
#include "RHI.h"

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2
#include "DataDrivenShaderPlatformInfo.h"
#endif

IMPLEMENT_TYPE_LAYOUT(FAlterMeshVertexFactoryShaderParameters);
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FAlterMeshVertexFactoryUniformShaderParameters, "AlterMeshInstanceVF");
IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FAlterMeshVertexFactory, SF_Vertex, FAlterMeshVertexFactoryShaderParameters);
IMPLEMENT_VERTEX_FACTORY_TYPE(FAlterMeshVertexFactory,"/Plugin/AlterMesh/AlterMeshVertexFactory.ush",
	  EVertexFactoryFlags::UsedWithMaterials
	| EVertexFactoryFlags::SupportsDynamicLighting
	| EVertexFactoryFlags::SupportsPrecisePrevWorldPos
	| EVertexFactoryFlags::SupportsPositionOnly
	| EVertexFactoryFlags::SupportsCachingMeshDrawCommands
);

bool FAlterMeshVertexFactory::ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters)
{
	return (Parameters.MaterialParameters.bIsUsedWithInstancedStaticMeshes || Parameters.MaterialParameters.bIsSpecialEngineMaterial)
			&& FLocalVertexFactory::ShouldCompilePermutation(Parameters);
}

void FAlterMeshVertexFactory::ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	if (RHISupportsManualVertexFetch(Parameters.Platform))
	{
		OutEnvironment.SetDefine(TEXT("MANUAL_VERTEX_FETCH"), TEXT("1"));
	}
	bool bExplicitWPO = ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3;
	OutEnvironment.SetDefine(TEXT("AM_EXPLICIT_WPO"),  bExplicitWPO ? TEXT("1") : TEXT("0"));
	OutEnvironment.SetDefine(TEXT("USE_INSTANCING"), TEXT("1"));
	OutEnvironment.SetDefine(TEXT("USE_INSTANCE_CULLING"), TEXT("0"));	
	OutEnvironment.SetDefine(TEXT("USE_DITHERED_LOD_TRANSITION_FOR_INSTANCED"), TEXT("0"));
	
	bool bRenameDFToLWC = ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 4;
	OutEnvironment.SetDefine(TEXT("DF_TO_LWC"), bRenameDFToLWC ? TEXT("1") : TEXT("0"));
	
	FLocalVertexFactory::ModifyCompilationEnvironment(Parameters, OutEnvironment);
}

void FAlterMeshVertexFactory::Copy(const FAlterMeshVertexFactory& Other)
{
	FAlterMeshVertexFactory* VertexFactory = this;
	const FDataType* DataCopy = &Other.Data;
	ENQUEUE_RENDER_COMMAND(FAlterMeshVertexFactoryCopyData)(
	[VertexFactory, DataCopy](FRHICommandListImmediate& RHICmdList)
	{
		VertexFactory->Data = *DataCopy;
	});
	BeginUpdateResourceRHI(this);
}

#if REQUIRES_COMMANDLIST
void FAlterMeshVertexFactory::InitRHI(FRHICommandListBase& RHICmdList)
#else
void FAlterMeshVertexFactory::InitRHI()
#endif
{
	SCOPED_LOADTIMER(FAlterMeshVertexFactory_InitRHI);

	check(HasValidFeatureLevel());

#if REQUIRES_COMMANDLIST
	FLocalVertexFactory::InitRHI(RHICmdList);
#else
	FLocalVertexFactory::InitRHI();
#endif
	

	FVertexDeclarationElementList Elements;
	if(Data.PositionComponent.VertexBuffer != NULL)
	{
		Elements.Add(AccessStreamComponent(Data.PositionComponent,0));
	}

	// only tangent,normal are used by the stream. the binormal is derived in the shader
	uint8 TangentBasisAttributes[2] = { 1, 2 };
	for(int32 AxisIndex = 0;AxisIndex < 2;AxisIndex++)
	{
		if(Data.TangentBasisComponents[AxisIndex].VertexBuffer != NULL)
		{
			Elements.Add(AccessStreamComponent(Data.TangentBasisComponents[AxisIndex],TangentBasisAttributes[AxisIndex]));
		}
	}

	if (!Data.ColorComponentsSRV)
	{
		Data.ColorComponentsSRV = GNullColorVertexBuffer.VertexBufferSRV;
		Data.ColorIndexMask = 0;
	}

	if(Data.ColorComponent.VertexBuffer)
	{
		Elements.Add(AccessStreamComponent(Data.ColorComponent,3));
	}
	else
	{
		//If the mesh has no color component, set the null color buffer on a new stream with a stride of 0.
		//This wastes 4 bytes of bandwidth per vertex, but prevents having to compile out twice the number of vertex factories.
		FVertexStreamComponent NullColorComponent(&GNullColorVertexBuffer, 0, 0, VET_Color, EVertexStreamUsage::ManualFetch);
		Elements.Add(AccessStreamComponent(NullColorComponent, 3));
	}

	if(Data.TextureCoordinates.Num())
	{
		const int32 BaseTexCoordAttribute = 4;
		for(int32 CoordinateIndex = 0;CoordinateIndex < Data.TextureCoordinates.Num();CoordinateIndex++)
		{
			Elements.Add(AccessStreamComponent(
				Data.TextureCoordinates[CoordinateIndex],
				BaseTexCoordAttribute + CoordinateIndex
				));
		}

		constexpr int32 MaxTexCoord = 8;
		for(int32 CoordinateIndex = Data.TextureCoordinates.Num(); CoordinateIndex < (MaxTexCoord + 1) / 2; CoordinateIndex++)
		{
			Elements.Add(AccessStreamComponent(
				Data.TextureCoordinates[Data.TextureCoordinates.Num() - 1],
				BaseTexCoordAttribute + CoordinateIndex
				));
		}
	}

	if(Data.LightMapCoordinateComponent.VertexBuffer)
	{
		Elements.Add(AccessStreamComponent(Data.LightMapCoordinateComponent,15));
	}
	else if(Data.TextureCoordinates.Num())
	{
		Elements.Add(AccessStreamComponent(Data.TextureCoordinates[0],15));
	}

	// toss in the instanced location stream
	check(Data.InstanceOriginComponent.VertexBuffer);
	if (Data.InstanceOriginComponent.VertexBuffer)
	{
		Elements.Add(AccessStreamComponent(Data.InstanceOriginComponent, 8));
	}

	check(Data.InstanceTransformComponent[0].VertexBuffer);
	if (Data.InstanceTransformComponent[0].VertexBuffer)
	{
		Elements.Add(AccessStreamComponent(Data.InstanceTransformComponent[0], 9));
		Elements.Add(AccessStreamComponent(Data.InstanceTransformComponent[1], 10));
		Elements.Add(AccessStreamComponent(Data.InstanceTransformComponent[2], 11));
	}

	InitDeclaration(Elements);

	{
		FAlterMeshVertexFactoryUniformShaderParameters UniformParameters;
		UniformParameters.VertexFetch_InstanceOriginBuffer = GetInstanceOriginSRV();
		UniformParameters.VertexFetch_InstanceTransformBuffer = GetInstanceTransformSRV();
		UniformBuffer = TUniformBufferRef<FAlterMeshVertexFactoryUniformShaderParameters>::CreateUniformBufferImmediate(UniformParameters, UniformBuffer_MultiFrame, EUniformBufferValidation::None);
	}
}

void FAlterMeshVertexFactoryShaderParameters::GetElementShaderBindings(
	const class FSceneInterface* Scene,
	const FSceneView* View,
	const FMeshMaterialShader* Shader,
	const EVertexInputStreamType InputStreamType,
	ERHIFeatureLevel::Type FeatureLevel,
	const FVertexFactory* VertexFactory,
	const FMeshBatchElement& BatchElement,
	FMeshDrawSingleShaderBindings& ShaderBindings,
	FVertexInputStreamArray& VertexStreams
	) const
{
	// Decode VertexFactoryUserData as VertexFactoryUniformBuffer
	const FAlterMeshVertexFactory* AlterMeshVertexFactory = static_cast<const FAlterMeshVertexFactory*>(VertexFactory);
	const FLocalVertexFactory* LocalVertexFactory = static_cast<const FLocalVertexFactory*>(VertexFactory);
	ShaderBindings.Add(Shader->GetUniformBufferParameter<FLocalVertexFactoryUniformShaderParameters>(), LocalVertexFactory->GetUniformBuffer());
	ShaderBindings.Add(Shader->GetUniformBufferParameter<FAlterMeshVertexFactoryUniformShaderParameters>(), AlterMeshVertexFactory->GetUniformBuffer());

	if (BatchElement.bUserDataIsColorVertexBuffer)
	{
		FColorVertexBuffer* OverrideColorVertexBuffer = (FColorVertexBuffer*)BatchElement.UserData;
		check(OverrideColorVertexBuffer);

		if (!AlterMeshVertexFactory->SupportsManualVertexFetch(FeatureLevel))
		{
			AlterMeshVertexFactory->GetColorOverrideStream(OverrideColorVertexBuffer, VertexStreams);
		}
	}

	const int32 InstanceOffsetValue = BatchElement.UserIndex;
	ShaderBindings.Add(InstanceOffset, InstanceOffsetValue);

	if (AlterMeshVertexFactory->SupportsManualVertexFetch(FeatureLevel))
	{
		ShaderBindings.Add(VertexFetch_InstanceOriginBufferParameter, AlterMeshVertexFactory->GetInstanceOriginSRV());
		ShaderBindings.Add(VertexFetch_InstanceTransformBufferParameter, AlterMeshVertexFactory->GetInstanceTransformSRV());
	}
	if (InstanceOffsetValue > 0 && VertexStreams.Num() > 0)
	{
		VertexFactory->OffsetInstanceStreams(InstanceOffsetValue, InputStreamType, VertexStreams);
	}
}


FAlterMeshInstanceBuffer::FAlterMeshInstanceBuffer()
{

}

FAlterMeshInstanceBuffer::~FAlterMeshInstanceBuffer()
{
	InstanceData.Reset();
}

	
#if REQUIRES_COMMANDLIST
void FAlterMeshInstanceBuffer::InitRHI(FRHICommandListBase& RHICmdList)
#else
void FAlterMeshInstanceBuffer::InitRHI()
#endif	
{
	check(InstanceData);
	if (InstanceData->GetNumInstances() > 0)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_FAlterMeshInstanceBuffer_InitRHI);
		SCOPED_LOADTIMER(FAlterMeshInstanceBuffer_InitRHI);

		LLM_SCOPE(ELLMTag::InstancedMesh);
		auto AccessFlags = BUF_Static;

#if REQUIRES_COMMANDLIST
		CreateVertexBuffer(RHICmdList, InstanceData->GetOriginResourceArray(), AccessFlags | BUF_ShaderResource, 16, PF_A32B32G32R32F, InstanceOriginBuffer.VertexBufferRHI, InstanceOriginSRV);
		CreateVertexBuffer(RHICmdList, InstanceData->GetTransformResourceArray(), AccessFlags | BUF_ShaderResource, 16, PF_A32B32G32R32F, InstanceTransformBuffer.VertexBufferRHI, InstanceTransformSRV);
#else
		CreateVertexBuffer(InstanceData->GetOriginResourceArray(), AccessFlags | BUF_ShaderResource, 16, PF_A32B32G32R32F, InstanceOriginBuffer.VertexBufferRHI, InstanceOriginSRV);
		CreateVertexBuffer(InstanceData->GetTransformResourceArray(), AccessFlags | BUF_ShaderResource, 16, PF_A32B32G32R32F, InstanceTransformBuffer.VertexBufferRHI, InstanceTransformSRV);
#endif	
	}
}

void FAlterMeshInstanceBuffer::ReleaseRHI()
{
	InstanceOriginSRV.SafeRelease();
	InstanceTransformSRV.SafeRelease();

	InstanceOriginBuffer.ReleaseRHI();
	InstanceTransformBuffer.ReleaseRHI();
}

#if REQUIRES_COMMANDLIST
void FAlterMeshInstanceBuffer::InitResource(FRHICommandListBase& RHICmdList)
{
	FRenderResource::InitResource(RHICmdList);
	InstanceOriginBuffer.InitResource(RHICmdList);
	InstanceTransformBuffer.InitResource(RHICmdList);
}
#else
void FAlterMeshInstanceBuffer::InitResource()
{
	FRenderResource::InitResource();
	InstanceOriginBuffer.InitResource();
	InstanceTransformBuffer.InitResource();
}
#endif
	
void FAlterMeshInstanceBuffer::ReleaseResource()
{
	FRenderResource::ReleaseResource();
	InstanceOriginBuffer.ReleaseResource();
	InstanceTransformBuffer.ReleaseResource();
}

SIZE_T FAlterMeshInstanceBuffer::GetResourceSize() const
{
	if (InstanceData && InstanceData->GetNumInstances() > 0)
	{
		return InstanceData->GetResourceSize();
	}
	return 0;
}

#if REQUIRES_COMMANDLIST
void FAlterMeshInstanceBuffer::CreateVertexBuffer(FRHICommandListBase& RHICmdList, FResourceArrayInterface* InResourceArray, EBufferUsageFlags InUsage, uint32 InStride, uint8 InFormat, FBufferRHIRef& OutVertexBufferRHI, FShaderResourceViewRHIRef& OutInstanceSRV)
#else
void FAlterMeshInstanceBuffer::CreateVertexBuffer(FResourceArrayInterface* InResourceArray, EBufferUsageFlags InUsage, uint32 InStride, uint8 InFormat, FBufferRHIRef& OutVertexBufferRHI, FShaderResourceViewRHIRef& OutInstanceSRV)
#endif
{
	check(InResourceArray);
	check(InResourceArray->GetResourceDataSize() > 0);

	const uint32 BufferSize = InResourceArray->GetResourceDataSize();
	FRHIBufferDesc BufferDesc(BufferSize, InStride, InUsage);
	FRHIBufferCreateDesc CreateDesc(TEXT("FAlterMeshInstanceBuffer"), BufferDesc);
	CreateDesc.SetInitActionResourceArray(InResourceArray);

#if REQUIRES_COMMANDLIST
	OutVertexBufferRHI = RHICmdList.CreateBuffer(CreateDesc);
#else
	OutVertexBufferRHI = RHICreateBuffer(CreateDesc);
#endif

	if (RHISupportsManualVertexFetch(GMaxRHIShaderPlatform))
	{
		FShaderResourceViewInitializer ViewDesc(OutVertexBufferRHI, InFormat, InStride);
#if REQUIRES_COMMANDLIST
		OutInstanceSRV = RHICmdList.CreateShaderResourceView(ViewDesc);
#else
		OutInstanceSRV = RHICreateShaderResourceView(ViewDesc);
#endif
	}
}

void FAlterMeshInstanceBuffer::BindInstanceVertexBuffer(const class FVertexFactory* VertexFactory, FAlterMeshDataType& InstancedStaticMeshData) const
{
	if (InstanceData->GetNumInstances())
	{
		if (RHISupportsManualVertexFetch(GMaxRHIShaderPlatform))
		{
			check(InstanceOriginSRV);
			check(InstanceTransformSRV);
		}
	}

	{
		InstancedStaticMeshData.InstanceOriginSRV = InstanceOriginSRV;
		InstancedStaticMeshData.InstanceTransformSRV = InstanceTransformSRV;
	}

	{
		InstancedStaticMeshData.InstanceOriginComponent = FVertexStreamComponent(
			&InstanceOriginBuffer,
			0,
			16,
			VET_Float4,
			EVertexStreamUsage::ManualFetch | EVertexStreamUsage::Instancing
		);

		EVertexElementType TransformType = VET_Float4;
		uint32 TransformStride = 16;

		InstancedStaticMeshData.InstanceTransformComponent[0] = FVertexStreamComponent(
			&InstanceTransformBuffer,
			0 * TransformStride,
			3 * TransformStride,
			TransformType,
			EVertexStreamUsage::ManualFetch | EVertexStreamUsage::Instancing
		);
		InstancedStaticMeshData.InstanceTransformComponent[1] = FVertexStreamComponent(
			&InstanceTransformBuffer,
			1 * TransformStride,
			3 * TransformStride,
			TransformType,
			EVertexStreamUsage::ManualFetch | EVertexStreamUsage::Instancing
		);
		InstancedStaticMeshData.InstanceTransformComponent[2] = FVertexStreamComponent(
			&InstanceTransformBuffer,
			2 * TransformStride,
			3 * TransformStride,
			TransformType,
			EVertexStreamUsage::ManualFetch | EVertexStreamUsage::Instancing
		);
	}
}

