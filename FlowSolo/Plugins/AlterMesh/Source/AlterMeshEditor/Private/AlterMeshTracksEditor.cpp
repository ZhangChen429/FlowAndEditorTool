// Copyright 2023 Aechmea

#include "../Public/AlterMeshTracksEditor.h"

#include "AlterMeshActor.h"
#include "AlterMeshAsset.h"
#include "GameModeInfoCustomizer.h"
#include "StructView.h"
#include "Sections/BoolPropertySection.h"
#include "Styling/SlateIconFinder.h"

#define LOCTEXT_NAMESPACE "AlterMeshTrackEditor"

void FAlterMeshTrackMenuBuilder::ShowPickerSubMenu(FMenuBuilder& MenuBuilder, TArray<FGuid> ObjectBinding,
                                                   UMovieSceneSection* Section)
{
	MenuBuilder.BeginSection(NAME_None, LOCTEXT("AlterMeshParams", "Available Params"));

	TArray<AAlterMeshActor*> Actors;

	for (const FGuid& Binding : ObjectBinding)
	{
		TArrayView<TWeakObjectPtr<>> WeakPointers = GetSequencer()->FindObjectsInCurrentSequence(Binding);

		for (TWeakObjectPtr<> WeakPointer : WeakPointers)
		{
			if (AAlterMeshActor* Actor = Cast<AAlterMeshActor>(WeakPointer))
			{
				Actors.Add(Actor);
			}
		}
	}

	TArray<FAlterMeshStructView> Params;
	for (auto* Actor : Actors)
	{
		TArray<UObject*> Objects{Actors};
		for (FAlterMeshParamBase* Param : Actor->InputParams.GetTyped<FAlterMeshParamBase>())
		{
			
			TSubclassOf<UMovieSceneTrack> TrackClass = Param->GetTrackClass();
			if (TrackClass)
			{
				MenuBuilder.AddMenuEntry(
					FText::FromString(Param->BaseData.Name),
					FText::FromString(FString::Format(TEXT("{0} {1}"), {Param->BaseData.Id.Input.ToString(), Param->GetTrackClass()->GetFName().ToString()})),
					FSlateIcon(),
					FUIAction(
						FExecuteAction::CreateStatic(FAlterMeshTrackMenuBuilder::NewTrack,
													 FAlterMeshStructView(FAlterMeshParamBase::StaticStruct(),
																		  (uint8*)Param), GetSequencer(), ObjectBinding),
						FCanExecuteAction::CreateLambda([]() { return true; })
					),
					NAME_None,
					EUserInterfaceActionType::Button
				);
			}
		}
	}

	MenuBuilder.EndSection();
}

void FAlterMeshTrackMenuBuilder::BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder,
                                                             const TArray<FGuid>& ObjectBindings,
                                                             const UClass* ObjectClass)
{
	if (ObjectClass != nullptr && ObjectClass->IsChildOf(AAlterMeshActor::StaticClass()))
	{
		UMovieSceneSection* Section = nullptr;

		const TSharedRef<SWidget> Widget = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			  .AutoWidth()
			  .Padding(4, 2)
			[
				SNew(SImage)
				.Image(FSlateIconFinder::FindIconBrushForClass(UAlterMeshAsset::StaticClass()))
			]
			+ SHorizontalBox::Slot()
			  .AutoWidth()
			  .Padding(4, 2)
			[
				SNew(STextBlock)
			.TextStyle(MenuBuilder.GetStyleSet(), FAppStyle::Join("Menu", ".Label"))
			.Text(LOCTEXT("AddParams", "AlterMesh Params"))
			];

		MenuBuilder.AddSubMenu(
			Widget, FNewMenuDelegate::CreateRaw(this, &FAlterMeshTrackMenuBuilder::ShowPickerSubMenu, ObjectBindings,
			                                    Section));
	}
}

bool FAlterMeshFloatTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> TrackClass) const
{
	return TrackClass == UAlterMeshFloatTrack::StaticClass();
}

UMovieSceneTrack* FAlterMeshFloatTrackEditor::AddTrack(UMovieScene* FocusedMovieScene, const FGuid& ObjectHandle,
                                                       TSubclassOf<UMovieSceneTrack> TrackClass, FName UniqueTypeName)
{
	return FMovieSceneTrackEditor::AddTrack(FocusedMovieScene, ObjectHandle, TrackClass, UniqueTypeName);
}

void FAlterMeshTrackMenuBuilder::NewTrack(FAlterMeshStructView Param, TSharedPtr<ISequencer> InSequencer,
                                          TArray<FGuid> Bindings)
{
	UMovieScene* FocusedMovieScene = InSequencer->GetFocusedMovieSceneSequence()->GetMovieScene();
	if (FocusedMovieScene == nullptr)
	{
		return;
	}

	if (FocusedMovieScene->IsReadOnly())
	{
		return;
	}

	TSubclassOf<UMovieSceneTrack> TrackClass = Param.Get<FAlterMeshParamBase>().GetTrackClass();
	if (!TrackClass)
	{
		return;
	}
	
	for (auto Binding : Bindings)
	{
		FName TrackName = FName(Param.Get<FAlterMeshParamBase>().BaseData.Id.NodeGroup.ToString() + Param.Get<FAlterMeshParamBase>().BaseData.Id.Input.ToString());
		UMovieSceneTrack* Track = FocusedMovieScene->FindTrack(TrackClass, Binding, TrackName);
		if (!Track)
		{
			const FScopedTransaction Transaction(NSLOCTEXT("Sequencer", "AddAlterMeshTrack_Transaction",
			                                               "Add AlterMeshTrack"));
			FocusedMovieScene->Modify();
			UMovieSceneNameableTrack* NewTrack = Cast<UMovieSceneNameableTrack>(FocusedMovieScene->AddTrack(TrackClass, Binding));
			IAlterMeshParamSequencerInterface* SequencerParams = Cast<IAlterMeshParamSequencerInterface>(NewTrack);
			if (NewTrack)
			{
				if (SequencerParams)
				{
					SequencerParams->SetParamId(Param.Get<FAlterMeshParamBase>().BaseData.Id);			
				}
				NewTrack->SetDisplayName(FText::FromString(Param.Get<FAlterMeshParamBase>().BaseData.Name));
				NewTrack->AddSection(*NewTrack->CreateNewSection());
				InSequencer->OnAddTrack(NewTrack, Binding);
			}
		}
	}
}
