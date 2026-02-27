// Copyright 2023 Aechmea

#include "CommonParams/AlterMeshStringParam.h"

#include "AlterMeshTracks.h"
#include "Dom/JsonObject.h"

void FAlterMeshStringParam::SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const
{
	TSharedPtr<FJsonObject> ParamEntry = MakeShareable(new FJsonObject);
	ParamEntry->SetStringField(TEXT("NodeGroup"), BaseData.Id.NodeGroup.ToString());
	ParamEntry->SetStringField(TEXT("Id"), BaseData.Id.Input.ToString());
	ParamEntry->SetStringField(TEXT("Value"), Data.Value);
	ParamEntry->SetStringField(TEXT("Type"), TEXT("STRING"));

	ParametersArray.Add(MakeShared<FJsonValueObject>(ParamEntry));
}

void FAlterMeshStringParam::DeserializeJson(const TSharedPtr<FJsonValue>& Param)
{	
	Data.DefaultValue = Param->AsObject()->GetStringField(TEXT("DefaultValue"));
	Data.Value = Data.DefaultValue;	
}

bool FAlterMeshStringParam::UsedForType(FString Type) const
{
	return Type == FString("STRING");
}

TSubclassOf<UMovieSceneTrack> FAlterMeshStringParam::GetTrackClass() const
{
	return UAlterMeshStringTrack::StaticClass();
}

