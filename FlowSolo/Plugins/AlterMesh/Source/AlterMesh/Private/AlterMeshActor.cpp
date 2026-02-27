// Copyright 2023 Aechmea

#include "AlterMeshActor.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Components/SplineComponent.h"
#include "AlterMeshAsset.h"
#include "AlterMeshRendering/Public/AlterMeshComponent.h"
#include "AlterMeshLibrary.h"
#include "AlterMeshSettings.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "CommonParams/AlterMeshCollectionParam.h"
#include "CommonParams/AlterMeshIntParam.h"
#include "Components/BillboardComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GeometryParams/AlterMeshGeometryBase.h"
#include "Interfaces/IPluginManager.h"
#include "UObject/ObjectSaveContext.h"
#include "Engine/StaticMesh.h"
#include "Misc/Paths.h"
#include "Engine/Blueprint.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

AAlterMeshActor::AAlterMeshActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(FName("SceneRootComponent"));
	RootSceneComponent->SetMobility(EComponentMobility::Static);
	RootComponent = RootSceneComponent;

#if WITH_EDITORONLY_DATA
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(FName("AlterMeshBillboard"));
	BillboardComponent->SetupAttachment(RootComponent);

	BillboardComponent->SetVisibility(false);
	BillboardComponent->SetHiddenInGame(true);
	BillboardComponent->SetEditorScale(0.5f);
#endif
}

void AAlterMeshActor::SetPreview(bool bInPreview)
{
	bPreview = bInPreview;
	
#if WITH_EDITORONLY_DATA
	const UAlterMeshEditorSettings* Settings = GetDefault<UAlterMeshEditorSettings>();
	BillboardComponent->SetSprite(Settings->BillboardTexture.LoadSynchronous());
	BillboardComponent->SetVisibility(bPreview);
#endif
	
	// Clear old components whether it got baked or became preview
	{
		for (UActorComponent* Component : AssetInstanceComponents)
		{
			Component->DestroyComponent();
		}
		AssetInstanceComponents.Empty();
	
		for (UAlterMeshComponent* Component : AlterMeshComponents)
		{
			Component->DestroyComponent();
		}
		AlterMeshComponents.Empty();
	}
	
	if (bPreview)
	{		
		OnBecomePreview.Broadcast();		
	}

	if (!bPreview)
	{
		const auto* AMSettings = GetDefault<UAlterMeshSettings>(); 
		if (AMSettings->bLockOnConvert)
		{
			bLocked = true;
		}
	}
}

void AAlterMeshActor::StopInstance()
{
	if (BlenderInstance)
	{
		BlenderInstance->CleanupProcess();
	}
}

void AAlterMeshActor::Convert()
{
#if WITH_EDITOR
	if (OnConvertClicked.IsBound())
	{
		OnConvertClicked.Execute();
	}
#endif
}

void AAlterMeshActor::InitializeParamsFromAsset()
{
	if (Asset)
	{
		if (auto* AssetInterface = Cast<UAlterMeshAssetInterface>(Asset))
		{
			InputParams = AssetInterface->GetParams();
		}
		
		// We dont want same Instanced class from the asset
		// else we get an editor asset that points to a class that lives in world
		// but still want to copy its properties
		InputParams.Copy(Asset->GetParams(), this);
	}
	else
	{
		// Clear so old params don't show up
		InputParams.Clear();
	}
}

void AAlterMeshActor::CleanupGeometryParam(FAlterMeshGeometryParam& GeometryParam)
{
	if (GeometryParam.Data.OldParam.IsValid())
	{
		GeometryParam.Data.OldParam->Cleanup();
		GeometryParam.Data.OldParam = nullptr;
	}
}

void AAlterMeshActor::InitializeGeometryParam(FAlterMeshGeometryParam& GeometryParam)
{
	if (GeometryParam.Data.InputType)
	{
		GeometryParam.Data.InputType->Initialize(this);
		GeometryParam.Data.OldParam = GeometryParam.Data.InputType;
	}

#if WITH_EDITOR
	if (GEditor)
	{
		// Refreshes components visualizers
		GEditor->NoteSelectionChange();
	}
#endif
}

void AAlterMeshActor::CleanupAllGeometryParams()
{
	for (FAlterMeshGeometryParam* Param : InputParams.GetTyped<FAlterMeshGeometryParam>())
	{
		CleanupGeometryParam(*Param);
	}
	
	for (FAlterMeshCollectionParam* Param : InputParams.GetTyped<FAlterMeshCollectionParam>())
	{
		for (FAlterMeshGeometryParam& GeometryParam : Param->Data.Geometries)
		{
			CleanupGeometryParam(GeometryParam);	
		}
	}
}

