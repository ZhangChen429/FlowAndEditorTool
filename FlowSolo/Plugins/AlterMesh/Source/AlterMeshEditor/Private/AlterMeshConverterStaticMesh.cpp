// Copyright 2023 Aechmea

#include "AlterMeshConverterStaticMesh.h"

#include "AlterMeshActor.h"
#include "AlterMeshComponent.h"
#include "AssetToolsModule.h"
#include "ConvexDecompTool.h"
#include "DetailWidgetRow.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "StructUtils/StructView.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "CommonParams/AlterMeshBoolParam.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Dialogs/DlgPickAssetPath.h"
#include "Engine/StaticMeshActor.h"
#include "ImportUtils/StaticMeshImportUtils.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PhysicsEngine/BodySetup.h"
#include "StaticMeshEditorSubsystem.h"
#include "UObject/MetaData.h"
#include "Editor.h"
#include "Materials/Material.h"

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2
#include "MaterialDomain.h"
#endif

UAlterMeshConverterStaticMesh::UAlterMeshConverterStaticMesh()
{
	BuildSettings.bRecomputeTangents = true;
	BuildSettings.bUseMikkTSpace = true;
	BuildSettings.bRecomputeNormals = false;
	BuildSettings.bRemoveDegenerates = false;
	BuildSettings.bUseHighPrecisionTangentBasis = false;
	BuildSettings.bUseFullPrecisionUVs = false;
	BuildSettings.bGenerateLightmapUVs = true;
	BuildSettings.SrcLightmapIndex = 0;
	BuildSettings.DstLightmapIndex = 1;

	bCombineMaterials = false;
	bBuildCollision = true;
}

