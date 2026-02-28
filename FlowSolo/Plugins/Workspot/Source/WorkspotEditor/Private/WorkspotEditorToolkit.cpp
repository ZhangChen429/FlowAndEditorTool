// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkspotEditorToolkit.h"
#include "WorkspotTree.h"
#include "SWorkspotTreeView.h"
#include "Widgets/Docking/SDockTab.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "WorkspotEditorToolkit"

// Tab identifiers
const FName FWorkspotEditorToolkit::TreeViewTabId(TEXT("WorkspotEditor_TreeView"));
const FName FWorkspotEditorToolkit::DetailsTabId(TEXT("WorkspotEditor_Details"));
const FName FWorkspotEditorToolkit::PreviewTabId(TEXT("WorkspotEditor_Preview"));

FWorkspotEditorToolkit::FWorkspotEditorToolkit()
	: WorkspotTree(nullptr)
{
}

FWorkspotEditorToolkit::~FWorkspotEditorToolkit()
{
}

void FWorkspotEditorToolkit::InitWorkspotEditor(
	const EToolkitMode::Type Mode,
	const TSharedPtr<IToolkitHost>& InitToolkitHost,
	UWorkspotTree* ObjectToEdit)
{
	WorkspotTree = ObjectToEdit;

	// Create internal widgets
	CreateInternalWidgets();

	// Define the layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_WorkspotEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.6f)
					->AddTab(TreeViewTabId, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.4f)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.6f)
						->AddTab(DetailsTabId, ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.4f)
						->AddTab(PreviewTabId, ETabState::OpenedTab)
					)
				)
			)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;

	FAssetEditorToolkit::InitAssetEditor(
		Mode,
		InitToolkitHost,
		FName("WorkspotEditorApp"),
		StandaloneDefaultLayout,
		bCreateDefaultStandaloneMenu,
		bCreateDefaultToolbar,
		ObjectToEdit);

	// Setup commands and toolbar
	BindCommands();
	ExtendToolbar();
}

UWorkspotTree* FWorkspotEditorToolkit::GetWorkspotTree() const
{
	return WorkspotTree.Get();
}

void FWorkspotEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_WorkspotEditor", "Workspot Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(TreeViewTabId, FOnSpawnTab::CreateSP(this, &FWorkspotEditorToolkit::SpawnTab_TreeView))
		.SetDisplayName(LOCTEXT("TreeViewTab", "Tree View"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Outliner"));

	InTabManager->RegisterTabSpawner(DetailsTabId, FOnSpawnTab::CreateSP(this, &FWorkspotEditorToolkit::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(PreviewTabId, FOnSpawnTab::CreateSP(this, &FWorkspotEditorToolkit::SpawnTab_Preview))
		.SetDisplayName(LOCTEXT("PreviewTab", "Preview"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));
}

void FWorkspotEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(TreeViewTabId);
	InTabManager->UnregisterTabSpawner(DetailsTabId);
	InTabManager->UnregisterTabSpawner(PreviewTabId);
}

FName FWorkspotEditorToolkit::GetToolkitFName() const
{
	return FName("WorkspotEditor");
}

FText FWorkspotEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Workspot Editor");
}

FString FWorkspotEditorToolkit::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Workspot ").ToString();
}

FLinearColor FWorkspotEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.0f, 0.6f, 0.6f, 0.5f);
}

void FWorkspotEditorToolkit::CreateInternalWidgets()
{
	// Create details view
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(WorkspotTree.Get());

	// Create tree view widget
	TreeViewWidget = SNew(SWorkspotTreeView)
		.WorkspotTree(WorkspotTree.Get());
}

void FWorkspotEditorToolkit::BindCommands()
{
	// TODO: Bind editor commands here
	// Commands will be added as the editor is developed
}

void FWorkspotEditorToolkit::ExtendToolbar()
{
	// TODO: Add custom toolbar buttons
	// Toolbar extensions will be added as needed
}

TSharedRef<SDockTab> FWorkspotEditorToolkit::SpawnTab_TreeView(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == TreeViewTabId);

	return SNew(SDockTab)
		.Label(LOCTEXT("TreeViewTitle", "Tree View"))
		.TabColorScale(GetTabColorScale())
		[
			TreeViewWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FWorkspotEditorToolkit::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == DetailsTabId);

	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTitle", "Details"))
		.TabColorScale(GetTabColorScale())
		[
			DetailsView.ToSharedRef()
		];
}

TSharedRef<SDockTab> FWorkspotEditorToolkit::SpawnTab_Preview(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == PreviewTabId);

	// TODO: Create preview viewport widget
	return SNew(SDockTab)
		.Label(LOCTEXT("PreviewTitle", "Preview"))
		.TabColorScale(GetTabColorScale())
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PreviewPlaceholder", "Preview viewport will be added here"))
		];
}

#undef LOCTEXT_NAMESPACE
