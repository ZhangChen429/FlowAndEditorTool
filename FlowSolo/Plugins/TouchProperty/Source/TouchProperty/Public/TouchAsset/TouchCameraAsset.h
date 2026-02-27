// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TouchCameraAsset.generated.h"
class UFlowAsset;
class UTouchCameraBaseNobe;
class UCameraGraphSchema;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnChangeAssetInPreview, UStaticMesh*);
class UCustomDetailsViewContent;
UCLASS(Blueprintable)
class TOUCHPROPERTY_API UTouchCameraAsset  : public UObject  
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString Name;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int32 ID;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UStaticMesh* TouchCameraStaticMesh;

	
	//=======View=======
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UCustomDetailsViewContent* CustomDetailsViewContent;

public:
#if WITH_EDITOR
	friend class UCameraGraphSchema;
	void SetDefaultFlowAsset(UFlowAsset* InFlowAsset);
	UEdGraph* GetGraph() const { return GraphSampleGraph; }
#endif

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<UEdGraph> GraphSampleGraph;
#endif

	UPROPERTY()
	UTouchCameraBaseNobe* RootNode;
	
	//=======View=======
	//Graph
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	FOnChangeAssetInPreview OnChangeAssetInPreview;


	UFlowAsset* GetMainFlowAsset() const {return  MainFlowAsset;};
protected:
	UPROPERTY(EditAnywhere)
	UFlowAsset* MainFlowAsset;
	
};
