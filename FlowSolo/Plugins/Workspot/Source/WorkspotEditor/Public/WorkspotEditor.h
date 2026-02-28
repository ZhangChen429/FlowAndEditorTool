// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FAssetTypeActions_WorkspotTree;

/**
 * Workspot Editor Module
 * Provides editor tools for authoring WorkspotTree assets
 */
class FWorkspotEditorModule : public IModuleInterface
{
public:
	// IModuleInterface implementation
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterAssetTools();
	void UnregisterAssetTools();
	void RegisterDetailsCustomizations();
	void UnregisterDetailsCustomizations();

private:
	TArray<TSharedPtr<FAssetTypeActions_WorkspotTree>> CreatedAssetTypeActions;
};
