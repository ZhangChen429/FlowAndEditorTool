// Copyright 2023 Aechmea

#include "AlterMeshConverter.h"

#include "AlterMeshActor.h"
#include "AlterMeshComponent.h"
#include "AssetToolsModule.h"
#include "ClassViewerFilter.h"
#include "Kismet2/SClassPickerDialog.h"

#define LOCTEXT_NAMESPACE "AlterMeshConverter"

void UAlterMeshConverterBase::ForEachSection(AAlterMeshActor* InActor, TFunctionRef<void(FAlterMeshSection& Section)> Function, bool bIncludeInstances)
{
	if (InActor)
	{
		for (UAlterMeshComponent* Component : InActor->AlterMeshComponents)
		{
			if (Component->IsVisible() || bIncludeInstances)
			{
				for (int32 SectionIndex = 0; SectionIndex < Component->Sections.Num(); SectionIndex++)
				{
					Function(Component->Sections[SectionIndex]);
				}
			}
		}
	}
}

void UAlterMeshConverterBase::CleanUp()
{
}

void UAlterMeshConverterBase::Convert(AAlterMeshActor* InActor)
{
}


#undef LOCTEXT_NAMESPACE