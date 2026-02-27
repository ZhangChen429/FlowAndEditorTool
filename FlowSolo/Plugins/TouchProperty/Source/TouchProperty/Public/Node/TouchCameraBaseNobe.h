// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TouchCameraBaseNobe.generated.h"


/**
 * 
 */
UENUM()
enum ETouchCameraNodeState : uint8
{
	Default = 0,
	Completed,
	Failed
};

UCLASS()
class TOUCHPROPERTY_API UTouchCameraBaseNobe : public UObject
{
	GENERATED_BODY()
	
public:
	/** For example, the organization of runtime data depends on the specific logic, officially remove VisibleAnywhere */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<UTouchCameraBaseNobe*> Children;

	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadWrite, meta = (ShowOnlyInnerProperties))
	UTouchCameraBaseNobe* GraphSampleNode;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual FString GetNodeCategory() const;
	virtual FText GetNodeTitle() const;
	virtual FText GetNodeToolTip() const;
	virtual ETouchCameraNodeState GetActivationState() const;
#endif
public:
	void SetGraphSampleNode(UTouchCameraBaseNobe* InGraphSampleNode);
#if WITH_EDITORONLY_DATA
	ETouchCameraNodeState ActivationState = ETouchCameraNodeState::Default;
#endif
	
};