void UAlterMeshConverterStaticMesh::BuildMeshDescription(const TArray<FAlterMeshSection>& Sections, FMeshDescription& OutMeshDescription)
{
	FStaticMeshAttributes AttributeGetter(OutMeshDescription);
	AttributeGetter.Register();

	TPolygonGroupAttributesRef<FName> PolygonGroupNames = AttributeGetter.GetPolygonGroupMaterialSlotNames();
	TVertexAttributesRef<FVector3f> VertexPositions = AttributeGetter.GetVertexPositions();
	TVertexInstanceAttributesRef<FVector3f> Tangents = AttributeGetter.GetVertexInstanceTangents();
	TVertexInstanceAttributesRef<float> BinormalSigns = AttributeGetter.GetVertexInstanceBinormalSigns();
	TVertexInstanceAttributesRef<FVector3f> Normals = AttributeGetter.GetVertexInstanceNormals();
	TVertexInstanceAttributesRef<FVector4f> Colors = AttributeGetter.GetVertexInstanceColors();
	TVertexInstanceAttributesRef<FVector2f> UVs = AttributeGetter.GetVertexInstanceUVs();

	// Materials to apply to new mesh
	const int32 NumSections = Sections.Num();
	int32 VertexCount = 0;
	int32 VertexInstanceCount = 0;
	int32 PolygonCount = 0;

	TMap<UMaterialInterface*, FPolygonGroupID> UniqueMaterials;
	TArray<FPolygonGroupID> PolygonGroupForSection;
	PolygonGroupForSection.Reserve(NumSections);

	for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
	{
		const FAlterMeshSection& Section = Sections[SectionIndex];
		VertexCount += Section.Vertices.Num();
		VertexInstanceCount += Section.Indices.Num();
		PolygonCount += Section.Indices.Num() / 3;
	}

	OutMeshDescription.ReserveNewVertices(VertexCount);
	OutMeshDescription.ReserveNewVertexInstances(VertexInstanceCount);
	OutMeshDescription.ReserveNewPolygons(PolygonCount);
	OutMeshDescription.ReserveNewEdges(PolygonCount * 2);

	int32 NumUVMaps = 0;

	// Create the Polygon Groups
	for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
	{
		UMaterialInterface* Material = Sections[SectionIndex].Material;
		if (!Material)
		{
			Material = UMaterial::GetDefaultMaterial(MD_Surface);
		}

		FPolygonGroupID NewPolygonGroup;
		if (!bCombineMaterials)
		{
			NewPolygonGroup = OutMeshDescription.CreatePolygonGroup();
			PolygonGroupNames[NewPolygonGroup] = Material->GetFName();			
		}

		if (bCombineMaterials && !UniqueMaterials.Contains(Material))
		{
			NewPolygonGroup = OutMeshDescription.CreatePolygonGroup();
			UniqueMaterials.Add(Material, NewPolygonGroup);
			PolygonGroupNames[NewPolygonGroup] = Material->GetFName();
		}

		FPolygonGroupID PolygonGroupID = bCombineMaterials ? *UniqueMaterials.Find(Material) : NewPolygonGroup;
		PolygonGroupForSection.Add(PolygonGroupID);
	}


	// Add Vertex and VertexInstance and polygon for each section
	for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
	{
		const FAlterMeshSection& Section = Sections[SectionIndex];
		FPolygonGroupID PolygonGroupID = PolygonGroupForSection[SectionIndex];

		NumUVMaps = FMath::Max(Section.UV0.Num() ? 1 : 0, NumUVMaps);
		NumUVMaps = FMath::Max(Section.UV1.Num() ? 2 : 0, NumUVMaps);
		NumUVMaps = FMath::Max(Section.UV2.Num() ? 3 : 0, NumUVMaps);
		NumUVMaps = FMath::Max(Section.UV3.Num() ? 4 : 0, NumUVMaps);

		if (NumUVMaps > UVs.GetNumChannels())
		{
			UVs.SetNumChannels(NumUVMaps);
		}
		
		// Create the vertex
		int32 NumVertex = Section.Vertices.Num();
		TMap<int32, FVertexID> VertexIndexToVertexID;
		VertexIndexToVertexID.Reserve(NumVertex);
		for (int32 VertexIndex = 0; VertexIndex < NumVertex; ++VertexIndex)
		{
			const FVertexID VertexID = OutMeshDescription.CreateVertex();
			VertexPositions[VertexID] = Section.Vertices[VertexIndex];
			VertexIndexToVertexID.Add(VertexIndex, VertexID);
		}

		// Create the VertexInstance
		int32 NumIndices = Section.Indices.Num();
		int32 NumTriangles = NumIndices / 3;
		TMap<int32, FVertexInstanceID> IndiceIndexToVertexInstanceID;
		IndiceIndexToVertexInstanceID.Reserve(NumVertex);
		
		for (int32 IndiceIndex = 0; IndiceIndex < NumIndices; IndiceIndex++)
		{
			const int32 VertexIndex = Section.Indices[IndiceIndex];
			const FVertexID VertexID = VertexIndexToVertexID[VertexIndex];
			const FVertexInstanceID VertexInstanceID =
				OutMeshDescription.CreateVertexInstance(VertexID);
			IndiceIndexToVertexInstanceID.Add(IndiceIndex, VertexInstanceID);

			Tangents[VertexInstanceID] = Section.Tangents[VertexIndex];
			Normals[VertexInstanceID] = Section.Normals[VertexIndex];
			BinormalSigns[VertexInstanceID] = GetBasisDeterminantSign(FVector(Section.Tangents[VertexIndex]), FVector(Section.Bitangents[VertexIndex]), FVector(Section.Normals[VertexIndex]));

			Colors[VertexInstanceID] =  Section.Colors.IsValidIndex(VertexIndex) ? FLinearColor(Section.Colors[VertexIndex]) : FLinearColor::Black;
			
			UVs.Set(VertexInstanceID, 0, Section.UV0.IsValidIndex(VertexIndex) ? Section.UV0[VertexIndex] : FVector2f::ZeroVector);
			
			if (NumUVMaps >= 2)
			{
				UVs.Set(VertexInstanceID, 1, Section.UV1[VertexIndex]);
			}
			
			if (NumUVMaps >= 3)
			{
				UVs.Set(VertexInstanceID, 2, Section.UV2[VertexIndex]);
			}
			
			if (NumUVMaps >= 4)
			{
				UVs.Set(VertexInstanceID, 3, Section.UV3[VertexIndex]);
			}
		}

		// Create the polygons for this section
		for (int32 TriangleIndex = 0; TriangleIndex < NumTriangles; TriangleIndex++)
		{
			TArray<FVertexInstanceID> VertexInstanceIDs;
			VertexInstanceIDs.SetNum(3);

			for (int32 CornerIndex = 0; CornerIndex < 3; ++CornerIndex)
			{
				const int32 IndiceIndex = (TriangleIndex * 3) + CornerIndex;
				VertexInstanceIDs[CornerIndex] = IndiceIndexToVertexInstanceID[IndiceIndex];
			}

			// Insert a polygon into the mesh
			OutMeshDescription.CreatePolygon(PolygonGroupID, VertexInstanceIDs);
		}
	}

}

