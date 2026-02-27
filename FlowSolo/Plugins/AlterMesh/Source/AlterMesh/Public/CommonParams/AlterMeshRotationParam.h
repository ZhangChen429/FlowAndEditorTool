// Copyright 2023 Aechmea

#pragma once

#include "Dom/JsonValue.h"
#include "AlterMeshParamBase.h"
#include "AlterMeshRotationParam.generated.h"

USTRUCT(BlueprintType)
struct FAlterMeshRotationParamData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="")
	FRotator DefaultValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, meta=(SequencerTrackClass="/Script/AlterMesh.AlterMeshRotationTrack"), Category="")
	FRotator Value;

	/*
	 * Converts the coordinate space of this param by swapping X and Y
	 * and mirroring the Z axis.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	bool bConvertCoordinateSpace = true;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="")
	float MinValue = -MAX_flt;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="")
	float MaxValue = MAX_flt;	
};


USTRUCT(BlueprintType)
struct ALTERMESH_API FAlterMeshRotationParam : public FAlterMeshParamBase
{
	GENERATED_BODY()
public:
	virtual void SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const override;
	virtual void DeserializeJson(const TSharedPtr<FJsonValue>& Param) override;
	virtual bool UsedForType(FString Type) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	FAlterMeshRotationParamData Data;
	
	virtual TSubclassOf<UMovieSceneTrack> GetTrackClass() const override;
};