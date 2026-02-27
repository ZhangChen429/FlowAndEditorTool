#include "InteractionCinematicAssetEditorToolkit.h"

#include "FlowAsset.h"
#include "InteractionCinematicAsset.h"
#include "InteractionCinematicAssetEditorHelpers.h"
#include "SInteractionCinematicAssetEditorWidget.h"
#include "SInteractionCinematicFlowGraphEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "ISequencerModule.h"
#include "ISequencer.h"
#include "LevelSequence.h"
#include "MovieSceneSequence.h"
#include "GraphEditor.h"
#include "InteractionCinematicFlowAsset.h"

#define LOCTEXT_NAMESPACE "InteractionCinematicAssetEditorToolkit"

const FName FInteractionCinematicAssetEditorToolkit::MainViewTabId(TEXT("InteractionCinematicAssetEditor_MainView"));
const FName FInteractionCinematicAssetEditorToolkit::DetailsTabId(TEXT("InteractionCinematicAssetEditor_Details"));
const FName FInteractionCinematicAssetEditorToolkit::SequencerTabId(TEXT("InteractionCinematicAssetEditor_Sequencer"));
const FName FInteractionCinematicAssetEditorToolkit::FlowGraphTabId(TEXT("InteractionCinematicAssetEditor_FlowGraph"));
const FName FInteractionCinematicAssetEditorToolkit::FlowAssetDetailsTabId(TEXT("InteractionCinematicAssetEditor_FlowAssetDetails"));

FInteractionCinematicAssetEditorToolkit::FInteractionCinematicAssetEditorToolkit()
	: EditingAsset(nullptr)
{
}

FInteractionCinematicAssetEditorToolkit::~FInteractionCinematicAssetEditorToolkit()
{
	if (Sequencer.IsValid())
	{
		Sequencer->Close();
		Sequencer.Reset();
	}

	if (FlowGraphEditor.IsValid())
	{
		FlowGraphEditor.Reset();
	}
}

void FInteractionCinematicAssetEditorToolkit::Initialize(UInteractionCinematicAsset* InAsset, const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost)
{
	EditingAsset = InAsset;

	// Ensure the embedded Flow Asset and its Graph exist
	FInteractionCinematicAssetEditorHelpers::CreateFlowAssetForAsset(EditingAsset);

	// Create the editor widget
	EditorWidget = SNew(SInteractionCinematicAssetEditorWidget)
		.Asset(EditingAsset);

	// Create the details view
	CreateDetailsView();

	// Create the FlowAsset details view
	CreateFlowAssetDetailsView();

	// Define the layout with five panels:
	// Left side - MainView (top) and FlowAsset Details (bottom)
	// Right side - Details (top), Sequencer (middle), FlowGraph (bottom)
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_InteractionCinematicAssetEditor_Layout_v5")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Horizontal)
			->Split
			(
				// Left side: MainView (top) and FlowAsset Details (bottom)
				FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.3f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.6f)
					->AddTab(MainViewTabId, ETabState::OpenedTab)
					->SetHideTabWell(true)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.4f)
					->AddTab(FlowAssetDetailsTabId, ETabState::OpenedTab)
					->SetHideTabWell(true)
				)
			)
			->Split
			(
				// Right side: Details, Sequencer, FlowGraph
				FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.7f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->AddTab(DetailsTabId, ETabState::OpenedTab)
					->SetHideTabWell(true)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.4f)
					->AddTab(SequencerTabId, ETabState::OpenedTab)
					->SetHideTabWell(true)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.4f)
					->AddTab(FlowGraphTabId, ETabState::OpenedTab)
					->SetHideTabWell(true)
				)
			)
		);

	InitAssetEditor(InMode, InToolkitHost, FName(TEXT("InteractionCinematicAssetEditorApp")), StandaloneDefaultLayout, true, true, InAsset);
}