void UAlterMeshConverterStaticMesh::Initialize(AAlterMeshActor* InActor)
{
	WeakActor = InActor;

	ConverterSteps.Empty();

	if (bCombineMeshes)
	{
		FAlterMeshConverterStep Action;
		Action.Action = EAlterMeshConverterAction::CreateNewMesh;

		// Combine all sections with same material together
		TMap<UMaterialInterface*, FAlterMeshSection> CombinedSections;

		ForEachSection(InActor, [&](FAlterMeshSection& Section)
		{
			FAlterMeshSection& UniqueSection = CombinedSections.FindOrAdd(Section.Material);
			UniqueSection += Section;
		});

		TArray<FAlterMeshSection> UniqueSections;
		for (auto& Elem : CombinedSections)
		{
			UniqueSections.Add(MoveTemp(Elem.Value));
		}

		Action.Checksum = CalculateMeshHash(UniqueSections);

		if (UniqueSections.Num())
		{
			ConverterSteps.Add(Action);
		}
	}
	else
	{
		for (UAlterMeshComponent* Component : InActor->AlterMeshComponents)
		{
			// Don't care about invisible components, those are instances of unreal assets
			if (Component->IsVisible())
			{
				FAlterMeshConverterStep Step;

				Step.Action = EAlterMeshConverterAction::CreateNewMesh;
				Step.Checksum = CalculateMeshHash(Component->Sections);

				ConverterSteps.Add(Step);
			}
		}
		
	}

	// Instances
	for (UActorComponent* Instance : InActor->AssetInstanceComponents)
	{
		FAlterMeshConverterStep Step;
		Step.Action = EAlterMeshConverterAction::UseExistingMesh;

		if (Cast<UInstancedStaticMeshComponent>(Instance))
		{
			Step.AssetToUse = Cast<UInstancedStaticMeshComponent>(Instance)->GetStaticMesh();
		}
		
		if (Cast<UBlueprint>(Instance))
		{
			Step.AssetToUse = Cast<UBlueprint>(Instance)->GeneratedClass.Get();
		}
		
		ConverterSteps.Add(Step);
	}

	// See if there are existing meshes already
	for (FAlterMeshConverterStep& Action : ConverterSteps)
	{
		TSet<FName>& GlobalTagsForAssetRegistry = UObject::GetMetaDataTagsForAssetRegistry();
		if (!GlobalTagsForAssetRegistry.Contains(ChecksumTagName))
		{
			GlobalTagsForAssetRegistry.Add(ChecksumTagName);
		}

		TArray<FAssetData> Assets;
		FAssetRegistryModule::GetRegistry().GetAssetsByTags({ ChecksumTagName }, Assets);

		for (const FAssetData& Asset : Assets)
		{
			FString TagValue;
			Asset.GetTagValue(ChecksumTagName, TagValue);
			if (TagValue == FString::Printf(TEXT("%lu"), Action.Checksum))
			{
				Action.Action = EAlterMeshConverterAction::UseExistingMesh;
				Action.AssetToUse = Asset.ToSoftObjectPath();
				break;
			}
		}
	}
}

void UAlterMeshConverterStaticMesh::Convert(AAlterMeshActor* InActor)
{
	Super::Convert(InActor);

	Initialize(InActor);
	
 	CreateAssets(InActor);

 	PlaceAssets(InActor);	
}

