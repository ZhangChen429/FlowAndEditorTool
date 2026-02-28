// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"

class UWorkspotTree;
class UWorkspotEntry;

/**
 * Tree item for Entry hierarchy visualization
 */
struct FWorkspotTreeViewItem
{
	/** The entry this item represents */
	TWeakObjectPtr<UWorkspotEntry> Entry;

	/** Display name */
	FText DisplayName;

	/** Child items */
	TArray<TSharedPtr<FWorkspotTreeViewItem>> Children;

	FWorkspotTreeViewItem(UWorkspotEntry* InEntry);
};

/**
 * Slate widget for visualizing WorkspotTree Entry hierarchy
 *
 * Displays the Entry tree in a hierarchical tree view, showing:
 * - Entry types (AnimClip, Sequence, Selector, etc.)
 * - Idle states
 * - Entry properties
 *
 * Supports:
 * - Selection
 * - Drag & drop (future)
 * - Context menus (future)
 * - Inline editing (future)
 */
class SWorkspotTreeView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SWorkspotTreeView)
		: _WorkspotTree(nullptr)
	{}
		SLATE_ARGUMENT(UWorkspotTree*, WorkspotTree)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Refresh the tree view from the WorkspotTree */
	void RefreshTreeView();

private:
	/** Generate a row widget for a tree item */
	TSharedRef<ITableRow> GenerateRowWidget(TSharedPtr<FWorkspotTreeViewItem> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** Get children of a tree item */
	void GetChildrenForItem(TSharedPtr<FWorkspotTreeViewItem> Item, TArray<TSharedPtr<FWorkspotTreeViewItem>>& OutChildren);

	/** Handle selection changed */
	void OnSelectionChanged(TSharedPtr<FWorkspotTreeViewItem> Item, ESelectInfo::Type SelectInfo);

	/** Build tree items from Entry hierarchy */
	void BuildTreeItems();
	void BuildTreeItemsRecursive(UWorkspotEntry* Entry, TSharedPtr<FWorkspotTreeViewItem> ParentItem);

private:
	/** The WorkspotTree being visualized */
	TWeakObjectPtr<UWorkspotTree> WorkspotTree;

	/** Root items for the tree view */
	TArray<TSharedPtr<FWorkspotTreeViewItem>> RootItems;

	/** Tree view widget */
	TSharedPtr<STreeView<TSharedPtr<FWorkspotTreeViewItem>>> TreeView;
};