void AAlterMeshActor::InitializeAllGeometryParams()
{
	// No permanent or transient objects
	if (!HasAnyFlags(RF_Transient | RF_Standalone))
	{
		for (FAlterMeshGeometryParam* Param : InputParams.GetTyped<FAlterMeshGeometryParam>())
		{
			InitializeGeometryParam(*Param);
		}	

		for (FAlterMeshCollectionParam* Param : InputParams.GetTyped<FAlterMeshCollectionParam>())
		{
			for (FAlterMeshGeometryParam& GeometryParam : Param->Data.Geometries)
			{
				InitializeGeometryParam(GeometryParam);	
			}
		}
	}
}

void AAlterMeshActor::RefreshMaterials()
{
	for (const FAlterMeshMaterial& OverrideMaterial : OverrideMaterials)
	{
		if (OverrideMaterial.Material)
		{
			for (UAlterMeshComponent* AlterMeshComponent : AlterMeshComponents)
			{
				if (AlterMeshComponent)
				{
					AlterMeshComponent->SetMaterialByName(OverrideMaterial.SlotName, OverrideMaterial.Material);
				}
			}
		}
	}
}

#if WITH_EDITOR
void AAlterMeshActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty
		&& PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AAlterMeshActor, InputParams))
	{

	}

	if (PropertyChangedEvent.Property
		&& PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AAlterMeshActor, Asset))
	{
		// AlterMeshAsset was changed, just refresh everything
		CleanupAllGeometryParams();
		InitializeParamsFromAsset();
		InitializeAllGeometryParams();

		if (BlenderInstance)
		{
			BlenderInstance->CleanupProcess();
		}

		RefreshAsync();
	}

	if (PropertyChangedEvent.MemberProperty
		&& PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AAlterMeshActor, OverrideMaterials))
	{
		RefreshMaterials();
	}
}

void AAlterMeshActor::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	// Recreate geometry exporters if geometry class was changed
	if (PropertyChangedEvent.Property
		&& PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(FAlterMeshGeometryParamData, InputType))
	{
		const auto PropertyNode = PropertyChangedEvent.PropertyChain.GetActiveNode();
		const auto DataPropertyNode = PropertyNode->GetPrevNode();
		const auto OuterParamNode = DataPropertyNode->GetPrevNode();
		const FName OuterPropertyName = OuterParamNode->GetValue()->GetFName();

		// Geometry params
		if (OuterPropertyName == GET_MEMBER_NAME_CHECKED(FAlterMeshInputParams, Params))
		{
			const int32 MeshIndex = PropertyChangedEvent.GetArrayIndex(OuterPropertyName.ToString());

			if (InputParams.Params.IsValidIndex(MeshIndex))
			{
				CleanupGeometryParam(InputParams.Params[MeshIndex].GetMutable<FAlterMeshGeometryParam>());
				InitializeGeometryParam(InputParams.Params[MeshIndex].GetMutable<FAlterMeshGeometryParam>());
			}
		}

		// Collection params
		if (OuterPropertyName == GET_MEMBER_NAME_CHECKED(FAlterMeshCollectionParamData, Geometries))
		{
			const auto CollectionPropertyNode = OuterParamNode->GetPrevNode();
			const FName CollectionPropertyName = CollectionPropertyNode->GetValue()->GetFName();
			const int32 MeshIndex = PropertyChangedEvent.GetArrayIndex(OuterPropertyName.ToString());
			const int32 CollectionIndex = PropertyChangedEvent.GetArrayIndex(CollectionPropertyName.ToString());

			if (InputParams.Params.IsValidIndex(CollectionIndex)
				&& InputParams.Params[CollectionIndex].Get<FAlterMeshCollectionParam>().Data.Geometries.IsValidIndex(MeshIndex))
			{
				CleanupGeometryParam(InputParams.Params[CollectionIndex].GetMutable<FAlterMeshCollectionParam>().Data.Geometries[MeshIndex]);
				InitializeGeometryParam(InputParams.Params[CollectionIndex].GetMutable<FAlterMeshCollectionParam>().Data.Geometries[MeshIndex]);
			}
		}
	}
}
#endif