void UAlterMeshConverterStaticMesh::CreateAssets(AAlterMeshActor* InActor)
{
	// Todo set path to same as asset
	FString NewNameSuggestion = FString(TEXT("StaticMesh"));
	FString PackageName = UKismetSystemLibrary::ConvertToRelativePath( InActor->Asset->GetPathName()) + NewNameSuggestion;
	FString Name;

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	AssetToolsModule.Get().CreateUniqueAssetName(PackageName, TEXT(""), PackageName, Name);

	TArray<FMeshDescription> MeshDescriptions;
	TArray<int32> MeshHashes;
	MeshHashes.AddZeroed(ConverterSteps.Num());
	MeshDescriptions.AddDefaulted(ConverterSteps.Num());

	bool bHasAnyCreateAssetStep = false;

	for (const FAlterMeshConverterStep& Step : ConverterSteps)
	{
		bHasAnyCreateAssetStep |= Step.Action == EAlterMeshConverterAction::CreateNewMesh;
	}

	TArray<UObject*> CreatedAssets;
	
	for (int32 Step = 0; Step < ConverterSteps.Num(); Step++)
	{
		if (ConverterSteps[Step].Action == EAlterMeshConverterAction::CreateNewMesh)
		{
			// Materials
			TArray<UMaterialInterface*> NewMeshMaterials;

			if (bCombineMeshes)
			{
				TArray<FAlterMeshSection> ResultSections;

				if (bCombineMaterials)
				{
					// All sections with same APPLIED MATERIAL gets combined
					TMap<UMaterialInterface*, FAlterMeshSection> CombinedMaterialSections;
					ForEachSection(InActor, [&](FAlterMeshSection& Section)
					{
						FAlterMeshSection* UniqueSection = CombinedMaterialSections.Find(Section.Material);
						if (!UniqueSection)
						{
							// Create new material slot
							NewMeshMaterials.AddUnique(Section.Material);
							UniqueSection = &CombinedMaterialSections.Add(Section.Material);
						}						
						*UniqueSection += Section;
					});

					for (auto& Elem : CombinedMaterialSections)
					{
						ResultSections.Add(MoveTemp(Elem.Value));
					}
				}
				else
				{
					// Only sections with same material SLOT NAME gets combined
					TMap<FName, FAlterMeshSection> CombinedSlotSections;
					ForEachSection(InActor, [&](FAlterMeshSection& Section)
					{
						FAlterMeshSection* UniqueSection = CombinedSlotSections.Find(Section.MaterialName);
						if (!UniqueSection)
						{
							// Create new material slot
							NewMeshMaterials.Add(Section.Material);
							UniqueSection = &CombinedSlotSections.Add(Section.MaterialName);
						}
						*UniqueSection += Section;
					});

					for (auto& Elem : CombinedSlotSections)
					{
						ResultSections.Add(MoveTemp(Elem.Value));
					}					
				}
				
				BuildMeshDescription({ ResultSections }, MeshDescriptions[0]);
				MeshHashes[0] = CalculateMeshHash(ResultSections);				
			}
			else
			{
				UAlterMeshComponent* Component = InActor->AlterMeshComponents[Step];
				for (int32 SectionIndex = 0; SectionIndex < Component->Sections.Num(); SectionIndex++)
				{
					FAlterMeshSection& Section = InActor->AlterMeshComponents[Step]->Sections[SectionIndex];

					if (bCombineMaterials)
					{
						NewMeshMaterials.AddUnique(Section.Material);
					}
					else
					{
						NewMeshMaterials.Add(Section.Material);
					}
				}

				BuildMeshDescription(InActor->AlterMeshComponents[Step]->Sections, MeshDescriptions[Step]);
				MeshHashes[Step] = CalculateMeshHash(InActor->AlterMeshComponents[Step]->Sections);
			}

			check(MeshDescriptions.Num() == MeshHashes.Num());
			check(MeshDescriptions.Num() == ConverterSteps.Num());

			AssetToolsModule.Get().CreateUniqueAssetName(PackageName, TEXT(""), PackageName, Name);

			// Then find/create it.
			UPackage* Package = CreatePackage(*PackageName);
			check(Package);

			// Create StaticMesh object
			UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, *Name, RF_Public | RF_Standalone);
			StaticMesh->InitResources();

			StaticMesh->SetLightingGuid(FGuid::NewGuid());

			// Add source to new StaticMesh
			FStaticMeshSourceModel& SrcModel = StaticMesh->AddSourceModel();
			SrcModel.BuildSettings = BuildSettings;

			StaticMesh->NaniteSettings = NaniteSettings;
			StaticMesh->CreateMeshDescription(0, MeshDescriptions[Step]);
			StaticMesh->CommitMeshDescription(0);

			if (UStaticMeshEditorSubsystem* StaticMeshEditorSubsystem = GEditor->GetEditorSubsystem<UStaticMeshEditorSubsystem>())
			{
				if (bBuildCollision)
				{
					// Simple Collision
					StaticMeshEditorSubsystem->BulkSetConvexDecompositionCollisions({StaticMesh},
						BuildCollisionSettings.NumHulls, BuildCollisionSettings.MaxHullVertices, BuildCollisionSettings.Resolution);
			

					// Custom Collision
					bool bHasCollisionParam = false;
					if (!InActor->Asset->Get()->CollisionSwitchParameter.IsEmpty())
					{
						FAlterMeshInputParams Params = InActor->InputParams;

						for (FAlterMeshBoolParam* Param : Params.GetTyped<FAlterMeshBoolParam>())
						{
							if (Param->BaseData.Name == InActor->Asset->Get()->CollisionSwitchParameter)
							{
								Param->Data.Value = true;
								bHasCollisionParam = true;
							}
						}
					
						if (bHasCollisionParam)
						{
							StaticMesh->GetBodySetup()->RemoveSimpleCollision();
							TArray<TSharedPtr<FAlterMeshPrimitive>> Meshes;
							InActor->BlenderInstance->RefreshSync(Params, InActor->Asset, Meshes);

							for (TSharedPtr<FAlterMeshPrimitive>& Mesh : Meshes)
							{
								TArray<FVector3f> Verts;
								for (auto& Section : Mesh->Sections)
								{	
									Verts.Append(Section.Vertices);								
								}
							
								for (FMatrix44f& Instance : Mesh->Sections[0].Instances)
								{
									TArray<FVector3f> InstanceVerts = Verts;
									for (FVector3f& Vert : InstanceVerts)
									{
										Vert = Instance.TransformPosition(Vert);
									}
								
									StaticMeshImportUtils::AddConvexGeomFromVertices(InstanceVerts, &StaticMesh->GetBodySetup()->AggGeom, TEXT(""));									
								}					
							}
						}
					}
				}
			}
			
			// Copy materials to new mesh
			for (auto* Material : NewMeshMaterials)
			{
				StaticMesh->GetStaticMaterials().Add(FStaticMaterial(Material));
			}

			//Set the Imported version before calling the build
			StaticMesh->ImportVersion = EImportStaticMeshVersion::LastVersion;

			// Build mesh from source
			StaticMesh->Build();
			StaticMesh->PostEditChange();

			// Metadata used to map the geometry to the generated asset
			// Initializes with UseExistingMesh when found
			StaticMesh->GetOutermost()->GetMetaData().SetValue(StaticMesh, ChecksumTagName, *FString::Printf(TEXT("%lu"), MeshHashes[Step]));

			// Notify asset registry of new asset
			FAssetRegistryModule::AssetCreated(StaticMesh);
			CreatedAssets.Add(StaticMesh);

			// Update action
			ConverterSteps[Step].AssetToUse = StaticMesh;
		}
	}
	
	GEditor->SyncBrowserToObjects(CreatedAssets);
}

