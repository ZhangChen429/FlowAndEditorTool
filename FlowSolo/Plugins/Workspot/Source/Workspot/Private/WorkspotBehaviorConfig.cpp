// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkspotBehaviorConfig.h"
#include "WorkspotTree.h"
#include "WorkspotGameplayBehavior.h"
#include "Workspot.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

UWorkspotBehaviorConfig::UWorkspotBehaviorConfig()
{
	// Set the behavior class to our Workspot behavior
	BehaviorClass = UGameplayBehavior_Workspot::StaticClass();

	// Default configuration
	WorkspotTree = nullptr;
	bAutoSelectClosestEntry = true;
	OccupyDuration = 0.0f;
	MinimumOccupyTime = 3.0f;
	bAllowFastExit = true;
	bTakeCharacterControl = true;
}

bool UWorkspotBehaviorConfig::IsValid() const
{
	if (!WorkspotTree)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("WorkspotBehaviorConfig: No WorkspotTree assigned"));
		return false;
	}

	if (!WorkspotTree->IsValid())
	{
		UE_LOG(LogWorkspot, Warning, TEXT("WorkspotBehaviorConfig: WorkspotTree is invalid"));
		return false;
	}

	if (OccupyDuration < 0.0f)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("WorkspotBehaviorConfig: OccupyDuration cannot be negative"));
		return false;
	}

	if (MinimumOccupyTime < 0.0f)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("WorkspotBehaviorConfig: MinimumOccupyTime cannot be negative"));
		return false;
	}

	return true;
}

#if WITH_EDITOR
EDataValidationResult UWorkspotBehaviorConfig::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!WorkspotTree)
	{
		Context.AddError(FText::FromString(TEXT("WorkspotBehaviorConfig has no WorkspotTree assigned")));
		Result = EDataValidationResult::Invalid;
	}
	else if (!WorkspotTree->IsValid())
	{
		Context.AddError(FText::FromString(TEXT("WorkspotBehaviorConfig's WorkspotTree is invalid")));
		Result = EDataValidationResult::Invalid;
	}

	if (OccupyDuration < 0.0f)
	{
		Context.AddError(FText::FromString(TEXT("OccupyDuration cannot be negative")));
		Result = EDataValidationResult::Invalid;
	}

	if (MinimumOccupyTime < 0.0f)
	{
		Context.AddError(FText::FromString(TEXT("MinimumOccupyTime cannot be negative")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
