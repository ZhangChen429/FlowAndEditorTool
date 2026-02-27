// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Graph/Widgets/SFlowGraphNode.h"

/**
 * Custom Slate widget for Fact Node
 * This is where you customize the visual appearance of your node
 */
class FLOWFACTEDITOR_API SFlowGraphNode_FactNode : public SFlowGraphNode
{
public:
	SLATE_BEGIN_ARGS(SFlowGraphNode_FactNode) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UFlowGraphNode* InNode);
	virtual void UpdateGraphNode() override;
	virtual const FSlateBrush* GetNodeBodyBrush() const override;
	virtual TSharedRef<SWidget> CreateNodeContentArea() override;

	// Override to ensure fixed size
	virtual FVector2D ComputeDesiredSize(float) const override;

	// Custom painting for hexagon shape
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

protected:
	// Override these to customize node appearance
	virtual FSlateColor GetBorderBackgroundColor() const override;
	virtual FText GetTitle() const;
	virtual const FSlateBrush* GetNameIcon() const override;

private:
	// Helper function to generate hexagon points
	static TArray<FVector2D> GenerateHexagonPoints(const FVector2D& Center, float Radius);
};
