// Copyright 2023 Aechmea

#include "CommonParams/AlterMeshTextureParam.h"

#include "AlterMeshExport.h"
#include "Dom/JsonObject.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DArray.h"
#include "Rendering/Texture2DResource.h"

void FAlterMeshTextureParam::SerializeJson(TArray<TSharedPtr<FJsonValue>>& ParametersArray) const
{
	if (UTexture2D* Texture = Cast<UTexture2D>(Data.Value))
	{	
		TSharedPtr<FJsonObject> ParamEntry = MakeShareable(new FJsonObject);
		ParamEntry->SetStringField(TEXT("NodeGroup"), BaseData.Id.NodeGroup.ToString());
		ParamEntry->SetStringField(TEXT("Id"), BaseData.Id.Input.ToString());
		ParamEntry->SetStringField(TEXT("Type"), TEXT("TEXTURE"));
		ParamEntry->SetNumberField(TEXT("X"), Texture->GetSizeX());
		ParamEntry->SetNumberField(TEXT("Y"), Texture->GetSizeY());

		// Value is not exported here, exported sequentially on FAlterMeshExport::Export()

		ParametersArray.Add(MakeShared<FJsonValueObject>(ParamEntry));
	}
}

void FAlterMeshTextureParam::DeserializeJson(const TSharedPtr<FJsonValue>& Param)
{
	
}

bool FAlterMeshTextureParam::UsedForType(FString Type) const
{
	return Type == TEXT("TEXTURE")
		|| Type == TEXT("IMAGE");
}

void FAlterMeshTextureParam::Export(FAlterMeshExport& Exporter) const
{
	if (UTexture2D* Texture = Cast<UTexture2D>(Data.Value))
	{
		switch(Texture->GetPixelFormat())
		{
		case PF_A32B32G32R32F:
			ExportTexture<FLinearColor>(Exporter, Texture);
			break;
		case PF_FloatRGBA:
			ExportTexture<FFloat16Color>(Exporter, Texture);
			break;
		case PF_R8:
			ExportTexture<uint8>(Exporter, Texture);
			break;
		case PF_DXT5:
		case PF_DXT1:
		case PF_BC5:
		case PF_R8G8B8A8:
		case PF_B8G8R8A8:
			ExportTexture<FColor>(Exporter, Texture);
			break;
		default:
			checkf(0, TEXT("Unsupported texture format"))
		}
	}
}


template<typename T>
void FAlterMeshTextureParam::ExportTexture(FAlterMeshExport& Exporter, UTexture2D* Texture) const
{
	// todo make work without editor
#if WITH_EDITOR
	if (Texture)
	{
		TArray<T> Colors;
		TArray<FColor> SRGBColors;

		int32 MipIndex = 0;
		const uint8* MipData = Texture->Source.LockMipReadOnly(MipIndex);		
		Colors.AddUninitialized(Texture->GetSizeX() * Texture->GetSizeY());
		SRGBColors.AddUninitialized(Texture->GetSizeX() * Texture->GetSizeY());
		FPlatformMemory::Memcpy(Colors.GetData(), MipData, Colors.Num() * sizeof(T));

		for (int32 i = 0; i < Colors.Num(); i++)
		{			
			if constexpr (std::is_same_v<T, uint8>)
			{
				SRGBColors[i] = FColor(Colors[i], Colors[i], Colors[i]);
			}
			else
			{
				SRGBColors[i] = FLinearColor(Colors[i]).ToFColor(true);				
			}
		}

		Texture->Source.UnlockMip(0);
		Exporter.WriteArray(SRGBColors);
	}
#endif
}
