// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyle.h"
#include "Widgets/Docking/SDockTab.h"
#include "Templates/SharedPointer.h"

class UDetailsView;
struct FAlterMeshBrowser;

class FAlterMeshEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void RegisterComponentVisualizer(FName ComponentClassName, TSharedPtr<FComponentVisualizer> Visualizer);
	TSharedRef<SDockTab> SpawnAlterMeshDetailsPanel(const FSpawnTabArgs& Args);
	
	static TSharedRef<FExtender> OnExtendLevelEditorActorContextMenu(TSharedRef<FUICommandList> FuiCommands, TArray<AActor*> Actors);
	
	TSharedPtr<FSlateStyleSet> AlterMeshStyleSet;
	TSharedPtr<IDetailsView> DetailsView;
	FDelegateHandle LevelViewportExtenderHandle;

	void SelectionChanged(const TArray<UObject*>& Objects, bool bArg);

	TArray<TWeakObjectPtr<AActor>> AlterMeshPanelActors;
	
private:
	TArray<FName> RegisteredComponentClassNames;
	TSharedPtr<FAlterMeshBrowser> Browser;
};
