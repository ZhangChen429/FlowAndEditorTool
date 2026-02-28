// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "WorkspotTreeFactory.generated.h"

/**
 * Factory for creating WorkspotTree assets in the Content Browser
 */
UCLASS()
class WORKSPOTEDITOR_API UWorkspotTreeFactory : public UFactory
{
	GENERATED_BODY()

public:
	UWorkspotTreeFactory();

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
};
