// Copyright 2023 Aechmea

#include "CommonParams/AlterMeshFloatParam.h"

#include "AlterMeshTracks.h"
#include "MovieScene.h"
#include "Dom/JsonObject.h"

void FAlterMeshFloatParam::SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const
{
	TSharedPtr<FJsonObject> ParamEntry = MakeShareable(new FJsonObject);
	ParamEntry->SetStringField(TEXT("NodeGroup"), BaseData.Id.NodeGroup.ToString());
	ParamEntry->SetStringField(TEXT("Id"), BaseData.Id.Input.ToString());
	ParamEntry->SetNumberField(TEXT("Value"), Data.Value);
	ParamEntry->SetStringField(TEXT("Type"), TEXT("VALUE"));

	ParametersArray.Add(MakeShared<FJsonValueObject>(ParamEntry));
}

void FAlterMeshFloatParam::DeserializeJson(const TSharedPtr<FJsonValue>& Param)
{	
	Data.DefaultValue = Param->AsObject()->GetNumberField(TEXT("DefaultValue"));
	Data.MinValue = Param->AsObject()->GetNumberField(TEXT("MinValue"));
	Data.MaxValue = Param->AsObject()->GetNumberField(TEXT("MaxValue"));
	Data.Value = FMath::Clamp(Data.DefaultValue, Data.MinValue, Data.MaxValue);
}

TSubclassOf<UMovieSceneTrack> FAlterMeshFloatParam::GetTrackClass() const
{
	return UAlterMeshFloatTrack::StaticClass();
}