void UAlterMeshConverterStaticMesh::PlaceAssets(AAlterMeshActor* InActor)
{
	for (int32 i = 0; i < ConverterSteps.Num(); i++)
	{
		if (ConverterSteps[i].Action != EAlterMeshConverterAction::DoNotImport)
		{
			if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(ConverterSteps[i].AssetToUse.LoadSynchronous()))
			{
				if (InActor->AlterMeshComponents[i]->Sections[0].Instances.Num() > 1)
				{
					UInstancedStaticMeshComponent* ISM = NewObject<UInstancedStaticMeshComponent>(InActor, ComponentClass);
					ISM->AttachToComponent(InActor->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
					ISM->RegisterComponent();
					InActor->AddInstanceComponent(ISM);
					ISM->SetStaticMesh(StaticMesh);

					for (const FMatrix44f& Instance : InActor->AlterMeshComponents[i]->Sections[0].Instances)
					{
						ISM->AddInstance(FTransform(FMatrix(Instance)));
					}

					SpawnedComponents.Add(ISM);
				}
				else
				{
					UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>(InActor);
					StaticMeshComponent->AttachToComponent(InActor->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
					StaticMeshComponent->RegisterComponent();
					InActor->AddInstanceComponent(StaticMeshComponent);
					StaticMeshComponent->SetStaticMesh(StaticMesh);
					StaticMeshComponent->SetRelativeTransform(
						bCombineMeshes ?
						FTransform::Identity
						: FTransform(FMatrix(InActor->AlterMeshComponents[i]->Sections[0].Instances[0])));
					
					SpawnedComponents.Add(StaticMeshComponent);
					
				}
			}

			// Blueprints
			if (UBlueprint* Blueprint = Cast<UBlueprint>(ConverterSteps[0].AssetToUse.LoadSynchronous()))
			{
				for (const FMatrix44f& Instance : InActor->AlterMeshComponents[i]->Sections[0].Instances)
				{
					FActorSpawnParameters Params;
					Params.bNoFail = true;
					Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					Params.OverrideLevel = InActor->GetLevel();
					AActor* BlueprintActor = InActor->GetWorld()->SpawnActor<AActor>(Blueprint->GeneratedClass, Params);
					BlueprintActor->AttachToComponent(InActor->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));

					FTransform InstanceTransform = FTransform(FMatrix(Instance));
					InstanceTransform.SetLocation(InstanceTransform.GetLocation() + InActor->GetActorLocation());
					BlueprintActor->SetActorTransform(InstanceTransform);
					SpawnedActors.Add(BlueprintActor);
				}
			}
		}
	}
}