void FInteractionCinematicAssetEditorToolkit::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_InteractionCinematicAssetEditor", "Interaction Cinematic Asset Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(MainViewTabId, FOnSpawnTab::CreateSP(this, &FInteractionCinematicAssetEditorToolkit::SpawnTab_MainView))
		.SetDisplayName(LOCTEXT("MainViewTab", "Main View"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(DetailsTabId, FOnSpawnTab::CreateSP(this, &FInteractionCinematicAssetEditorToolkit::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(SequencerTabId, FOnSpawnTab::CreateSP(this, &FInteractionCinematicAssetEditorToolkit::SpawnTab_Sequencer))
		.SetDisplayName(LOCTEXT("SequencerTab", "Sequencer"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Cinematics"));

	InTabManager->RegisterTabSpawner(FlowGraphTabId, FOnSpawnTab::CreateSP(this, &FInteractionCinematicAssetEditorToolkit::SpawnTab_FlowGraph))
		.SetDisplayName(LOCTEXT("FlowGraphTab", "Flow Graph"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(FlowAssetDetailsTabId, FOnSpawnTab::CreateSP(this, &FInteractionCinematicAssetEditorToolkit::SpawnTab_FlowAssetDetails))
		.SetDisplayName(LOCTEXT("FlowAssetDetailsTab", "Flow Asset Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FInteractionCinematicAssetEditorToolkit::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(MainViewTabId);
	InTabManager->UnregisterTabSpawner(DetailsTabId);
	InTabManager->UnregisterTabSpawner(SequencerTabId);
	InTabManager->UnregisterTabSpawner(FlowGraphTabId);
	InTabManager->UnregisterTabSpawner(FlowAssetDetailsTabId);
}

FName FInteractionCinematicAssetEditorToolkit::GetToolkitFName() const
{
	return FName("InteractionCinematicAssetEditor");
}

FText FInteractionCinematicAssetEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Interaction Cinematic Asset Editor");
}

FString FInteractionCinematicAssetEditorToolkit::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "InteractionCinematicAsset ").ToString();
}

FLinearColor FInteractionCinematicAssetEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(1.0f, 0.6f, 0.4f, 0.5f);
}

TSharedRef<SDockTab> FInteractionCinematicAssetEditorToolkit::SpawnTab_MainView(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == MainViewTabId);

	return SNew(SDockTab)
		.Label(LOCTEXT("MainViewTitle", "Main View"))
		[
			EditorWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FInteractionCinematicAssetEditorToolkit::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == DetailsTabId);

	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTitle", "Details"))
		[
			DetailsView.ToSharedRef()
		];
}

TSharedRef<SDockTab> FInteractionCinematicAssetEditorToolkit::SpawnTab_Sequencer(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == SequencerTabId);

	// Create sequencer on first spawn if not already created
	if (!Sequencer.IsValid() && EditingAsset && EditingAsset->Sequence)
	{
		CreateSequencer();
	}

	TSharedRef<SWidget> SequencerWidget = SNullWidget::NullWidget;

	if (Sequencer.IsValid())
	{
		SequencerWidget = Sequencer->GetSequencerWidget();
	}
	else
	{
		// Show a message when no sequence is assigned
		SequencerWidget = SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(20.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NoSequenceMessage", "No Sequence assigned.\nPlease select a Level Sequence in the Details panel."))
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
			];
	}

	return SNew(SDockTab)
		.Label(LOCTEXT("SequencerTitle", "Sequencer"))
		[
			SequencerWidget
		];
}

TSharedRef<SDockTab> FInteractionCinematicAssetEditorToolkit::SpawnTab_FlowGraph(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FlowGraphTabId);

	TSharedRef<SWidget> FlowGraphWidget = SNullWidget::NullWidget;

	// Get the Flow Graph from the embedded FlowAsset
	UEdGraph* FlowGraph = nullptr;
	if (EditingAsset && EditingAsset->GetFlowAsset())
	{
		FlowGraph = EditingAsset->GetFlowAsset()->GetGraph();
	}

	// Create FlowGraph editor on first spawn if not already created
	if (!FlowGraphEditor.IsValid() && FlowGraph)
	{
		// Create the custom FlowGraph editor widget
		// Pass the FlowAsset Details view so it can update when nodes are selected
		FlowGraphEditor = SNew(SInteractionCinematicFlowGraphEditor, EditingAsset->GetFlowAsset())
			.DetailsView(FlowAssetDetailsView);
	}

	if (FlowGraphEditor.IsValid())
	{
		FlowGraphWidget = FlowGraphEditor.ToSharedRef();
	}
	else
	{
		// Show a message when the Flow Graph hasn't been created yet
		FlowGraphWidget = SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(20.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NoFlowGraphMessage", "Flow Graph is not available.\nThe graph will be created automatically."))
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
			];
	}

	return SNew(SDockTab)
		.Label(LOCTEXT("FlowGraphTitle", "Flow Graph"))
		[
			FlowGraphWidget
		];
}

void FInteractionCinematicAssetEditorToolkit::CreateDetailsView()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bHideSelectionTip = true;

	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(EditingAsset);
}

void FInteractionCinematicAssetEditorToolkit::CreateSequencer()
{
	if (!EditingAsset || !EditingAsset->Sequence)
	{
		return;
	}

	// Ensure we have a valid toolkit host (should be valid after InitAssetEditor)
	TSharedPtr<IToolkitHost> ToolkitHostPtr = GetToolkitHost();
	if (!ToolkitHostPtr.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateSequencer: ToolkitHost is not valid yet. Sequencer creation skipped."));
		return;
	}

	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");

	FSequencerInitParams SequencerInitParams;
	SequencerInitParams.RootSequence = EditingAsset->Sequence;
	SequencerInitParams.bEditWithinLevelEditor = false;
	SequencerInitParams.ToolkitHost = ToolkitHostPtr;
	SequencerInitParams.ViewParams.bReadOnly = false;

	Sequencer = SequencerModule.CreateSequencer(SequencerInitParams);
}

TSharedRef<SDockTab> FInteractionCinematicAssetEditorToolkit::SpawnTab_FlowAssetDetails(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FlowAssetDetailsTabId);

	TSharedRef<SWidget> DetailsWidget = SNullWidget::NullWidget;

	if (FlowAssetDetailsView.IsValid())
	{
		DetailsWidget = FlowAssetDetailsView.ToSharedRef();
	}
	else
	{
		// Show a message when the FlowAsset details view hasn't been created
		DetailsWidget = SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(20.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NoFlowAssetDetailsMessage", "FlowAsset Details is not available."))
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
			];
	}

	return SNew(SDockTab)
		.Label(LOCTEXT("FlowAssetDetailsTitle", "Flow Asset Details"))
		[
			DetailsWidget
		];
}

void FInteractionCinematicAssetEditorToolkit::CreateFlowAssetDetailsView()
{
	if (!EditingAsset || !EditingAsset->GetFlowAsset())
	{
		return;
	}

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bHideSelectionTip = true;

	FlowAssetDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	FlowAssetDetailsView->SetObject(EditingAsset->GetFlowAsset());
}

#undef LOCTEXT_NAMESPACE
