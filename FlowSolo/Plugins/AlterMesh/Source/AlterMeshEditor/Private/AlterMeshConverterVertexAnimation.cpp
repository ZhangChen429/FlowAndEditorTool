// Copyright 2023 Aechmea

#include "AlterMeshConverterVertexAnimation.h"

#include "AlterMeshActor.h"
#include "AlterMeshComponent.h"
#include "AlterMeshImport.h"
#include "AlterMeshSettings.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

UAlterMeshConverterVertexAnimation::UAlterMeshConverterVertexAnimation()
{
	bCombineMeshes = true;
}

void UAlterMeshConverterVertexAnimation::Convert(AAlterMeshActor* InActor)
{
#if !ALTERMESH_FREE
	if (!GeneratedTexture)
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		FString PackageName = FPackageName::FilenameToLongPackageName(TEXT("/Game/AlterMesh/") + InActor->Asset->Get()->ObjectName);
		FString Name = InActor->Asset->Get()->ObjectName;
		AssetToolsModule.Get().CreateUniqueAssetName(PackageName, TEXT(""), PackageName, Name);

		// Then find/create it.
		UPackage* Package = CreatePackage(*PackageName);
		check(Package);

		TArray<FFloat16Color> NewColors;

		TArray<TSharedPtr<FAlterMeshPrimitive>> OrigMeshes;
		FIntPoint TexSize = FIntPoint::ZeroValue;
		
		ProcessFrames([&NewColors, &OrigMeshes, &TexSize, this](TArray<TSharedPtr<FAlterMeshPrimitive>> Meshes, int32 Frame)
		{
			// Initialize
			if (Frame == 0)
			{
				OrigMeshes = Meshes;
				TexSize.Y = LastFrame - InitialFrame +1;
					
				for (auto Mesh : Meshes)
				{
					TexSize.X += Mesh->Sections[0].Vertices.Num();
				}

				if (TexSize.X*TexSize.Y - NewColors.Num() > 0)
				{
					NewColors.AddZeroed(TexSize.X*TexSize.Y - NewColors.Num());				
				}			
			}

			//todo multiple meshes vat
			
			for (int32 i = 0; i < Meshes[0].Get()->Sections[0].Vertices.Num(); i++)
			{
				FVector3f OrigLocation =  OrigMeshes[0].Get()->Sections[0].Vertices[i];
				FVector3f Location = Meshes[0].Get()->Sections[0].Vertices[i] - OrigLocation;
				NewColors[Frame*TexSize.X+i] = FFloat16Color(Location);
			}
			
			return true;
		});

		// Create Texture object
		UTexture2D* Texture = UTexture2D::CreateTransient(TexSize.X, TexSize.Y, PF_FloatRGBA);
		Texture->AddToRoot();
	
		Texture->Rename(*Name, Package);
		Texture->ClearFlags(RF_Transient);
		Texture->SetFlags(RF_Public | RF_Standalone | RF_Transactional);

		ForEachSection(InActor, [&](FAlterMeshSection& Section)
		{
			Section.UV1.SetNumUninitialized(Section.Vertices.Num());
			for (int32 i = 0; i < Section.UV1.Num(); i++)
			{
				Section.UV1[i].X = (i + 0.5f) / TexSize.X; 
			}
		});
		
		void* MipData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		FPlatformMemory::Memcpy(MipData, NewColors.GetData(), NewColors.Num() * NewColors.GetTypeSize());	
		Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
		Texture->MarkPackageDirty();
		Texture->UpdateResource();
		
		// Notify asset registry of new asset
		FAssetRegistryModule::AssetCreated(Texture);

		GeneratedTexture = Texture;
	}

	// Convert to mesh
	Super::Convert(InActor);

	if (SpawnedComponents.Num() && GeneratedTexture)
	{
		if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(SpawnedComponents[0]))
		{
			const UAlterMeshEditorSettings* Settings = GetDefault<UAlterMeshEditorSettings>();
			UMaterialInstanceDynamic* Material = StaticMesh->CreateDynamicMaterialInstance(
				0, Settings->DefaultVATMaterial.LoadSynchronous());
			
			Material->SetTextureParameterValue(FName("VertexAnimationTexture"), GeneratedTexture);
		}
	}
#endif
}

void UAlterMeshConverterVertexAnimation::CleanUp()
{
	Super::CleanUp();
}