void AAlterMeshActor::PostLoad()
{
	Super::PostLoad();

#if WITH_EDITOR
	FEditorDelegates::PreSaveWorldWithContext.AddWeakLambda(this, [this](UWorld* World, FObjectPreSaveContext Context){ Convert(); });
	FEditorDelegates::PreSaveExternalActors.AddWeakLambda(this, [this](UWorld* World){ Convert(); });
#endif
}

void AAlterMeshActor::PostActorCreated()
{
	Super::PostActorCreated();

#if WITH_EDITOR
	FEditorDelegates::PreSaveWorldWithContext.AddWeakLambda(this, [this](UWorld* World, FObjectPreSaveContext Context){ Convert(); });
	FEditorDelegates::PreSaveExternalActors.AddWeakLambda(this, [this](UWorld* World){ Convert(); });
#endif
}

void AAlterMeshActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAnyFlags(RF_ArchetypeObject | RF_ClassDefaultObject))
		return;

	InstanceState = BlenderInstance ? BlenderInstance->State : EAlterMeshInstanceState::Closed;

	if (bExportEveryFrame)
	{
		RefreshSync();
	}
}

void AAlterMeshActor::Destroyed()
{
	if (BlenderInstance)
	{
		BlenderInstance->CleanupProcess();
		BlenderInstance = nullptr;
	}

	Super::Destroyed();
}

#if WITH_EDITOR
void AAlterMeshActor::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	if (!bFinished && bExportOnMove)
	{
		RefreshAsync();
	}

	if (bFinished && bExportOnFinishMove)
	{
		RefreshAsync();
	}
}

void AAlterMeshActor::PostEditUndo()
{
	Super::PostEditUndo();
	
	if (GEditor)
	{
		GEditor->SelectNone( true, true );
		GEditor->SelectActor( this, true, true );	
	}
}

#endif

void AAlterMeshActor::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	if (bPreview)
	{
		RefreshSync();
	}
#endif
}


void AAlterMeshActor::CreateComponents(const TSharedPtr<FAlterMeshPrimitive>& Mesh, bool bInstance)
{
	// Create one new component per mesh, with possibly multiple sections/materials
	UAlterMeshComponent* NewComponent = NewObject<UAlterMeshComponent>(this);

	// Don't display instances, it will be placed as unreal assets instead
	NewComponent->SetVisibility(!bInstance);
	
	for (FAlterMeshSection& Section : Mesh->Sections)
	{
		bool bContainsMaterial = OverrideMaterials.ContainsByPredicate([&Section](const FAlterMeshMaterial& Other)
		{
			return Section.MaterialName == Other.SlotName;
		});

		if (!bContainsMaterial)
		{
			FAlterMeshMaterial Override;
			Override.Material = Section.Material;
			Override.SlotName = Section.MaterialName;
			OverrideMaterials.Add(Override);
		}

		TriangleCount += Section.Instances.Num() * (Section.Indices.Num() / 3);
		NewComponent->Sections.Add(MoveTemp(Section));
	}

	NewComponent->AttachToComponent(RootSceneComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	AlterMeshComponents.Add(NewComponent);
	NewComponent->OnMaterialChanged.AddUObject(this, &AAlterMeshActor::OnComponentMaterialChanged);
	NewComponent->RegisterComponent();
}

FOnRefreshDelegate* AAlterMeshActor::GetOnRefreshDelegate()
{
	if (!BlenderInstance || !BlenderInstance->IsValid())
	{
		StartInstance();
	}

	if (BlenderInstance && BlenderInstance->IsValid())
	{
		return &BlenderInstance->OnRefreshDelegate;
	}
	
	return nullptr;
}

void AAlterMeshActor::StartInstance()
{
	if (Asset)
	{
		const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("AlterMesh"))->GetBaseDir();
		const FString ScriptPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(PluginDir, FString("\\Source\\ThirdParty\\__init__.py")));
		const FString FilePath = UAlterMeshLibrary::ConvertFilenameToFull(Asset->Get()->Filename.FilePath);

		BlenderInstance = NewObject<UAlterMeshInstance>(this);
		BlenderInstance->Initialize(FilePath, ScriptPath);
	}
}

void AAlterMeshActor::RefreshSync()
{
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient))
	{
		return;
	}

	if (bLocked)
	{
		UE_LOG(LogAlterMeshExport, Log, TEXT("Actor %s tried to refresh but bLocked is enabled"), *GetName());
		return;
	}
	
	if (!BlenderInstance || !BlenderInstance->IsValid())
	{
		StartInstance();
	}

	if (BlenderInstance && BlenderInstance->IsValid())
	{
		TArray<TSharedPtr<FAlterMeshPrimitive>> Meshes;
		BlenderInstance->RefreshSync(InputParams, Asset, Meshes);
		OnImport(Meshes);
	}
}

