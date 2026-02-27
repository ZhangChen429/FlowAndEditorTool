// Copyright 2023 Aechmea

#include "AlterMeshComponent.h"

#include "AlterMeshVertexFactory.h"
#include "EngineUtils.h"
#include "PrimitiveViewRelevance.h"
#include "RenderResource.h"
#include "PrimitiveSceneProxy.h"
#include "VertexFactory.h"
#include "MaterialShared.h"
#include "SceneManagement.h"
#include "Engine/CollisionProfile.h"
#include "Materials/Material.h"
#include "Engine/Engine.h"
#include "StaticMeshResources.h"
#include "SceneInterface.h"

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2
#include "MaterialDomain.h"
#include "Materials/MaterialRenderProxy.h"
#endif

IMPLEMENT_HIT_PROXY(HAlterMeshComponentProxy, HHitProxy);

class FAlterMeshSectionProxy
{
public:
	int32 VertexOffset;
	int32 NumVertices;
	UMaterialInterface* Material;
	FPositionVertexBuffer PositionBuffer;
	FColorVertexBuffer ColorVertexBuffer;
	FStaticMeshVertexBuffer VertexDataBuffer;
	FRawStaticIndexBuffer IndexBuffer;
	FAlterMeshVertexFactory VertexFactory;
	FAlterMeshInstanceBuffer InstanceBuffer;
	bool bSectionVisible;

	FAlterMeshSectionProxy(ERHIFeatureLevel::Type InFeatureLevel)
		: Material(nullptr)
		, VertexFactory(InFeatureLevel)
		, bSectionVisible(true)
	{
	}
};

class FAlterMeshSceneProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	static void InitOrUpdateResource(FRenderResource* Resource)
	{
		check(IsInRenderingThread());

		if (!Resource->IsInitialized())
		{
#if REQUIRES_COMMANDLIST
			Resource->InitResource(FRHICommandListImmediate::Get());
#else
			Resource->InitResource();
#endif			
		}
		else
		{
#if REQUIRES_COMMANDLIST
			Resource->UpdateRHI(FRHICommandListImmediate::Get());
#else
			Resource->UpdateRHI();
#endif
		}
	}

	FAlterMeshSceneProxy(UAlterMeshComponent* Component)
		: FPrimitiveSceneProxy(Component)
		, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetShaderPlatform()))
	{
		bEvaluateWorldPositionOffset = true;
		
		check(Component->Sections.Num() > 0);
		uint32 VertexOffset = 0;
		Sections.AddZeroed(Component->Sections.Num());

		int32 SectionIndex = 0;
		for (FAlterMeshSection& Section : Component->Sections)
		{
			check(Section.Instances.Num() > 0);

			FAlterMeshSectionProxy* NewSection = new FAlterMeshSectionProxy(GetScene().GetFeatureLevel());
			NewSection->PositionBuffer.Init(Section.Vertices);
			NewSection->VertexDataBuffer.Init(Section.Vertices.Num(), 4);

			for (int32 i = 0; i < Section.Vertices.Num(); i++)
			{
				if (Section.UV0.Num())
				{
					NewSection->VertexDataBuffer.SetVertexUV(i, 0, Section.UV0[i]);
				}
				
				if (Section.UV1.Num())
				{
					NewSection->VertexDataBuffer.SetVertexUV(i, 1, Section.UV1[i]);
				}
				
				if (Section.UV2.Num())
				{
					NewSection->VertexDataBuffer.SetVertexUV(i, 2, Section.UV2[i]);
				}
				
				if (Section.UV3.Num())
				{
					NewSection->VertexDataBuffer.SetVertexUV(i, 3, Section.UV3[i]);
				}
				
				NewSection->VertexDataBuffer.SetVertexTangents(i, Section.Tangents[i],Section.Bitangents[i], Section.Normals[i]);
			}

			if (Section.Colors.Num())
			{
				NewSection->ColorVertexBuffer.InitFromColorArray(Section.Colors);
			}
			
			NewSection->IndexBuffer.SetIndices(Section.Indices, EIndexBufferStride::Force32Bit);

			NewSection->Material = Component->GetMaterial(SectionIndex) ? Component->GetMaterial(SectionIndex) : Section.Material;
			if (NewSection->Material)
			{
				NewSection->Material->CheckMaterialUsage(EMaterialUsage::MATUSAGE_InstancedStaticMeshes);
			}
			else
			{
				NewSection->Material = UMaterial::GetDefaultMaterial(MD_Surface);
			}

			NewSection->NumVertices = Section.Vertices.Num();
			NewSection->VertexOffset = VertexOffset;
			VertexOffset += Section.Vertices.Num();

			TSharedRef<FAlterMeshInstanceData, ESPMode:: ThreadSafe> InstanceBufferData = MakeShared<FAlterMeshInstanceData, ESPMode::ThreadSafe>();
			InstanceBufferData->AllocateInstances(Section.Instances.Num(), EResizeBufferFlags::AllowSlackOnGrow | EResizeBufferFlags::AllowSlackOnReduce, false);

			for (int32 i = 0; i < Section.Instances.Num(); i++)
			{
				InstanceBufferData->SetInstance(i, Section.Instances[i], 0);
			}

			NewSection->InstanceBuffer.InstanceData = InstanceBufferData;
			Sections[SectionIndex] = NewSection;

			ENQUEUE_RENDER_COMMAND(AlterMeshVertexBuffersInit)(
			[SectionIndex, this](FRHICommandListImmediate& RHICmdList)
			{
				FAlterMeshSectionProxy* Section = Sections[SectionIndex];
				
#if REQUIRES_COMMANDLIST
				Section->PositionBuffer.InitResource(RHICmdList);
				Section->ColorVertexBuffer.InitResource(RHICmdList);
				Section->VertexDataBuffer.InitResource(RHICmdList);
				Section->InstanceBuffer.InitResource(RHICmdList);
#else
				Section->PositionBuffer.InitResource();
				Section->ColorVertexBuffer.InitResource();
				Section->VertexDataBuffer.InitResource();
				Section->InstanceBuffer.InitResource();
#endif
				
				FAlterMeshVertexFactory::FDataType Data;
				Section->PositionBuffer.BindPositionVertexBuffer(&Section->VertexFactory, Data);
				Section->VertexDataBuffer.BindTangentVertexBuffer(&Section->VertexFactory, Data);
				Section->VertexDataBuffer.BindPackedTexCoordVertexBuffer(&Section->VertexFactory, Data);
				Section->ColorVertexBuffer.BindColorVertexBuffer(&Section->VertexFactory, Data);
				Section->InstanceBuffer.BindInstanceVertexBuffer(&Section->VertexFactory, Data);
				Section->VertexFactory.SetData(Data);

#if REQUIRES_COMMANDLIST
				Section->VertexFactory.InitResource(RHICmdList);
 				Section->IndexBuffer.InitResource(RHICmdList);
#else
				Section->VertexFactory.InitResource();
				Section->IndexBuffer.InitResource();
#endif
				
			});

			SectionIndex++;
		}
	}

	virtual ~FAlterMeshSceneProxy()
	{
		for (FAlterMeshSectionProxy* Section : Sections)
		{
			Section->PositionBuffer.ReleaseResource();
			Section->VertexDataBuffer.ReleaseResource();
			Section->ColorVertexBuffer.ReleaseResource();
			Section->IndexBuffer.ReleaseResource();
			Section->VertexFactory.ReleaseResource();
			Section->InstanceBuffer.ReleaseResource();
			delete Section;
		}

		Sections.Empty();
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_AlterMeshSceneProxy_GetDynamicMeshElements );

		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
			FLinearColor(0, 0.5f, 1.f)
			);

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

		for (const FAlterMeshSectionProxy* Section : Sections)
		{
			FMaterialRenderProxy* MaterialProxy;
			if(bWireframe)
			{
				MaterialProxy = WireframeMaterialInstance;
			}
			else
			{
				MaterialProxy = Section->Material->GetRenderProxy();
			}

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					FMeshBatch& Mesh = Collector.AllocateMesh();
					FMeshBatchElement& BatchElement = Mesh.Elements[0];

					const uint32 NumInstances = Section->InstanceBuffer.InstanceData->GetNumInstances();
					BatchElement.InstancedLODIndex = 0;
					BatchElement.UserIndex = 0;
					BatchElement.PrimitiveUniformBuffer = GetUniformBuffer();
					BatchElement.NumInstances = NumInstances;
					BatchElement.IndexBuffer = &Section->IndexBuffer;
					Mesh.bWireframe = bWireframe;
					Mesh.VertexFactory = &Section->VertexFactory;
					Mesh.MaterialRenderProxy = MaterialProxy;

					bool bHasPrecomputedVolumetricLightmap;
					FMatrix PreviousLocalToWorld;
					int32 SingleCaptureIndex;
					bool bOutputVelocity;
					GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);

					FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 4
					DynamicPrimitiveUniformBuffer.Set(Collector.GetRHICommandList(), GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, bOutputVelocity);
