// Copyright 2023 Aechmea

#pragma once

#include "Dom/JsonValue.h"
#include "AlterMeshParamBase.h"
#include "AlterMeshStringParam.generated.h"

USTRUCT(BlueprintType)
struct FAlterMeshStringParamData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category="")
	FString DefaultValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(SequencerTrackClass="/Script/AlterMesh.AlterMeshStringTrack"), Category="")
	FString Value;
};

USTRUCT(BlueprintType)
struct ALTERMESH_API FAlterMeshStringParam : public FAlterMeshParamBase
{
	GENERATED_BODY()
public:
	virtual void SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const override;
	virtual void DeserializeJson(const TSharedPtr<FJsonValue>& Param) override;
	virtual bool UsedForType(FString Type) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	FAlterMeshStringParamData Data;

	virtual TSubclassOf<UMovieSceneTrack> GetTrackClass() const override;
};

