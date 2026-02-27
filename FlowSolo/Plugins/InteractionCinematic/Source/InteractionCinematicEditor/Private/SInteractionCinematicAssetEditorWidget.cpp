#include "SInteractionCinematicAssetEditorWidget.h"
#include "InteractionCinematicAsset.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSeparator.h"
#include "LevelSequence.h"

#define LOCTEXT_NAMESPACE "SInteractionCinematicAssetEditorWidget"

void SInteractionCinematicAssetEditorWidget::Construct(const FArguments& InArgs)
{
	Asset = InArgs._Asset;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(10.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(5.0f)
			[
				SNew(SVerticalBox)

				// Title
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 10)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("EditorTitle", "Interaction Cinematic Asset Editor"))
					.Font(FAppStyle::GetFontStyle("HeadingExtraSmall"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 5)
				[
					SNew(SSeparator)
				]

				// Sequence Information
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 10, 0, 5)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("SequenceLabel", "Level Sequence:"))
					.Font(FAppStyle::GetFontStyle("PropertyWindow.BoldFont"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 10)
				[
					SNew(STextBlock)
					.Text(this, &SInteractionCinematicAssetEditorWidget::GetSequenceNameText)
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 5)
				[
					SNew(SSeparator)
				]

				// Description
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 10, 0, 5)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("DescriptionLabel", "Description:"))
					.Font(FAppStyle::GetFontStyle("PropertyWindow.BoldFont"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 10)
				[
					SNew(STextBlock)
					.Text(this, &SInteractionCinematicAssetEditorWidget::GetDescriptionText)
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.AutoWrapText(true)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 5)
				[
					SNew(SSeparator)
				]

				// Info text
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(0, 10)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("InfoText", "Use the Details panel to edit the Sequence reference and Description."))
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					.AutoWrapText(true)
				]
			]
		]
	];
}

FText SInteractionCinematicAssetEditorWidget::GetSequenceNameText() const
{
	if (Asset && Asset->Sequence)
	{
		return FText::FromString(Asset->Sequence->GetName());
	}
	return LOCTEXT("NoSequence", "No Sequence Assigned");
}

FText SInteractionCinematicAssetEditorWidget::GetDescriptionText() const
{
	if (Asset && !Asset->Description.IsEmpty())
	{
		return Asset->Description;
	}
	return LOCTEXT("NoDescription", "No Description");
}

#undef LOCTEXT_NAMESPACE
