// Copyright 2023 Aechmea

#pragma once
#include "ComponentVisualizer.h"
#include "SplineComponentVisualizer.h"

class FPrimitiveDrawInterface;
class FSceneView;

class FAlterMeshSplineComponentVisualizer : public FSplineComponentVisualizer
{
public:

	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};
