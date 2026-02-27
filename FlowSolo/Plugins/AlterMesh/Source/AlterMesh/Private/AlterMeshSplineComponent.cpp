// Copyright 2023 Aechmea

#include "AlterMeshSplineComponent.h"

#include "AlterMeshSettings.h"

#if WITH_EDITOR
void UAlterMeshSplineComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	if (GetDefault<UAlterMeshSettings>()->bCustomStyle)
	{
		EditorSelectedSplineSegmentColor = FLinearColor::White;
		EditorUnselectedSplineSegmentColor = FLinearColor(0.f, 1.f, 1.f);
		EditorTangentColor = FLinearColor::White;
	}
}

void UAlterMeshSplineComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FEditorScriptExecutionGuard ScriptGuard;
	OnSplineChange.Broadcast();
}

void UAlterMeshSplineComponent::PostEditUndo()
{
	Super::PostEditUndo();

	FEditorScriptExecutionGuard ScriptGuard;
	OnSplineChange.Broadcast();
}
#endif