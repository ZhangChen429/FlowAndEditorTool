// Fill out your copyright notice in the Description page of Project Settings.


#include "Graph/CameraGraphSchema.h"
#include "Graph/TouchCameraGraphSchemaAction.h"


#define LOCTEXT_NAMESPACE "CameraGraphSchema"

void UCameraGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	
	GetCommentAction(ContextMenuBuilder, ContextMenuBuilder.CurrentGraph);
}

void UCameraGraphSchema::GetCommentAction(FGraphActionMenuBuilder& ActionMenuBuilder, const UEdGraph* CurrentGraph)
{
	
}


void UCameraGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	Super::CreateDefaultNodesForGraph(Graph);
}

const FPinConnectionResponse UCameraGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	return Super::CanCreateConnection(A, B);
}

const FPinConnectionResponse UCameraGraphSchema::CanMergeNodes(const UEdGraphNode* NodeA,
	const UEdGraphNode* NodeB) const
{
	return Super::CanMergeNodes(NodeA, NodeB);
}

bool UCameraGraphSchema::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const
{
	return Super::TryCreateConnection(A, B);
}

bool UCameraGraphSchema::ShouldHidePinDefaultValue(UEdGraphPin* Pin) const
{
	return Super::ShouldHidePinDefaultValue(Pin);
}

FLinearColor UCameraGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return Super::GetPinTypeColor(PinType);
}

FText UCameraGraphSchema::GetPinDisplayName(const UEdGraphPin* Pin) const
{
	return Super::GetPinDisplayName(Pin);
}

void UCameraGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const
{
	Super::BreakNodeLinks(TargetNode);
}

void UCameraGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotification) const
{
	Super::BreakPinLinks(TargetPin, bSendsNodeNotification);
}

int32 UCameraGraphSchema::GetNodeSelectionCount(const UEdGraph* Graph) const
{
	return Super::GetNodeSelectionCount(Graph);
}

TSharedPtr<FEdGraphSchemaAction> UCameraGraphSchema::GetCreateCommentAction() const
{
	return Super::GetCreateCommentAction();
}

void UCameraGraphSchema::OnPinConnectionDoubleCicked(UEdGraphPin* PinA, UEdGraphPin* PinB,
	const FVector2D& GraphPosition) const
{
	Super::OnPinConnectionDoubleCicked(PinA, PinB, GraphPosition);
}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6
void UCameraGraphSchema::OnPinConnectionDoubleCicked(UEdGraphPin* PinA, UEdGraphPin* PinB,
	const FVector2f& GraphPosition) const
{
	Super::OnPinConnectionDoubleCicked(PinA, PinB, GraphPosition);
}
#endif

bool UCameraGraphSchema::IsCacheVisualizationOutOfDate(int32 InVisualizationCacheID) const
{
	return Super::IsCacheVisualizationOutOfDate(InVisualizationCacheID);
}

int32 UCameraGraphSchema::GetCurrentVisualizationCacheID() const
{
	return Super::GetCurrentVisualizationCacheID();
}

void UCameraGraphSchema::ForceVisualizationCacheClear() const
{
	Super::ForceVisualizationCacheClear();
}

bool UCameraGraphSchema::ArePinsCompatible(const UEdGraphPin* PinA, const UEdGraphPin* PinB,
	const UClass* CallingContext, bool bIgnoreArray) const
{
	return Super::ArePinsCompatible(PinA, PinB, CallingContext, bIgnoreArray);
}

void UCameraGraphSchema::ConstructBasicPinTooltip(const UEdGraphPin& Pin, const FText& PinDescription,
	FString& TooltipOut) const
{
	Super::ConstructBasicPinTooltip(Pin, PinDescription, TooltipOut);
}

bool UCameraGraphSchema::IsTitleBarPin(const UEdGraphPin& Pin) const
{
	return Super::IsTitleBarPin(Pin);
}

bool UCameraGraphSchema::CanShowDataTooltipForPin(const UEdGraphPin& Pin) const
{
	return Super::CanShowDataTooltipForPin(Pin);
}

void UCameraGraphSchema::GatherNodes()
{
	// prevent asset crunching during PIE
	if (GEditor && GEditor->PlayWorld)
	{
		return;
	}
	if (GCompilingBlueprint)
	{
		return;
	}
	
}

UClass* UCameraGraphSchema::GetMappingGraphNodeClass(const UClass* GraphSampleNodeClass)
{
	
	return UTouchCameraBaseNobe::StaticClass();
}


#undef LOCTEXT_NAMESPACE // "GraphSampleGraphSchema"
