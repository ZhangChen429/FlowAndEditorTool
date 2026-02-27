// Copyright 2023 Aechmea

#include "CommonParams/AlterMeshVectorParam.h"

#include "AlterMeshTracks.h"
#include "Dom/JsonObject.h"

void FAlterMeshVectorParam::SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const
{
	TSharedPtr<FJsonObject> ParamEntry = MakeShareable(new FJsonObject);
	ParamEntry->SetStringField(TEXT("NodeGroup"), BaseData.Id.NodeGroup.ToString());
	ParamEntry->SetStringField(TEXT("Id"), BaseData.Id.Input.ToString());

	TSharedPtr<FJsonObject> VectorObject = MakeShareable(new FJsonObject);

	// Remap for coordinate system
	if (Data.bConvertCoordinateSpace)
	{
		VectorObject->SetNumberField(TEXT("X"), -Data.Value.Y);
		VectorObject->SetNumberField(TEXT("Y"), -Data.Value.X);
	}
	else
	{
		VectorObject->SetNumberField(TEXT("X"), Data.Value.X);
		VectorObject->SetNumberField(TEXT("Y"), Data.Value.Y);
	}

	VectorObject->SetNumberField(TEXT("Z"), Data.Value.Z);

	TArray<TSharedPtr<FJsonValue>> Vector;
	Vector.Add(MakeShared<FJsonValueObject>(VectorObject));

	ParamEntry->SetArrayField(TEXT("Value"), Vector);
	ParamEntry->SetStringField(TEXT("Type"), TEXT("VECTOR"));
	
	ParametersArray.Add(MakeShared<FJsonValueObject>(ParamEntry));

}

void FAlterMeshVectorParam::DeserializeJson(const TSharedPtr<FJsonValue>& Param)
{
	Data.DefaultValue = FVector(Param->AsObject()->GetArrayField(TEXT("DefaultValue"))[0]->AsNumber(),
			Param->AsObject()->GetArrayField(TEXT("DefaultValue"))[1]->AsNumber(),
			Param->AsObject()->GetArrayField(TEXT("DefaultValue"))[2]->AsNumber());
	Data.MinValue = Param->AsObject()->GetNumberField(TEXT("MinValue"));
	Data.MaxValue = Param->AsObject()->GetNumberField(TEXT("MaxValue"));
	Data.Value.X = FMath::Clamp(Data.DefaultValue.X, Data.MinValue, Data.MaxValue);
	Data.Value.Y = FMath::Clamp(Data.DefaultValue.Y, Data.MinValue, Data.MaxValue);
	Data.Value.Z = FMath::Clamp(Data.DefaultValue.Z, Data.MinValue, Data.MaxValue);
	Data.bConvertCoordinateSpace = false;
}

bool FAlterMeshVectorParam::UsedForType(FString Type) const
{
	return Type == TEXT("VECTOR");
}

TSubclassOf<UMovieSceneTrack> FAlterMeshVectorParam::GetTrackClass() const
{
	return UAlterMeshVectorTrack::StaticClass();
}
