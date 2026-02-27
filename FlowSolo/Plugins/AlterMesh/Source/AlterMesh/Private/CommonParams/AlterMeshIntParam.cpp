// Copyright 2023 Aechmea

#include "CommonParams/AlterMeshIntParam.h"

#include "AlterMeshTracks.h"
#include "Dom/JsonObject.h"

void FAlterMeshIntParam::SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const
{
	TSharedPtr<FJsonObject> ParamEntry = MakeShareable(new FJsonObject);
	ParamEntry->SetStringField(TEXT("NodeGroup"), BaseData.Id.NodeGroup.ToString());
	ParamEntry->SetStringField(TEXT("Id"), BaseData.Id.Input.ToString());
	ParamEntry->SetNumberField(TEXT("Value"), Data.Value);
	ParamEntry->SetStringField(TEXT("Type"), TEXT("INT"));

	ParametersArray.Add(MakeShared<FJsonValueObject>(ParamEntry));

}

void FAlterMeshIntParam::DeserializeJson(const TSharedPtr<FJsonValue>& Param)
{
	Data.DefaultValue = Param->AsObject()->GetNumberField(TEXT("DefaultValue"));
	Data.MinValue = Param->AsObject()->GetNumberField(TEXT("MinValue"));
	Data.MaxValue = Param->AsObject()->GetNumberField(TEXT("MaxValue"));
	Data.Value = FMath::Clamp(Data.DefaultValue, Data.MinValue, Data.MaxValue);
}

bool FAlterMeshIntParam::UsedForType(FString Type) const
{
	return Type == TEXT("INT");
}

TSubclassOf<UMovieSceneTrack> FAlterMeshIntParam::GetTrackClass() const
{
	return UAlterMeshIntegerTrack::StaticClass();
}
