// Copyright 2023 Aechmea

#pragma once

#include "Dom/JsonValue.h"
#include "AlterMeshParamBase.h"
#include "AlterMeshVectorParam.generated.h"

USTRUCT(BlueprintType)
struct FAlterMeshVectorParamData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="")
	FVector DefaultValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, meta=(SequencerTrackClass="/Script/AlterMesh.AlterMeshVectorTrack"), Category="")
	FVector Value;

	/*
	 * Converts the coordinate space of this param by swapping and mirroring X and Y axis
	 *
	 * This should be turned on if you want to respect UE4 coordinate system
	 * but may break any math that doesnt expect negative values (use absolute values)
	 *
	 * Generally this should be enabled for "Position" and "Direction" parameters
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	bool bConvertCoordinateSpace = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="")
	float MinValue = -MAX_flt;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="")
	float MaxValue = MAX_flt;	
};


USTRUCT(BlueprintType)
struct ALTERMESH_API FAlterMeshVectorParam : public FAlterMeshParamBase
{
	GENERATED_BODY()
public:
	virtual void SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const override;
	virtual void DeserializeJson(const TSharedPtr<FJsonValue>& Param) override;
	virtual bool UsedForType(FString Type) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	FAlterMeshVectorParamData Data;
	
	virtual TSubclassOf<UMovieSceneTrack> GetTrackClass() const override;
};