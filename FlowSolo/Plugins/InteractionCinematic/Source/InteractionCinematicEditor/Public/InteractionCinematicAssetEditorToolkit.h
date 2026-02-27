#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"

class UInteractionCinematicAsset;
class SInteractionCinematicAssetEditorWidget;
class SInteractionCinematicFlowGraphEditor;
class IDetailsView;
class ISequencer;
class SGraphEditor;

/**
 * Editor Toolkit for Interaction Cinematic Assets
 */
class FInteractionCinematicAssetEditorToolkit : public FAssetEditorToolkit
{
public:
	FInteractionCinematicAssetEditorToolkit();
	virtual ~FInteractionCinematicAssetEditorToolkit();

	/** Initialize the toolkit with the asset */
	void Initialize(UInteractionCinematicAsset* InAsset, const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost);

	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) override;
	// End of IToolkit interface

	// FAssetEditorToolkit interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	// End of FAssetEditorToolkit interface

private:
	/** Create the main widget for the editor */
	TSharedRef<SDockTab> SpawnTab_MainView(const FSpawnTabArgs& Args);

	/** Create the details panel tab */
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);

	/** Create the sequencer tab */
	TSharedRef<SDockTab> SpawnTab_Sequencer(const FSpawnTabArgs& Args);

	/** Create the FlowGraph tab */
	TSharedRef<SDockTab> SpawnTab_FlowGraph(const FSpawnTabArgs& Args);

	/** Create the FlowAsset details tab */
	TSharedRef<SDockTab> SpawnTab_FlowAssetDetails(const FSpawnTabArgs& Args);

	/** Create the details view */
	void CreateDetailsView();

	/** Create the FlowAsset details view */
	void CreateFlowAssetDetailsView();

	/** Create the sequencer */
	void CreateSequencer();

private:
	/** The asset being edited */
	UInteractionCinematicAsset* EditingAsset;

	/** The main editor widget */
	TSharedPtr<SInteractionCinematicAssetEditorWidget> EditorWidget;

	/** Details view for editing asset properties */
	TSharedPtr<IDetailsView> DetailsView;

	/** Details view for editing FlowAsset properties */
	TSharedPtr<IDetailsView> FlowAssetDetailsView;

	/** Sequencer instance for editing the level sequence */
	TSharedPtr<ISequencer> Sequencer;

	/** FlowGraph editor widget */
	TSharedPtr<SGraphEditor> FlowGraphEditor;

	/** Tab identifiers */
	static const FName MainViewTabId;
	static const FName DetailsTabId;
	static const FName SequencerTabId;
	static const FName FlowGraphTabId;
	static const FName FlowAssetDetailsTabId;
};
