// Copyright 2023 Aechmea

#include "AlterMeshActorFactory.h"

#include "AlterMeshActor.h"
#include "AlterMeshAsset.h"
#include "AlterMeshConverter.h"

UAlterMeshActorFactory::UAlterMeshActorFactory()
{
	NewActorClass = AAlterMeshActor::StaticClass();
	//bUseSurfaceOrientation = true;
}

bool UAlterMeshActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if ( !AssetData.IsValid()		
		|| !AssetData.GetClass()->IsChildOf( UAlterMeshAssetInterface::StaticClass() ) )
	{
		OutErrorMsg = NSLOCTEXT("CanCreateActor", "NoAlterMeshAsset", "Invalid AlterMeshAsset.");
		return false;
	}

	return true;
}

void UAlterMeshActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	UAlterMeshAsset* AlterMeshAsset = Cast<UAlterMeshAssetInterface>(Asset)->Get();

	UE_LOG(LogActorFactory, Log, TEXT("Actor Factory created %s"), *AlterMeshAsset->GetName());

	AAlterMeshActor* AlterMeshActor = CastChecked<AAlterMeshActor>( NewActor );
	AlterMeshActor->Asset = Cast<UAlterMeshAssetInterface>(Asset);
	AlterMeshActor->CleanupAllGeometryParams();
	AlterMeshActor->InitializeParamsFromAsset();
	AlterMeshActor->InitializeAllGeometryParams();

	static bool bConverterClassLogged = false;
	if (!bConverterClassLogged && !AlterMeshAsset->ConverterClass)
	{
		bConverterClassLogged = true;
		UE_LOG(LogTemp, Warning, TEXT("AlterMesh: Converter Class not set in the asset defaults, converter options won't be shown"))
	}
	
	if (!AlterMeshActor->ConverterInstance && AlterMeshAsset->ConverterClass)
	{
		AlterMeshActor->ConverterInstance = NewObject<UAlterMeshConverterBase>(AlterMeshActor, AlterMeshAsset->ConverterClass.LoadSynchronous());
		AlterMeshActor->OnBecomePreview.AddDynamic(Cast<UAlterMeshConverterBase>(AlterMeshActor->ConverterInstance), &UAlterMeshConverterBase::CleanUp);
	}
}

UObject* UAlterMeshActorFactory::GetAssetFromActorInstance(AActor* ActorInstance)
{
	return Cast<AAlterMeshActor>(ActorInstance)->Asset;
}
