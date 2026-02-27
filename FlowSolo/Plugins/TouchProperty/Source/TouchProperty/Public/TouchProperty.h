// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/UICommandList.h"
#include "Modules/ModuleManager.h"
#include "AssetTypeCategories.h"

class IAssetTypeActions;

class FTouchPropertyModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	void RegisterMenus();

	/** Get the asset category for TouchCamera assets */
	static EAssetTypeCategories::Type GetAssetCategory() { return TouchCameraAssetCategory; }

private:
	TArray<TSharedPtr<IAssetTypeActions>> RegisteredAssetTypeActions;
	TSharedPtr< FUICommandList> ShowCaseCommands;

	/** The asset category for TouchCamera assets */
	static EAssetTypeCategories::Type TouchCameraAssetCategory;
};
