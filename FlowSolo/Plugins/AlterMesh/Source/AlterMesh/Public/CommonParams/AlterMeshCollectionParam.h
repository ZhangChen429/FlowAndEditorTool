// Copyright 2023 Aechmea

#pragma once

#include "AlterMeshGeometryParam.h"
#include "AlterMeshCollectionParam.generated.h"

USTRUCT(BlueprintType)
struct FAlterMeshCollectionParamData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Collection=true), Category="")
	TArray<FAlterMeshGeometryParam> Geometries;
};

USTRUCT(BlueprintType)
struct ALTERMESH_API FAlterMeshCollectionParam : public FAlterMeshParamBase
{
	GENERATED_BODY()
public:
	virtual void SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const override;
	virtual void DeserializeJson(const TSharedPtr<FJsonValue>& Param) override;
	virtual bool UsedForType(FString Type) const override;
	virtual void Export(FAlterMeshExport& Exporter) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	FAlterMeshCollectionParamData Data;
};
