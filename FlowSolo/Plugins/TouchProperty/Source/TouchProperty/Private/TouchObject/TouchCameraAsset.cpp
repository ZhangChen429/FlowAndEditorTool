// Fill out your copyright notice in the Description page of Project Settings.


#include "TouchAsset/TouchCameraAsset.h"


void UTouchCameraAsset::SetDefaultFlowAsset(UFlowAsset* InFlowAsset)
{
	MainFlowAsset=InFlowAsset;
}

void UTouchCameraAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UObject::PostEditChangeProperty(PropertyChangedEvent);
	
	FName PropertyName = (PropertyChangedEvent.MemberProperty != nullptr) ? 
							  PropertyChangedEvent.MemberProperty->GetFName() : 
							  NAME_None;
	
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UTouchCameraAsset, TouchCameraStaticMesh))
	{
		// 触发相机位置变化委托
		OnChangeAssetInPreview.Broadcast(TouchCameraStaticMesh);
	}
	
}

