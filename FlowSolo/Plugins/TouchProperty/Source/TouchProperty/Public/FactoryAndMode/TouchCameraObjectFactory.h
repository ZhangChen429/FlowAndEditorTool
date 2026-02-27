// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Graph/TouchCameraGraph.h"
#include "TouchAsset/TouchCameraAsset.h"
#include "TouchCameraObjectFactory.generated.h"

/**
 * 
 */
UCLASS()
class TOUCHPROPERTY_API UTouchCameraObjectFactory : public UFactory
{
	GENERATED_BODY()
	UTouchCameraObjectFactory()
	{
		SupportedClass = UTouchCameraAsset::StaticClass();
		// 指定是否可以通过菜单来创建
		bCreateNew = true;
		// 创建Asset之后是否自动打开编辑
		bEditAfterNew = true;
	}
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName,
										EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

};
