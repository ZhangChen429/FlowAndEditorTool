// Copyright 2023 Aechmea

#include "AlterMeshExport.h"
#include "AlterMeshAsset.h"
#include "AlterMeshParam.h"
#include "EngineUtils.h"
#include "LandscapeDataAccess.h"
#include "LandscapeRender.h"
#include "NiagaraComponent.h"
#include "AlterMesh/AlterMesh.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "NiagaraFunctionLibrary.h"
#include "GeometryParams/AlterMeshGeometryBase.h"
#include "Rendering/Texture2DResource.h"

DEFINE_LOG_CATEGORY(LogAlterMeshExport)

void FAlterMeshExport::PreExport()
{	
	for (const FAlterMeshParamBase* Param : Params.GetTyped<FAlterMeshParamBase>())
	{
		Param->PreExport(*this);
	}
}

void FAlterMeshExport::Export()
{
	if (AlterMeshHandle.IsValid() && WriteLock(AlterMeshHandle->Get()))
	{
		const double StartTime = FPlatformTime::Seconds();

		// Params
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		TSharedPtr<FJsonObject> JsonParams = MakeShareable(new FJsonObject);
		TArray<TSharedPtr<FJsonValue>> ParamsArray;

		// Write out new values
		for (const FAlterMeshParamBase* Param : Params.GetTyped<FAlterMeshParamBase>())
		{
			Param->SerializeJson(ParamsArray);
		}

		TArray<TSharedPtr<FJsonValue>> AttributesArray;

		for (const FAlterMeshAttributeMapping& Mapping : Asset->Get()->AttributeMapping)
		{			
			TSharedPtr<FJsonObject> ParamEntry = MakeShareable(new FJsonObject);
			ParamEntry->SetStringField(FString("AttributeName"), Mapping.From.ToString());
			AttributesArray.Add(MakeShared<FJsonValueObject>(ParamEntry));
		}
		
		JsonObject->SetArrayField(TEXT("Params"), ParamsArray);
		JsonObject->SetNumberField(TEXT("Frame"), Params.Frame);
		JsonObject->SetStringField(TEXT("Object"), Asset->Get()->ObjectName);
		JsonObject->SetArrayField(TEXT("Attributes"), AttributesArray);
		TSharedRef<FJsonValueArray> ParamsArrayValue = MakeShared<FJsonValueArray>(ParamsArray);

		FString JsonString;
		const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonString);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

		Write(AlterMeshHandle->Get(), TCHAR_TO_ANSI(*JsonString), JsonString.Len());

		// Export non json data
		for (const FAlterMeshParamBase* Param : Params.GetTyped<FAlterMeshParamBase>())
		{
			Param->Export(*this);
		}

		WriteUnlock(AlterMeshHandle->Get());

		const double TotalTime = FPlatformTime::Seconds() - StartTime;
		if (CVarAlterMeshDebugProcessOutput.GetValueOnAnyThread())
		{
			UE_LOG( LogAlterMeshExport, Log, TEXT( "Exporting took %.4f sec(s)" ), TotalTime );
		}
	}
}

void FAlterMeshExport::InvertUVs(TArray<FVector2f>& OutUVs)
{
	for (FVector2f& UV : OutUVs)
	{
		UV.Y = 1.f - UV.Y;
	}
}
