// Copyright 2023 Aechmea

#include "AlterMeshSplineComponentVisualizer.h"

#include "AlterMeshSettings.h"
#include "Settings/LevelEditorViewportSettings.h"

bool FAlterMeshSplineComponentVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	// Hack styling onto settings
	if (GetDefault<UAlterMeshSettings>()->bCustomStyle)
	{
		ULevelEditorViewportSettings* Settings = GetMutableDefault<ULevelEditorViewportSettings>();
		const float OldTangentScale = Settings->SplineTangentScale;

		// This makes it about the same as blender's
		Settings->SplineTangentScale = OldTangentScale/3.f;

		const bool bResult = FSplineComponentVisualizer::GetWidgetLocation(ViewportClient, OutLocation);

		Settings->SplineTangentScale = OldTangentScale;

		return bResult;
	}
	else
	{
		return FSplineComponentVisualizer::GetWidgetLocation(ViewportClient, OutLocation);
	}
}

void FAlterMeshSplineComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	// Hack styling onto settings
	if (GetDefault<UAlterMeshSettings>()->bCustomStyle)
	{
		ULevelEditorViewportSettings* Settings = GetMutableDefault<ULevelEditorViewportSettings>();
		const float OldHandleSize = Settings->SplineTangentHandleSizeAdjustment;
		const float OldTangentScale = Settings->SplineTangentScale;
		const float OldPointSize = Settings->SelectedSplinePointSizeAdjustment;

		// This makes it about the same as blender's
		Settings->SplineTangentHandleSizeAdjustment = FMath::Max(OldHandleSize, 4.f);
		Settings->SplineTangentScale = OldTangentScale/3.f;
		Settings->SelectedSplinePointSizeAdjustment = FMath::Max(OldPointSize, 4.f);

		FSplineComponentVisualizer::DrawVisualization(Component, View, PDI);

		Settings->SplineTangentHandleSizeAdjustment = OldHandleSize;
		Settings->SplineTangentScale = OldTangentScale;
		Settings->SelectedSplinePointSizeAdjustment = OldPointSize;
	}
	else
	{
		FSplineComponentVisualizer::DrawVisualization(Component, View, PDI);
	}
}
