// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"

#include "ActorFactories/ActorFactory.h"

#include "AlterMeshActorFactory.generated.h"

UCLASS(MinimalAPI, config=Editor)
class UAlterMeshActorFactory : public UActorFactory
{
	GENERATED_BODY()
public:
	UAlterMeshActorFactory();
	//~ Begin UActorFactory Interface
	virtual bool CanCreateActorFrom( const FAssetData& AssetData, FText& OutErrorMsg ) override;
	virtual void PostSpawnActor( UObject* Asset, AActor* NewActor) override;
	virtual UObject* GetAssetFromActorInstance(AActor* ActorInstance) override;
	//~ End UActorFactory Interface
};
