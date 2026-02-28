// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkits/IToolkitHost.h"

class UWorkspotTree;
class IDetailsView;
class SWorkspotTreeView;

/**
 * Workspot Editor Toolkit
 * Custom editor for authoring WorkspotTree assets
 *
 * Provides:
 * - Visual graph view for Entry tree composition
 * - Details panel for editing properties
 * - Preview viewport for testing animations
 * - Extensible tab system for additional Slate widgets
 */
class FWorkspotEditorToolkit : public FAssetEditorToolkit
{
public:
	FWorkspotEditorToolkit();
	virtual ~FWorkspotEditorToolkit();

	/**
	 * Initialize the editor toolkit
	 * @param Mode The mode to open the editor in
	 * @param InitToolkitHost The toolkit host
	 * @param ObjectToEdit The WorkspotTree to edit
	 */
	void InitWorkspotEditor(
		const EToolkitMode::Type Mode,
		const TSharedPtr<class IToolkitHost>& InitToolkitHost,
		UWorkspotTree* ObjectToEdit);

	/** Get the WorkspotTree being edited */
	UWorkspotTree* GetWorkspotTree() const;

public:
	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	// End of IToolkit interface

	// FAssetEditorToolkit interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	// End of FAssetEditorToolkit interface

private:
	/** Create the internal widgets for the asset editor */
	void CreateInternalWidgets();

	/** Build toolbar for the asset editor */
	void BindCommands();
	void ExtendToolbar();

	/** Tab spawners */
	TSharedRef<SDockTab> SpawnTab_TreeView(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Preview(const FSpawnTabArgs& Args);

private:
	/** The WorkspotTree being edited */
	TObjectPtr<UWorkspotTree> WorkspotTree;

	/** Details view for editing WorkspotTree properties */
	TSharedPtr<IDetailsView> DetailsView;

	/** Tree view widget for visualizing Entry hierarchy */
	TSharedPtr<SWorkspotTreeView> TreeViewWidget;

	/** Tab identifiers */
	static const FName TreeViewTabId;
	static const FName DetailsTabId;
	static const FName PreviewTabId;
};
