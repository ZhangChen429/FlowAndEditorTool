// Copyright 2023 Aechmea

#pragma once

#include "Dom/JsonObject.h"
#include "AlterMeshParamBase.h"
#include "AlterMeshBoolParam.generated.h"

USTRUCT(BlueprintType)
struct ALTERMESH_API FAlterMeshBoolParamData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category="")
	bool DefaultValue;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(SequencerTrackClass="/Script/AlterMesh.AlterMeshBoolTrack"), Category="")
	bool Value;
};

USTRUCT(BlueprintType)
struct ALTERMESH_API FAlterMeshBoolParam : public FAlterMeshParamBase
{
	GENERATED_BODY()
public:
	virtual void SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const override;
	virtual void DeserializeJson(const TSharedPtr<FJsonValue>& Param) override;
	virtual bool UsedForType(FString Type) const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	FAlterMeshBoolParamData Data;	

	virtual TSubclassOf<UMovieSceneTrack> GetTrackClass() const override;
};
