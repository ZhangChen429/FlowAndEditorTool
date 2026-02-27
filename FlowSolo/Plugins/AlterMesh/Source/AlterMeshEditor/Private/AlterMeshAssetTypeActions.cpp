// Copyright 2023 Aechmea

#include "AlterMeshAssetTypeActions.h"

#include "AlterMeshAsset.h"
#include "AlterMeshAssetEditor.h"
#include "AlterMeshParam.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Brushes/SlateImageBrush.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateIconFinder.h"

void FAlterMeshAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	if (InObjects.Num() == 1)
	{
		MenuBuilder.AddMenuEntry(
			NSLOCTEXT("AMTypeActions", "AlterMesh_CreateInstance", "Create Instance"),
			NSLOCTEXT("AMTypeActions", "AlterMesh_CreateInstanceTooltip", "Create a child Instance to save your modified params"),
			FSlateIcon("AlterMesh", "ClassIcon.AlterMeshAsset"),
			FUIAction(
			FExecuteAction::CreateLambda(
				[Asset = Cast<UAlterMeshAssetInterface>(InObjects[0])]()
				{
					UAlterMeshAssetInstance::CreateInstance(Asset->Get(), Asset->GetParams());
				}),
			FCanExecuteAction()
			)
		);
	}
}

FText FAlterMeshAssetTypeActions::GetName() const
{
	return NSLOCTEXT("AlterMeshAssetTypeActions", "FAlterMeshAssetTypeActionsName", "AlterMesh");
}

UClass* FAlterMeshAssetTypeActions::GetSupportedClass() const
{
	return UAlterMeshAssetInterface::StaticClass();
}

FColor FAlterMeshAssetTypeActions::GetTypeColor() const
{
	return FColor(5, 239, 227);
}

uint32 FAlterMeshAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::None;
}

TSharedPtr<SWidget> FAlterMeshAssetTypeActions::GetThumbnailOverlay(const FAssetData& AssetData) const
{
	const FSlateBrush* Icon = FSlateIconFinder::FindIconBrushForClass(UAlterMeshAsset::StaticClass());
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetNoBrush())
		.Visibility(EVisibility::HitTestInvisible)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 3.0f))
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		[
			SNew(SImage)
			.Image(Icon)
		];
}


void FAlterMeshAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	for (UObject* Object : InObjects)
	{
		UAlterMeshAssetInterface* Asset = Cast<UAlterMeshAssetInterface>(Object);
		if (Asset != nullptr)
		{
			TSharedRef<FAlterMeshAssetEditor> NewEditor(new FAlterMeshAssetEditor());
			NewEditor->InitAlterMeshAssetEditor(Mode, EditWithinLevelEditor, Asset);
		}
	}
}