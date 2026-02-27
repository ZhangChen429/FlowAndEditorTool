// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TouchCameraGraph.generated.h"

class UTouchCameraAsset;
class UCameraGraphSchema;
/**
 * 
 */
UCLASS()
class TOUCHPROPERTY_API UTouchCameraGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	static UEdGraph* CreateGraph(UTouchCameraAsset* InGraphSampleAsset);
	static UEdGraph* CreateGraph(UTouchCameraAsset* InGraphSampleAsset, const TSubclassOf<UCameraGraphSchema>& GraphSampleSchema);
	UTouchCameraAsset* GetGraphSampleAsset() const;
	
	void OnGraphSampleNodeChanged();
};
