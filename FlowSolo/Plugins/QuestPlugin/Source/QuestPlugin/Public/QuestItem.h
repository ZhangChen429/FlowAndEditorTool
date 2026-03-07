// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QuestItem.generated.h"

/**
 * 
 */
UCLASS()
class QUESTPLUGIN_API UQuestItem : public UObject
{
	GENERATED_BODY()
	
	public:
	UQuestItem();
	UFUNCTION()
	void SetQuestName(FString Name);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString QuestName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString QuestDescription;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString QuestReward;
};
