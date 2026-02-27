// Copyright 2023 Aechmea

#pragma once

#include "AlterMeshParamBase.h"
#include "AlterMeshColorParam.generated.h"

class FJsonValue;

USTRUCT(BlueprintType)
struct FAlterMeshColorParamData
{
	GENERATED_BODY()
public:
	
	UPROPERTY(BlueprintReadOnly, Category="")
	FLinearColor DefaultValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(SequencerTrackClass="/Script/AlterMesh.AlterMeshColorTrack"), Category="")
	FLinearColor Value;
};

USTRUCT(BlueprintType)
struct ALTERMESH_API FAlterMeshColorParam : public FAlterMeshParamBase
{
	GENERATED_BODY()
public:
	virtual void SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const override;
	virtual void DeserializeJson(const TSharedPtr<FJsonValue>& Param) override;
	virtual bool UsedForType(FString Type) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	FAlterMeshColorParamData Data;

	virtual TSubclassOf<UMovieSceneTrack> GetTrackClass() const override;
};