uint32 UAlterMeshConverterStaticMesh::CalculateMeshHash(const TArray<FAlterMeshSection>& Sections) const
{
	uint32 Checksum = 0;
	for (const FAlterMeshSection& Section : Sections)
	{
		Checksum = FCrc::MemCrc32(Section.Vertices.GetData(), Section.Vertices.Num() * Section.Vertices.GetTypeSize(), Checksum);
		Checksum = FCrc::MemCrc32(Section.Normals.GetData(), Section.Normals.Num() * Section.Normals.GetTypeSize(), Checksum);
		Checksum = FCrc::MemCrc32(Section.Colors.GetData(), Section.Colors.Num() * Section.Colors.GetTypeSize(), Checksum);
		Checksum = FCrc::MemCrc32(Section.UV0.GetData(), Section.UV0.Num() * Section.UV0.GetTypeSize(), Checksum);
		Checksum = FCrc::MemCrc32(Section.UV1.GetData(), Section.UV1.Num() * Section.UV1.GetTypeSize(), Checksum);
		Checksum = FCrc::MemCrc32(Section.UV2.GetData(), Section.UV2.Num() * Section.UV2.GetTypeSize(), Checksum);
		Checksum = FCrc::MemCrc32(Section.UV3.GetData(), Section.UV3.Num() * Section.UV3.GetTypeSize(), Checksum);
	}

	return Checksum;
}

void UAlterMeshConverterStaticMesh::CleanUp()
{
	Super::CleanUp();

	for (UActorComponent* Component : SpawnedComponents)
	{
		Component->GetOwner()->RemoveInstanceComponent(Component);
		Component->DestroyComponent();
	}

	SpawnedComponents.Empty();
	
	for (AActor* Actor : SpawnedActors)
	{
		Actor->Destroy();
	}

	SpawnedActors.Empty();
}