// Copyright 2023 Aechmea

#pragma once

#include "Dom/JsonValue.h"
#include "AlterMeshParamBase.h"
#include "AlterMeshMenuParam.generated.h"

USTRUCT(BlueprintType)
struct FAlterMeshMenuParamData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category="")
	int32 DefaultValue;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	FString Value;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	TArray<FString> Entries;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	TArray<int32> Values;
};

USTRUCT(BlueprintType)
struct ALTERMESH_API FAlterMeshMenuParam : public FAlterMeshParamBase
{
	GENERATED_BODY()
public:
	virtual void SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const override;
	virtual void DeserializeJson(const TSharedPtr<FJsonValue>& Param) override;
	virtual bool UsedForType(FString Type) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	FAlterMeshMenuParamData Data;
};

