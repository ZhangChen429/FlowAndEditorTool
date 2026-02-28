// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkspotComponent.h"
#include "WorkspotSubsystem.h"
#include "WorkspotInstance.h"
#include "WorkspotTree.h"
#include "Workspot.h"

UWorkspotComponent::UWorkspotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	EntryPointTag = NAME_None;
}


bool UWorkspotComponent::StartWorkspotWithTree(UWorkspotTree* Tree)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogWorkspot, Error, TEXT("WorkspotComponent::StartWorkspotWithTree - No owner actor"));
		return false;
	}

	if (!Tree)
	{
		UE_LOG(LogWorkspot, Error, TEXT("WorkspotComponent::StartWorkspotWithTree - Invalid tree"));
		return false;
	}

	// Get subsystem
	UWorkspotSubsystem* Subsystem = GetWorld()->GetSubsystem<UWorkspotSubsystem>();
	if (!Subsystem)
	{
		UE_LOG(LogWorkspot, Error, TEXT("WorkspotComponent::StartWorkspotWithTree - No WorkspotSubsystem"));
		return false;
	}

	// Start workspot through subsystem
	UWorkspotInstance* Instance = Subsystem->StartWorkspot(Owner, Tree, EntryPointTag);

	return Instance != nullptr;
}

void UWorkspotComponent::StopWorkspot(bool bForceStop)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Get subsystem
	UWorkspotSubsystem* Subsystem = GetWorld()->GetSubsystem<UWorkspotSubsystem>();
	if (!Subsystem)
	{
		return;
	}

	// Stop workspot through subsystem
	Subsystem->StopWorkspot(Owner, bForceStop);
}

bool UWorkspotComponent::IsInWorkspot() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	UWorkspotSubsystem* Subsystem = GetWorld()->GetSubsystem<UWorkspotSubsystem>();
	if (!Subsystem)
	{
		return false;
	}

	return Subsystem->IsActorInWorkspot(Owner);
}

UWorkspotInstance* UWorkspotComponent::GetActiveInstance() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	UWorkspotSubsystem* Subsystem = GetWorld()->GetSubsystem<UWorkspotSubsystem>();
	if (!Subsystem)
	{
		return nullptr;
	}

	return Subsystem->GetActiveWorkspot(Owner);
}