#else
					DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, bOutputVelocity);
#endif
					//BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;
					BatchElement.PrimitiveIdMode = PrimID_FromPrimitiveSceneInfo;

					BatchElement.FirstIndex = 0;
					BatchElement.NumPrimitives = Section->IndexBuffer.GetNumIndices()/3;
					BatchElement.MinVertexIndex = 0;
					BatchElement.MaxVertexIndex = Section->NumVertices -1;
					Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
					Mesh.Type = PT_TriangleList;
					Mesh.DepthPriorityGroup = SDPG_World;
					Mesh.bCanApplyViewModeOverrides = false;
					Collector.AddMesh(ViewIndex, Mesh);

	#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
					// Render bounds
					RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
	#endif
				}
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;

		Result.bStaticRelevance = false; 
		Result.bDynamicRelevance = true;
		Result.bDrawRelevance = IsShown(View);
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bRenderInMainPass = ShouldRenderInMainPass();
		Result.bVelocityRelevance = Result.bRenderInMainPass;
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		Result.bRenderInDepthPass = ShouldRenderInDepthPass();
		Result.bEditorVisualizeLevelInstanceRelevance = IsEditingLevelInstanceChild();
		Result.bEditorStaticSelectionRelevance = (IsSelected() || IsHovered());
		Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
		
		MaterialRelevance.SetPrimitiveViewRelevance(Result);
		return Result;
	}

	virtual uint32 GetMemoryFootprint( void ) const override { return( sizeof( *this ) + GetAllocatedSize() ); }

	uint32 GetAllocatedSize( void ) const { return( FPrimitiveSceneProxy::GetAllocatedSize() ); }

	virtual HHitProxy* CreateHitProxies(UPrimitiveComponent* Component, TArray<TRefCountPtr<HHitProxy>>& OutHitProxies) override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_UAlterMeshComponent_CreateHitProxyData);

		OutHitProxies.Empty();

		for (int32 i = 0; i < Cast<UAlterMeshComponent>(Component)->Sections.Num(); i++)
		{
			OutHitProxies.Add(new HActor(Component->GetOwner(), Component, EHitProxyPriority::HPP_Wireframe, i, i));
		}

		return OutHitProxies.Last();
	}

private:

	TArray<FAlterMeshSectionProxy*> Sections;
	FMaterialRelevance MaterialRelevance;
};

//////////////////////////////////////////////////////////////////////////

UAlterMeshComponent::UAlterMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;
	SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
}

void UAlterMeshComponent::OnRegister()
{
	Super::OnRegister();
}

void UAlterMeshComponent::TickComponent(float DeltaTime, ELevelTick Tick, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, Tick, ThisTickFunction);
}

FBoxSphereBounds UAlterMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	return BoundingBox.TransformBy(LocalToWorld);
}

FPrimitiveSceneProxy* UAlterMeshComponent::CreateSceneProxy()
{
	if (Sections.Num() > 0)
	{
		FAlterMeshSceneProxy* Proxy = new FAlterMeshSceneProxy(this);
		return Proxy;
	}

	return nullptr;
}

int32 UAlterMeshComponent::GetNumMaterials() const
{
	return Sections.Num();
}

TArray<FName> UAlterMeshComponent::GetMaterialSlotNames() const
{
	TArray<FName> OutNames;
	for (const FAlterMeshSection& Section : Sections)
	{
		OutNames.Add(Section.MaterialName);
	}

	return OutNames;
}

int32 UAlterMeshComponent::GetMaterialIndex(FName MaterialSlotName) const
{
	for (int32 i = 0; i < Sections.Num(); i++)
	{
		if (Sections[i].MaterialName == MaterialSlotName)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

void UAlterMeshComponent::SetMaterial(int32 ElementIndex, UMaterialInterface* Material)
{
	Super::SetMaterial(ElementIndex, Material);
	Sections[ElementIndex].Material = Material;
	OnMaterialChanged.Broadcast(this, ElementIndex, Material);
}

void UAlterMeshComponent::UpdateBounds()
{
	FBox3f TotalBounds(ForceInitToZero);
	for (FAlterMeshSection& Section : Sections)
	{
		FBox3f MeshBounds = FBox3f(Section.Vertices);
		for (FMatrix44f InstanceMatrix : Section.Instances)
		{
			 TotalBounds += MeshBounds.TransformBy(InstanceMatrix);
		}
	}

	BoundingBox = FBox3d(TotalBounds);

	Super::UpdateBounds();
}
