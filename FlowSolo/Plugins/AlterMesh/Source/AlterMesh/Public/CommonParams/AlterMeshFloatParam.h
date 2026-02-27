// Copyright 2023 Aechmea

#pragma once

#include "AlterMeshParamBase.h"
#include "Dom/JsonValue.h"
#include "AlterMeshFloatParam.generated.h"

USTRUCT(BlueprintType)
struct FAlterMeshFloatParamData
{
	GENERATED_BODY()
public:
	
	UPROPERTY(BlueprintReadOnly, Category="")
	float DefaultValue;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, meta=(SequencerTrackClass="/Script/AlterMesh.AlterMeshFloatTrack"), Category="")
	float Value;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="")
	float MinValue = -MAX_flt;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="")
	float MaxValue = MAX_flt;
};

USTRUCT(BlueprintType)
struct ALTERMESH_API FAlterMeshFloatParam : public FAlterMeshParamBase
{
	GENERATED_BODY()
public:

	virtual void SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const override;
	virtual void DeserializeJson(const TSharedPtr<FJsonValue>& Param) override;
	virtual bool UsedForType(FString Type) const override { return Type == TEXT("VALUE"); }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	FAlterMeshFloatParamData Data;

	/* Sequencer Interface */
	virtual TSubclassOf<UMovieSceneTrack> GetTrackClass() const override;
};
