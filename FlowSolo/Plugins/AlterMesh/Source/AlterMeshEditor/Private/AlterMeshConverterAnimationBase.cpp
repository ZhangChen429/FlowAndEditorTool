// Copyright 2023 Aechmea

#include "AlterMeshConverterAnimationBase.h"

#include "AlterMeshActor.h"

UAlterMeshConverterAnimationBase::UAlterMeshConverterAnimationBase()
{
	BuildSettings.bGenerateLightmapUVs = false;
	BuildSettings.bUseFullPrecisionUVs = true;
}

bool UAlterMeshConverterAnimationBase::ProcessFrames(TFunctionRef<bool(TArray<TSharedPtr<FAlterMeshPrimitive>>, int32)> InFunc)
{
 	if (AAlterMeshActor* OwnerActor = GetTypedOuter<AAlterMeshActor>())
	{
		for (int32 Frame = InitialFrame; Frame <= LastFrame; Frame++)
		{
			OwnerActor->InputParams.Frame = Frame;
			TArray<TSharedPtr<FAlterMeshPrimitive>> Meshes;			
			OwnerActor->BlenderInstance->RefreshSync(OwnerActor->InputParams, OwnerActor->Asset, Meshes);
			
			if (!InFunc(Meshes, Frame - InitialFrame))
			{
				return false;
			}
		}
	}
	
	return true;
}

void UAlterMeshConverterAnimationBase::Convert(AAlterMeshActor* InActor)
{
	Super::Convert(InActor);	
}

void UAlterMeshConverterAnimationBase::CleanUp()
{
	Super::CleanUp();
}
