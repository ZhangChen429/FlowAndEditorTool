// Copyright 2023 Aechmea

#pragma once
#include "Dom/JsonValue.h"
#include "Templates/SubclassOf.h"

#include "AlterMeshParamBase.generated.h"

class UMovieSceneTrack;
class UMovieScene;
struct FAlterMeshExport;

USTRUCT(BlueprintType)
struct ALTERMESH_API FAlterMeshParamId
{		
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="")
	FName NodeGroup;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="")
	FName Input;

	bool operator==(const FAlterMeshParamId& Other) const
	{
		return NodeGroup == Other.NodeGroup && Input == Other.Input;
	}
};

USTRUCT(BlueprintType)
struct ALTERMESH_API FAlterMeshParamBaseData
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="")
	FName NodeGroup;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="")
	FAlterMeshParamId Id;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="")
	FString Name;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="")
	FString Panel;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="")
	FString Tooltip;
};

USTRUCT(BlueprintType)
struct ALTERMESH_API FAlterMeshParamBase
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	FAlterMeshParamBaseData BaseData;
	
	virtual ~FAlterMeshParamBase() = default;

	// Write param to json
	virtual void SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const {};

	// Get param from json
	virtual void DeserializeJson(const TSharedPtr<FJsonValue>& Param) {};

	virtual bool UsedForType(FString Type) const { return false; } ;

	// Prepare data for export
	virtual void PreExport(FAlterMeshExport& Exporter) const {};
	
	// Export into buffer
	virtual void Export(FAlterMeshExport& Exporter) const {};

	/* Sequencer Interface */
	virtual TSubclassOf<UMovieSceneTrack> GetTrackClass() const { return nullptr; };
};