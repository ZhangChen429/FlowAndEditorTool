// Copyright 2023 Aechmea

#pragma once
#include "ComponentVisualizer.h"

class UAlterMeshTransformComponent;
class FPrimitiveDrawInterface;
class FSceneView;

struct ALTERMESHEDITOR_API HAlterMeshTransformComponentVisProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	HAlterMeshTransformComponentVisProxy(const UActorComponent* InComponent)
	: HComponentVisProxy(InComponent, HPP_Foreground)
	{}

	virtual EMouseCursor::Type GetMouseCursor() override
	{
		return  EMouseCursor::Default;
	}
};

class ALTERMESHEDITOR_API FAlterMeshTransformComponentVisualizer : public FComponentVisualizer
{
public:

	virtual void EndEditing() override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
private:
	TArray<const UActorComponent*> EditingComponents;
};