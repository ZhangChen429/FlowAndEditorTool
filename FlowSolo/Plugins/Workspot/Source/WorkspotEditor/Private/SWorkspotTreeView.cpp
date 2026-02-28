// Copyright Epic Games, Inc. All Rights Reserved.

#include "SWorkspotTreeView.h"
#include "WorkspotTree.h"
#include "WorkspotEntry.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"

#include "Widgets/SBoxPanel.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "SWorkspotTreeView"

//////////////////////////////////////////////////////////////////////////
// FWorkspotTreeViewItem
//////////////////////////////////////////////////////////////////////////

FWorkspotTreeViewItem::FWorkspotTreeViewItem(UWorkspotEntry* InEntry)
	: Entry(InEntry)
{
	if (InEntry)
	{
		// Generate display name from entry type and properties
		FString TypeName = InEntry->GetClass()->GetName();
		TypeName.RemoveFromStart("Workspot");

		DisplayName = FText::Format(
			LOCTEXT("EntryDisplayFormat", "{0} ({1})"),
			FText::FromString(TypeName),
			FText::FromName(InEntry->IdleAnim)
		);
	}
}

//////////////////////////////////////////////////////////////////////////
// SWorkspotTreeView
//////////////////////////////////////////////////////////////////////////

void SWorkspotTreeView::Construct(const FArguments& InArgs)
{
	WorkspotTree = InArgs._WorkspotTree;

	// Build tree structure
	BuildTreeItems();

	// Create tree view widget
	ChildSlot
	[
		SNew(SVerticalBox)

		// Toolbar
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TreeViewHeader", "Entry Hierarchy"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			]
		]

		// Tree view
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(TreeView, STreeView<TSharedPtr<FWorkspotTreeViewItem>>)
			.TreeItemsSource(&RootItems)
			.OnGenerateRow(this, &SWorkspotTreeView::GenerateRowWidget)
			.OnGetChildren(this, &SWorkspotTreeView::GetChildrenForItem)
			.OnSelectionChanged(this, &SWorkspotTreeView::OnSelectionChanged)
			.SelectionMode(ESelectionMode::Single)
		]
	];

	// Expand all items by default
	for (const TSharedPtr<FWorkspotTreeViewItem>& Item : RootItems)
	{
		TreeView->SetItemExpansion(Item, true);
	}
}

void SWorkspotTreeView::RefreshTreeView()
{
	BuildTreeItems();

	if (TreeView.IsValid())
	{
		TreeView->RequestTreeRefresh();
	}
}

TSharedRef<ITableRow> SWorkspotTreeView::GenerateRowWidget(
	TSharedPtr<FWorkspotTreeViewItem> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!Item.IsValid() || !Item->Entry.IsValid())
	{
		return SNew(STableRow<TSharedPtr<FWorkspotTreeViewItem>>, OwnerTable)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("InvalidEntry", "Invalid Entry"))
			];
	}

	UWorkspotEntry* Entry = Item->Entry.Get();

	// Determine icon based on entry type
	const FSlateBrush* IconBrush = nullptr;
	if (Cast<UWorkspotSequence>(Entry))
	{
		IconBrush = FAppStyle::GetBrush("Graph.SequencePlayer");
	}
	else if (Cast<UWorkspotSelector>(Entry))
	{
		IconBrush = FAppStyle::GetBrush("Graph.Selector");
	}
	else if (Cast<UWorkspotRandomList>(Entry))
	{
		IconBrush = FAppStyle::GetBrush("Graph.Random");
	}
	else
	{
		IconBrush = FAppStyle::GetBrush("Graph.AnimationResultNode");
	}

	return SNew(STableRow<TSharedPtr<FWorkspotTreeViewItem>>, OwnerTable)
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(IconBrush)
				.ColorAndOpacity(FSlateColor::UseForeground())
			]

			// Name
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(4.0f, 2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Item->DisplayName)
			]
		];
}

void SWorkspotTreeView::GetChildrenForItem(
	TSharedPtr<FWorkspotTreeViewItem> Item,
	TArray<TSharedPtr<FWorkspotTreeViewItem>>& OutChildren)
{
	if (Item.IsValid())
	{
		OutChildren = Item->Children;
	}
}

void SWorkspotTreeView::OnSelectionChanged(
	TSharedPtr<FWorkspotTreeViewItem> Item,
	ESelectInfo::Type SelectInfo)
{
	// TODO: Update details panel or other views based on selection
}

void SWorkspotTreeView::BuildTreeItems()
{
	RootItems.Empty();

	if (UWorkspotTree* Tree = WorkspotTree.Get())
	{
		if (Tree->RootEntry)
		{
			TSharedPtr<FWorkspotTreeViewItem> RootItem = MakeShared<FWorkspotTreeViewItem>(Tree->RootEntry);
			BuildTreeItemsRecursive(Tree->RootEntry, RootItem);
			RootItems.Add(RootItem);
		}
	}
}

void SWorkspotTreeView::BuildTreeItemsRecursive(
	UWorkspotEntry* Entry,
	TSharedPtr<FWorkspotTreeViewItem> ParentItem)
{
	if (!Entry || !ParentItem.IsValid())
	{
		return;
	}

	// Handle container entries
	if (UWorkspotSequence* Sequence = Cast<UWorkspotSequence>(Entry))
	{
		for (UWorkspotEntry* ChildEntry : Sequence->Entries)
		{
			if (ChildEntry)
			{
				TSharedPtr<FWorkspotTreeViewItem> ChildItem = MakeShared<FWorkspotTreeViewItem>(ChildEntry);
				ParentItem->Children.Add(ChildItem);
				BuildTreeItemsRecursive(ChildEntry, ChildItem);
			}
		}
	}
	else if (UWorkspotRandomList* RandomList = Cast<UWorkspotRandomList>(Entry))
	{
		for (UWorkspotEntry* ChildEntry : RandomList->Entries)
		{
			if (ChildEntry)
			{
				TSharedPtr<FWorkspotTreeViewItem> ChildItem = MakeShared<FWorkspotTreeViewItem>(ChildEntry);
				ParentItem->Children.Add(ChildItem);
				BuildTreeItemsRecursive(ChildEntry, ChildItem);
			}
		}
	}
	else if (UWorkspotSelector* Selector = Cast<UWorkspotSelector>(Entry))
	{
		for (UWorkspotEntry* ChildEntry : Selector->Entries)
		{
			if (ChildEntry)
			{
				TSharedPtr<FWorkspotTreeViewItem> ChildItem = MakeShared<FWorkspotTreeViewItem>(ChildEntry);
				ParentItem->Children.Add(ChildItem);
				BuildTreeItemsRecursive(ChildEntry, ChildItem);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
