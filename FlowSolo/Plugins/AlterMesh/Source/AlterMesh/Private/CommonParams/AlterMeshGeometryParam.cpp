// Copyright 2023 Aechmea

#include "CommonParams/AlterMeshGeometryParam.h"
#include "Dom/JsonObject.h"
#include "GeometryParams/AlterMeshGeometryBase.h"

void FAlterMeshGeometryParam::SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const
{
	if (Data.InputType && Data.InputType->ShouldExport())
	{
		TSharedPtr<FJsonObject> ParamEntry = MakeShareable(new FJsonObject);
		ParamEntry->SetStringField(TEXT("NodeGroup"), BaseData.Id.NodeGroup.ToString());
		ParamEntry->SetStringField(TEXT("Id"), BaseData.Id.Input.ToString());
		ParamEntry->SetStringField(TEXT("Type"), TEXT("MESH"));
		ParamEntry->SetStringField(TEXT("Class"), Data.InputType->GetClass()->GetFName().ToString());

		// Value is not exported here, exported sequentially on FAlterMeshExport::Export()

		ParametersArray.Add(MakeShared<FJsonValueObject>(ParamEntry));
	}
}

bool FAlterMeshGeometryParam::UsedForType(FString Type) const
{	
	return Type == TEXT("OBJECT");
}

void FAlterMeshGeometryParam::PreExport(FAlterMeshExport& Exporter) const
{
	if (Data.InputType && Data.InputType->ShouldExport())
	{
		Data.InputType->PreExport( Exporter);
	}
}

void FAlterMeshGeometryParam::Export(FAlterMeshExport& Exporter) const
{
	if (Data.InputType && Data.InputType->ShouldExport())
	{
		Data.InputType->Export( Exporter);
	}
}
