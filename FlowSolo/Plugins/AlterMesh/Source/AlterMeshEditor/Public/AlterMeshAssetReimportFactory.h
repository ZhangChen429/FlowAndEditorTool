// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshAssetFactory.h"
#include "EditorReimportHandler.h"
#include "Factories/Factory.h"
#include "AlterMeshAssetReimportFactory.generated.h"

UCLASS(hidecategories=Object, collapsecategories)
class UAlterMeshAssetReimportFactory : public UAlterMeshAssetFactory, public FReimportHandler
{
	GENERATED_BODY()
public:
	UAlterMeshAssetReimportFactory();

	//~ Begin FReimportHandler Interface
	virtual bool CanReimport( UObject* Obj, TArray<FString>& OutFilenames ) override;
	virtual void SetReimportPaths( UObject* Obj, const TArray<FString>& NewReimportPaths ) override;
	virtual EReimportResult::Type Reimport( UObject* Obj ) override;
	virtual int32 GetPriority() const override;
	//~ End FReimportHandler Interface

	//~ Being UFactory Interface
	virtual void CleanUp() override;
	//~ End UFactory Interface
};
