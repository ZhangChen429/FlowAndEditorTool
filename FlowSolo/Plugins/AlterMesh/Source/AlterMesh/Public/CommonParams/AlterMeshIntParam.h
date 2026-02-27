// Copyright 2023 Aechmea

#pragma once

#include "AlterMeshParamBase.h"
#include "Dom/JsonValue.h"
#include "AlterMeshIntParam.generated.h"

USTRUCT(BlueprintType)
struct FAlterMeshIntParamData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category="")
	int32 DefaultValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(SequencerTrackClass="/Script/AlterMesh.AlterMeshIntegerTrack"), Category="")
	int32 Value;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="")
	int32 MinValue = -MAX_int32;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="")
	int32 MaxValue = MAX_int32;
};

USTRUCT(BlueprintType)
struct ALTERMESH_API FAlterMeshIntParam : public FAlterMeshParamBase
{
	GENERATED_BODY()
public:

	virtual void SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const override;
	virtual void DeserializeJson(const TSharedPtr<FJsonValue>& Param) override;
	virtual bool UsedForType(FString Type) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	FAlterMeshIntParamData Data;

	virtual TSubclassOf<UMovieSceneTrack> GetTrackClass() const override;
};
