// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkspotTreeFactory.h"
#include "WorkspotTree.h"
#include "WorkspotEntry.h"

#define LOCTEXT_NAMESPACE "WorkspotTreeFactory"

UWorkspotTreeFactory::UWorkspotTreeFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UWorkspotTree::StaticClass();
}

UObject* UWorkspotTreeFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UWorkspotTree::StaticClass()));

	UWorkspotTree* NewTree = NewObject<UWorkspotTree>(InParent, Class, Name, Flags);

	if (NewTree)
	{
		// Create a default root sequence
		UWorkspotSequence* RootSequence = NewObject<UWorkspotSequence>(NewTree, NAME_None, RF_Transactional);
		RootSequence->IdleAnim = TEXT("stand");
		NewTree->RootEntry = RootSequence;

		// Set default values
		NewTree->DefaultTransitionBlendTime = 0.3f;
		NewTree->InertializationDurationEnter = 0.5f;
		NewTree->InertializationDurationExitNatural = 0.5f;
		NewTree->InertializationDurationExitForced = 0.2f;

		UE_LOG(LogTemp, Log, TEXT("Created new WorkspotTree: %s"), *Name.ToString());
	}

	return NewTree;
}

bool UWorkspotTreeFactory::ShouldShowInNewMenu() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE
