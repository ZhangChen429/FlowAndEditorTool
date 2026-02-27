// Copyright 2023 Aechmea

#include "AlterMeshTransformComponentVisualizer.h"

#include "AlterMeshSettings.h"
#include "AlterMeshTransformComponent.h"

#if WITH_EDITOR
#include "Editor.h"
#include "EditorViewportClient.h"
#endif

IMPLEMENT_HIT_PROXY(HAlterMeshTransformComponentVisProxy, HComponentVisProxy);

void FAlterMeshTransformComponentVisualizer::EndEditing()
{
	EditingComponents.Empty();

	FComponentVisualizer::EndEditing();
}

void FAlterMeshTransformComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FComponentVisualizer::DrawVisualization(Component, View, PDI);

	FLinearColor Color;
	if (GetDefault<UAlterMeshSettings>()->bCustomStyle)
	{
		// Cyan
		Color = EditingComponents.Contains(Component) && Component->IsSelected() ? FLinearColor(0.1f, 1.f, 1.f) : FLinearColor(0.6f, 0.9f, 0.9f);
	}
	else
	{
		Color = EditingComponents.Contains(Component) && Component->IsSelected() ? GEngine->GetSelectionOutlineColor() : GEngine->GetSelectionOutlineColor().Desaturate(0.4);
	}

	const UAlterMeshTransformComponent* TransformComponent = Cast<UAlterMeshTransformComponent>(Component);
	const FVector Location = TransformComponent->GetComponentLocation();
	const FVector Scale = TransformComponent->GetComponentScale();

	PDI->SetHitProxy(new HAlterMeshTransformComponentVisProxy(Component));

	switch (TransformComponent->DrawAs)
	{
		case EAlterMeshTransformDrawType::Point:
		{
			FTransform ComponentTransform = TransformComponent->GetComponentTransform();
			ComponentTransform.SetScale3D(ComponentTransform.GetScale3D() * 10);
			DrawWireDiamond(PDI, ComponentTransform.ToMatrixWithScale(), 3.f, Color, SDPG_Foreground, 3);
			break;
		}
		case EAlterMeshTransformDrawType::Box:
		{
			DrawOrientedWireBox(PDI, Location,  TransformComponent->GetForwardVector(),
				TransformComponent->GetRightVector(),
				TransformComponent->GetUpVector(), Scale * 100, Color, SDPG_Foreground, 3);
			break;
		}
		case EAlterMeshTransformDrawType::Sphere:
		{
			DrawWireSphereAutoSides(PDI, Location, Color, FMath::Max3<float>(Scale.X,Scale.Y,Scale.Z) * 100, SDPG_Foreground, 3);
			break;
		}
		case EAlterMeshTransformDrawType::Axis:
		{
			PDI->DrawLine(Location, Location + TransformComponent->GetForwardVector() * Scale.X * 100, Color, SDPG_Foreground, 3.f);
			PDI->DrawLine(Location, Location + TransformComponent->GetRightVector() * Scale.Y * 100, Color, SDPG_Foreground, 3.f);
			PDI->DrawLine(Location, Location + TransformComponent->GetUpVector() * Scale.Z * 100, Color, SDPG_Foreground, 3.f);
			break;
		}
		default: ;
	}

	PDI->SetHitProxy(nullptr);
}

bool FAlterMeshTransformComponentVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	if (VisProxy && VisProxy->Component.IsValid())
	{
		if (!(Click.IsShiftDown() || Click.IsControlDown()))
		{
			for (const UActorComponent* Component : EditingComponents)
			{
				GEditor->SelectComponent(const_cast<UActorComponent*>(Component), false, false);
			}

			EditingComponents.Empty();
		}

		GEditor->SelectComponent(const_cast<UActorComponent*>(VisProxy->Component.Get()), true, false);
		EditingComponents.Add(VisProxy->Component.Get());

		return true;
	}

	return true;
}

bool FAlterMeshTransformComponentVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	OutLocation = Cast<USceneComponent>(EditingComponents.Last())->GetComponentLocation();
	return true;
}