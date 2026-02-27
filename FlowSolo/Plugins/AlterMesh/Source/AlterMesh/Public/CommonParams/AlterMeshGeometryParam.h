// Copyright 2023 Aechmea

#pragma once

#include "AlterMeshParamBase.h"
#include "Dom/JsonValue.h"
#include "AlterMeshGeometryParam.generated.h"

class UAlterMeshGeometryBase;

USTRUCT(BlueprintType)
struct FAlterMeshGeometryParamData
{
	GENERATED_BODY()
public:
	
	// Hold a pointer to old param for cleanup
	UPROPERTY()
	TWeakObjectPtr<UAlterMeshGeometryBase> OldParam = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	UAlterMeshGeometryBase* InputType = nullptr;
};

USTRUCT(BlueprintType)
struct ALTERMESH_API FAlterMeshGeometryParam : public FAlterMeshParamBase
{
	GENERATED_BODY()
public:
	virtual void SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const override;
	virtual bool UsedForType(FString Type) const override;
	virtual void PreExport(FAlterMeshExport& Exporter) const override;
	virtual void Export(FAlterMeshExport& Exporter) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	FAlterMeshGeometryParamData Data;

};

