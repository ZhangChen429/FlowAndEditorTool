// Copyright 2023 Aechmea

#include "CommonParams/AlterMeshColorParam.h"

#include "AlterMeshTracks.h"
#include "Dom/JsonObject.h"


void FAlterMeshColorParam::SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const
{
	TSharedPtr<FJsonObject> ParamEntry = MakeShareable(new FJsonObject);
	ParamEntry->SetStringField(TEXT("NodeGroup"), BaseData.Id.NodeGroup.ToString());
	ParamEntry->SetStringField(TEXT("Id"), BaseData.Id.Input.ToString());

	TSharedPtr<FJsonObject> ColorObject = MakeShareable(new FJsonObject);
	ColorObject->SetNumberField(TEXT("R"), Data.Value.R);
	ColorObject->SetNumberField(TEXT("G"), Data.Value.G);
	ColorObject->SetNumberField(TEXT("B"), Data.Value.B);
	ColorObject->SetNumberField(TEXT("A"), Data.Value.A);

	TArray<TSharedPtr<FJsonValue>> Color;
	Color.Add(MakeShared<FJsonValueObject>(ColorObject));
	ParamEntry->SetArrayField(TEXT("Value"), Color);
	ParamEntry->SetStringField(TEXT("Type"), TEXT("RGBA"));

	ParametersArray.Add(MakeShared<FJsonValueObject>(ParamEntry));
}

void FAlterMeshColorParam::DeserializeJson(const TSharedPtr<FJsonValue>& Param)
{
	Data.DefaultValue = FLinearColor(Param->AsObject()->GetArrayField(TEXT("DefaultValue"))[0]->AsNumber(),
						Param->AsObject()->GetArrayField(TEXT("DefaultValue"))[1]->AsNumber(),
						Param->AsObject()->GetArrayField(TEXT("DefaultValue"))[1]->AsNumber(),
						Param->AsObject()->GetArrayField(TEXT("DefaultValue"))[2]->AsNumber());
	Data.Value = Data.DefaultValue;
}

bool FAlterMeshColorParam::UsedForType(FString Type) const
{
	return Type == TEXT("RGBA");
}

TSubclassOf<UMovieSceneTrack> FAlterMeshColorParam::GetTrackClass() const
{
	return UAlterMeshColorTrack::StaticClass();
}
