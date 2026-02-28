// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkspotBehaviorDefinition.h"
#include "WorkspotTree.h"
#include "WorkspotComponent.h"
#include "WorkspotGameplayBehavior.h"
#include "Workspot.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

UWorkspotBehaviorDefinition::UWorkspotBehaviorDefinition()
{
	WorkspotTree = nullptr;
	bAutoSelectClosestEntry = true;
	OccupyDuration = 0.0f;
	MinimumOccupyTime = 3.0f;
	bAllowFastExit = true;
	bTakeCharacterControl = true;
}

TSubclassOf<UGameplayBehavior> UWorkspotBehaviorDefinition::GetBehaviorClass() const
{
	return UGameplayBehavior_Workspot::StaticClass();
}

bool UWorkspotBehaviorDefinition::StartWorkspotOnActor(AActor* Actor) const
{
	if (!Actor)
	{
		UE_LOG(LogWorkspot, Error, TEXT("StartWorkspotOnActor: Actor is null"));
		return false;
	}

	if (!WorkspotTree)
	{
		UE_LOG(LogWorkspot, Error, TEXT("StartWorkspotOnActor: No WorkspotTree assigned"));
		return false;
	}

	// Find or create workspot component
	UWorkspotComponent* WorkspotComp = Actor->FindComponentByClass<UWorkspotComponent>();
	if (!WorkspotComp)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("StartWorkspotOnActor: No WorkspotComponent found on %s"),
			*Actor->GetName());
		return false;
	}

	// Start the workspot with the tree
	bool bSuccess = WorkspotComp->StartWorkspotWithTree(WorkspotTree);

	if (bSuccess)
	{
		UE_LOG(LogWorkspot, Log, TEXT("StartWorkspotOnActor: Successfully started workspot '%s' on actor '%s'"),
			*WorkspotTree->GetName(), *Actor->GetName());
	}
	else
	{
		UE_LOG(LogWorkspot, Error, TEXT("StartWorkspotOnActor: Failed to start workspot on actor '%s'"),
			*Actor->GetName());
	}

	return bSuccess;
}

#if WITH_EDITOR
EDataValidationResult UWorkspotBehaviorDefinition::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = Super::IsDataValid(ValidationErrors);

	if (!WorkspotTree)
	{
		ValidationErrors.Add(FText::FromString(TEXT("WorkspotBehaviorDefinition has no WorkspotTree assigned")));
		Result = EDataValidationResult::Invalid;
	}
	else if (!WorkspotTree->IsValid())
	{
		ValidationErrors.Add(FText::FromString(TEXT("WorkspotBehaviorDefinition's WorkspotTree is invalid")));
		Result = EDataValidationResult::Invalid;
	}

	if (OccupyDuration < 0.0f)
	{
		ValidationErrors.Add(FText::FromString(TEXT("OccupyDuration cannot be negative")));
		Result = EDataValidationResult::Invalid;
	}

	if (MinimumOccupyTime < 0.0f)
	{
		ValidationErrors.Add(FText::FromString(TEXT("MinimumOccupyTime cannot be negative")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
