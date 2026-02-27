// Copyright 2023 Aechmea

#include "CommonParams/AlterMeshRotationParam.h"

#include "AlterMeshTracks.h"
#include "Dom/JsonObject.h"

void FAlterMeshRotationParam::SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const
{
	TSharedPtr<FJsonObject> ParamEntry = MakeShareable(new FJsonObject);
	ParamEntry->SetStringField(TEXT("NodeGroup"), BaseData.Id.NodeGroup.ToString());
	ParamEntry->SetStringField(TEXT("Id"), BaseData.Id.Input.ToString());

	TSharedPtr<FJsonObject> RotationObject = MakeShareable(new FJsonObject);

	// Remap for coordinate system
	if (Data.bConvertCoordinateSpace)
	{
		RotationObject->SetNumberField(TEXT("X"), -Data.Value.Pitch);
		RotationObject->SetNumberField(TEXT("Y"), -Data.Value.Roll);
		RotationObject->SetNumberField(TEXT("Z"), -Data.Value.Yaw);
	}
	else
	{
		RotationObject->SetNumberField(TEXT("X"), Data.Value.Roll);
		RotationObject->SetNumberField(TEXT("Y"), Data.Value.Pitch);
		RotationObject->SetNumberField(TEXT("Z"), Data.Value.Yaw);
	}

	TArray<TSharedPtr<FJsonValue>> Rotation;
	Rotation.Add(MakeShared<FJsonValueObject>(RotationObject));

	ParamEntry->SetArrayField(TEXT("Value"), Rotation);
	ParamEntry->SetStringField(TEXT("Type"), TEXT("ROTATION"));
	
	ParametersArray.Add(MakeShared<FJsonValueObject>(ParamEntry));

}

void FAlterMeshRotationParam::DeserializeJson(const TSharedPtr<FJsonValue>& Param)
{
	Data.DefaultValue = FRotator(Param->AsObject()->GetArrayField(TEXT("DefaultValue"))[1]->AsNumber(),
			Param->AsObject()->GetArrayField(TEXT("DefaultValue"))[2]->AsNumber(),
			Param->AsObject()->GetArrayField(TEXT("DefaultValue"))[0]->AsNumber());
	//Data.MinValue = Param->AsObject()->GetNumberField(TEXT("MinValue"));
	//Data.MaxValue = Param->AsObject()->GetNumberField(TEXT("MaxValue"));
	Data.Value.Roll = FMath::Clamp(Data.DefaultValue.Roll, Data.MinValue, Data.MaxValue);
	Data.Value.Pitch = FMath::Clamp(Data.DefaultValue.Pitch, Data.MinValue, Data.MaxValue);
	Data.Value.Yaw = FMath::Clamp(Data.DefaultValue.Yaw, Data.MinValue, Data.MaxValue);
	Data.bConvertCoordinateSpace = true;
}

bool FAlterMeshRotationParam::UsedForType(FString Type) const
{
	return Type == TEXT("ROTATION");
}

TSubclassOf<UMovieSceneTrack> FAlterMeshRotationParam::GetTrackClass() const
{
	return UAlterMeshRotationTrack::StaticClass();
}
