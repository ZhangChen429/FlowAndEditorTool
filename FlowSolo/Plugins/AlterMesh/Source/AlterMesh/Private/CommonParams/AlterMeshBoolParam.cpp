// Copyright 2023 Aechmea

#include "CommonParams/AlterMeshBoolParam.h"

#include "AlterMeshTracks.h"
#include "Dom/JsonObject.h"

void FAlterMeshBoolParam::SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const
{
	TSharedPtr<FJsonObject> ParamEntry = MakeShareable(new FJsonObject);
	ParamEntry->SetStringField(TEXT("NodeGroup"), BaseData.Id.NodeGroup.ToString());
	ParamEntry->SetStringField(TEXT("Id"), BaseData.Id.Input.ToString());
	ParamEntry->SetNumberField(TEXT("Value"), Data.Value);
	ParamEntry->SetStringField(TEXT("Type"), TEXT("BOOLEAN"));

	ParametersArray.Add(MakeShared<FJsonValueObject>(ParamEntry));
}

void FAlterMeshBoolParam::DeserializeJson(const TSharedPtr<FJsonValue>& Param)
{
	Data.DefaultValue = Param->AsObject()->GetBoolField(TEXT("DefaultValue"));
	Data.Value = Data.DefaultValue;
}

bool FAlterMeshBoolParam::UsedForType(FString Type) const
{
	return Type == TEXT("BOOLEAN");
}

TSubclassOf<UMovieSceneTrack> FAlterMeshBoolParam::GetTrackClass() const
{
	return UAlterMeshBoolTrack::StaticClass();
}
