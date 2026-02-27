// Fill out your copyright notice in the Description page of Project Settings.



#include "Node/TouchCameraBaseNobe.h"


ETouchCameraNodeState UTouchCameraBaseNobe::GetActivationState() const
{
	return ActivationState;
}

void UTouchCameraBaseNobe::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);


}

void UTouchCameraBaseNobe::SetGraphSampleNode(UTouchCameraBaseNobe* InGraphSampleNode)
{
	GraphSampleNode = InGraphSampleNode;
}


FString UTouchCameraBaseNobe::GetNodeCategory() const
{
	return TEXT("DefaultCategory");
}

FText UTouchCameraBaseNobe::GetNodeTitle() const
{
	return FText::FromString(TEXT("DefaultTitle"));
}

FText UTouchCameraBaseNobe::GetNodeToolTip() const
{
	return FText::FromString(TEXT("DefaultToolTip"));
}
