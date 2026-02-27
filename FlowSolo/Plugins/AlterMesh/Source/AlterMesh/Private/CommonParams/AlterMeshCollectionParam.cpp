// Copyright 2023 Aechmea

#include "CommonParams/AlterMeshCollectionParam.h"

#include "Dom/JsonObject.h"
#include "GeometryParams/AlterMeshGeometryBase.h"

void FAlterMeshCollectionParam::SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const
{
	bool bShouldExportCollection = false;

	// Only export collection if theres at least one valid item
	for (const FAlterMeshGeometryParam& MeshParam : Data.Geometries)
	{
		bShouldExportCollection |= MeshParam.Data.InputType && MeshParam.Data.InputType->ShouldExport();
	}

	if (bShouldExportCollection)
	{
		TSharedPtr<FJsonObject> ParamEntry = MakeShareable(new FJsonObject);
		ParamEntry->SetStringField(TEXT("NodeGroup"), BaseData.Id.NodeGroup.ToString());
		ParamEntry->SetStringField(TEXT("Id"), BaseData.Id.Input.ToString());
		ParamEntry->SetStringField(TEXT("Type"), TEXT("COLLECTION"));
		ParamEntry->SetNumberField(TEXT("Num"),  Data.Geometries.Num());

		TArray<TSharedPtr<FJsonValue>> Classes;
		for (const FAlterMeshGeometryParam& GeometryParam : Data.Geometries)
		{
			if (GeometryParam.Data.InputType && GeometryParam.Data.InputType->ShouldExport())
			{
				Classes.Add(MakeShared<FJsonValueString>(GeometryParam.Data.InputType->GetClass()->GetFName().ToString()));
			}
		}

		ParamEntry->SetArrayField(TEXT("Classes"), Classes);
		ParametersArray.Add(MakeShared<FJsonValueObject>(ParamEntry));
	}
}

void FAlterMeshCollectionParam::DeserializeJson(const TSharedPtr<FJsonValue>& Param)
{
	
}

bool FAlterMeshCollectionParam::UsedForType(FString Type) const
{
	return Type == TEXT("COLLECTION");
}

void FAlterMeshCollectionParam::Export(FAlterMeshExport& Exporter) const
{	
	for (const FAlterMeshGeometryParam& MeshParam : Data.Geometries)
	{
		if (MeshParam.Data.InputType && MeshParam.Data.InputType->ShouldExport())
		{
			MeshParam.Data.InputType->Export(Exporter);
		}
	}
}
