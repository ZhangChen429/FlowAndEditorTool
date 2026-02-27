// Copyright 2023 Aechmea

#include "CommonParams/AlterMeshMenuParam.h"

#include "AlterMeshTracks.h"
#include "Dom/JsonObject.h"

void FAlterMeshMenuParam::SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const
{
	TSharedPtr<FJsonObject> ParamEntry = MakeShareable(new FJsonObject);
	ParamEntry->SetStringField(TEXT("NodeGroup"), BaseData.Id.NodeGroup.ToString());
	ParamEntry->SetStringField(TEXT("Id"), BaseData.Id.Input.ToString());
	ParamEntry->SetNumberField(TEXT("Value"), Data.Entries.Find(Data.Value));
	ParamEntry->SetStringField(TEXT("Type"), TEXT("MENU"));

	ParametersArray.Add(MakeShared<FJsonValueObject>(ParamEntry));
}

void FAlterMeshMenuParam::DeserializeJson(const TSharedPtr<FJsonValue>& Param)
{	
	Data.DefaultValue = Param->AsObject()->GetIntegerField(TEXT("DefaultValue"));
	Data.Entries.Empty();
	for (auto JsonEntry : Param->AsObject()->GetArrayField(TEXT("Entries")))
	{
		FString Entry = JsonEntry->AsString();
		Data.Entries.Add(Entry);		
	}

	Data.Values.Empty();
	for (auto JsonValue : Param->AsObject()->GetArrayField(TEXT("Values")))
	{
		int32 Value = JsonValue->AsNumber();
		Data.Values.Add(Value);		
	}
	
	Data.Value = Data.Entries.IsValidIndex(Data.DefaultValue) ? Data.Entries[Data.DefaultValue] : FString("");	
}

bool FAlterMeshMenuParam::UsedForType(FString Type) const
{
	return Type == FString("MENU");
}