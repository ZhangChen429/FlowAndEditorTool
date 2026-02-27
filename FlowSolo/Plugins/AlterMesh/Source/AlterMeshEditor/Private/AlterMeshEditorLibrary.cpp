// Copyright 2023 Aechmea

#include "AlterMeshEditorLibrary.h"
#include "AlterMeshActor.h"
#include "AlterMeshConverter.h"

void UAlterMeshEditorLibrary::SetConverterClass(AAlterMeshActor* InActor, TSubclassOf<UAlterMeshConverterBase> InClass)
{
	if (InClass)
	{
		InActor->ConverterInstance = NewObject<UAlterMeshConverterBase>(InActor, InClass);
		InActor->OnBecomePreview.AddDynamic(Cast<UAlterMeshConverterBase>(InActor->ConverterInstance), &UAlterMeshConverterBase::CleanUp);
	}
}
