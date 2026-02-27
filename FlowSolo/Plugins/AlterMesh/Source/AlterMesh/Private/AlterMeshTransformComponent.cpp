// Copyright 2023 Aechmea

// Copyright 2023 Aechmea


#include "AlterMeshTransformComponent.h"

#if WITH_EDITOR
void UAlterMeshTransformComponent::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);

	FEditorScriptExecutionGuard ScriptGuard;
	OnChangeDelegate.Broadcast();
}

void UAlterMeshTransformComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

	FEditorScriptExecutionGuard ScriptGuard;
	OnChangeDelegate.Broadcast();
}
#endif