void AAlterMeshActor::RefreshAsync()
{
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient))
	{
		return;
	}
	
	if (bLocked)
	{
		UE_LOG(LogAlterMeshExport, Log, TEXT("Actor %s tried to refresh but bLocked is enabled"), *GetName());
		return;
	}
	
	if (!BlenderInstance || !BlenderInstance->IsValid())
	{
		StartInstance();
	}

	if (BlenderInstance && BlenderInstance->IsValid())
	{
		BlenderInstance->RefreshAsync(InputParams, Asset, FImportMeshCallback::CreateUObject(this, &AAlterMeshActor::OnImport));
	}
}

void AAlterMeshActor::OnImport(TArray<TSharedPtr<FAlterMeshPrimitive>> Meshes)
{
	const double ImportTimestamp = FPlatformTime::Seconds();

	// Clear old components
	TArray<UAlterMeshComponent*> Components;
	GetComponents<UAlterMeshComponent>(Components);
	for (UAlterMeshComponent* Component : Components)
	{
		Component->DestroyComponent();
	}
	
	AlterMeshComponents.Empty();
	
	TriangleCount = 0;
	SetPreview(true);
	
	for (const TSharedPtr<FAlterMeshPrimitive>& Mesh : Meshes)
	{
		bool bHasUnrealEquivalentAsset = Mesh->bIsInstance && Mesh->AssetPath != NAME_None;
	
		if (bHasUnrealEquivalentAsset)
		{
			// Place instances of existing unreal assets
			PlaceInstances(FSoftObjectPath(Mesh->AssetPath.ToString()).TryLoad(), Mesh->Sections[0].Instances);
		}

		// Create components even if its a instance, converters may need it
		CreateComponents(Mesh, bHasUnrealEquivalentAsset);
	}

	RefreshMaterials();

	ImportTime = FPlatformTime::Seconds() - ImportTimestamp;
	FEditorScriptExecutionGuard ScriptGuard;
	OnImportDelegate.Broadcast();
	OnImportEvent();
}

void AAlterMeshActor::OnComponentMaterialChanged(const UAlterMeshComponent* Component, int32 ElementIndex, UMaterialInterface* Material)
{
	// In case this material was not set from the override materials panel, update overrides
	FName SlotName = Component->GetMaterialSlotNames()[ElementIndex];
	FAlterMeshMaterial* Override = OverrideMaterials.FindByPredicate([SlotName](const FAlterMeshMaterial& Material)
	{
		 return Material.SlotName == SlotName;
	});

	if (Override)
	{
		Override->Material = Material;
	}
}

void AAlterMeshActor::PlaceInstances(UObject* InObject, const TArray<FMatrix44f>& InInstances)
{
	if (InObject)
	{
		for (const FMatrix44f& Instance : InInstances)
		{
			UBlueprint* Blueprint = Cast<UBlueprint>(InObject);
			UStaticMesh* StaticMesh = Cast<UStaticMesh>(InObject);

			FTransform InstanceTransform = FTransform(FMatrix(Instance));
			
			if (Blueprint && Blueprint->GeneratedClass && Blueprint->GeneratedClass->IsChildOf(AActor::StaticClass()))
			{
				UChildActorComponent* CAC = NewObject<UChildActorComponent>(this);
				CAC->SetChildActorClass(Blueprint->GeneratedClass.Get());
				CAC->AttachToComponent(RootSceneComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
				CAC->RegisterComponent();
				CAC->SetRelativeTransform(InstanceTransform);
				AssetInstanceComponents.Add(CAC);
			}
			else if (StaticMesh)
			{
				UInstancedStaticMeshComponent* ISM = nullptr;
				
				for (UActorComponent* Component : AssetInstanceComponents)
				{
					if (Cast<UInstancedStaticMeshComponent>(Component) && Cast<UInstancedStaticMeshComponent>(Component)->GetStaticMesh() == InObject)
					{
						 ISM = Cast<UInstancedStaticMeshComponent>(Component);		
					}
				}

				if (!ISM)
				{
					ISM = NewObject<UInstancedStaticMeshComponent>(this);
					ISM->AttachToComponent(RootSceneComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
					ISM->RegisterComponent();
					ISM->SetStaticMesh(StaticMesh);
					AssetInstanceComponents.Add(ISM);
				}

				ISM->AddInstance(InstanceTransform);
			}
		}
	}
}
