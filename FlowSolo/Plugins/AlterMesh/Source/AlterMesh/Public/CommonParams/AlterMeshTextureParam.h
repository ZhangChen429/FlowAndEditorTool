// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonValue.h"
#include "AlterMeshParamBase.h"
#include "AlterMeshTextureParam.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FAlterMeshTextureParamData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowedClasses="/Script/Engine.Texture, /Script/Engine.MaterialInterface"), Category="")
	UObject* Value = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	int32 MipBias;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="")
	FVector2D Resolution = FVector2D(64,64);
};

struct FAlterMeshExport;
USTRUCT(BlueprintType)
struct ALTERMESH_API FAlterMeshTextureParam : public FAlterMeshParamBase
{
	GENERATED_BODY()
public:
	virtual void SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const override;
	virtual void DeserializeJson(const TSharedPtr<FJsonValue>& Param) override;
	virtual bool UsedForType(FString Type) const override;

	virtual void Export(FAlterMeshExport& Exporter) const override;

	template<typename T>
	void ExportTexture(FAlterMeshExport& Exporter, UTexture2D* Texture) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	FAlterMeshTextureParamData Data;
};
