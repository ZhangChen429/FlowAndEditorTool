// Copyright 2023 Aechmea

#include "AlterMeshConverterFoliage.h"
#include "AlterMeshActor.h"
#include "AlterMeshComponent.h"
#include "Editor.h"
#include "FoliageEditUtility.h"
#include "FoliageType_Actor.h"
#include "InstancedFoliageActor.h"

UAlterMeshConverterFoliage::UAlterMeshConverterFoliage()
{
	// Dont create components
	ComponentClass = nullptr;
}

void UAlterMeshConverterFoliage::Convert(AAlterMeshActor* InActor)
{
	// Wouldn't make sense to combine all foliages
	bCombineMeshes = false;

	// Do conversion to static mesh before trying to place as foliage
	Super::Convert(InActor);
}

void UAlterMeshConverterFoliage::PlaceAssets(AAlterMeshActor* InActor)
{
#if !ALTERMESH_FREE
	Guid = FGuid::NewGuid();
	
	AInstancedFoliageActor* FoliageActor = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(InActor->GetWorld(), true);
	for (int32 i = 0; i < ConverterSteps.Num(); i++)
	{
		UObject* StaticMesh = Cast<UStaticMesh>(ConverterSteps[i].AssetToUse.LoadSynchronous());
		UObject* ActorClass = Cast<UClass>(ConverterSteps[i].AssetToUse.LoadSynchronous());
		if (StaticMesh || ActorClass)
		{
			UFoliageType* FoliageType = FoliageActor->GetLocalFoliageTypeForSource(StaticMesh);

			if (!FoliageType)
			{
				FoliageType = NewObject<UFoliageType>(FoliageActor, StaticMesh ? UFoliageType_InstancedStaticMesh::StaticClass() : UFoliageType_Actor::StaticClass());
				FoliageType->SetSource(StaticMesh ? StaticMesh : ActorClass);
				FFoliageEditUtility::SaveFoliageTypeObject(FoliageType);
			}

			FFoliageInfo* FoliageInfo = FoliageActor->FindInfo(FoliageType);

			if (!FoliageInfo)
			{
				FoliageActor->AddFoliageType(FoliageType, &FoliageInfo);
			}

			// Assumes all sections have same amount of instances
			for (const FMatrix44f& Instance : InActor->AlterMeshComponents[i]->Sections[0].Instances)
			{
				FMatrix44d RelativeMatrix = FMatrix44d(Instance) * InActor->GetTransform().ToMatrixWithScale();

				FFoliageInstance FoliageInstance;
				FoliageInstance.Location = RelativeMatrix.GetOrigin();
				FoliageInstance.Rotation = RelativeMatrix.Rotator();
				FoliageInstance.DrawScale3D = FVector3f(RelativeMatrix.GetScaleVector());
				FoliageInstance.ProceduralGuid = Guid; 

				FoliageInfo->AddInstance(FoliageType, FoliageInstance);
				FoliageInstances.FindOrAdd(StaticMesh).Indices.Add(FoliageInfo->Instances.Num()-1);
			}
		}
	}

	GEditor->SelectNone( true, true );
#endif
}

void UAlterMeshConverterFoliage::CleanUp()
{
	Super::CleanUp();

	AInstancedFoliageActor* FoliageActor = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(GetOuter()->GetWorld(), true);
	
	for (auto& Pair : FoliageActor->GetFoliageInfos())
	{
		FFoliageInfo& Info = *FoliageActor->FindInfo(Pair.Key);
		//FFoliageInfo& Info = *Pair.Value;
		
		TArray<int32> InstancesToRemove;
		for (int32 InstanceIdx = 0; InstanceIdx < Info.Instances.Num(); InstanceIdx++)
		{
			if (Info.Instances[InstanceIdx].ProceduralGuid == Guid)
			{
				InstancesToRemove.Add(InstanceIdx);
			}
		}

		if (InstancesToRemove.Num())
		{
			Info.RemoveInstances(InstancesToRemove, true);
		}
	}

	FFoliageInstanceBaseCache::CompactInstanceBaseCache(FoliageActor);

/*	for (auto& Instance : FoliageInstances)
	{
		UFoliageType* FoliageType = FoliageActor->GetLocalFoliageTypeForSource(Instance.Key);
		FFoliageInfo* FoliageInfo = FoliageActor->FindInfo(FoliageType);		
		FoliageInfo->RemoveInstances(FoliageActor, Instance.Value.Indices, true);
	}
	*/
	FoliageInstances.Empty();
}
