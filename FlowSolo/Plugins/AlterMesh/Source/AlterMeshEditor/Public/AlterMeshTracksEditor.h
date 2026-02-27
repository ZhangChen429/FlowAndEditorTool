// Copyright 2023 Aechmea

#pragma once

#include "AlterMeshTracks.h"
#include "PropertyTrackEditor.h"

class FAlterMeshTrackMenuBuilder
	: public FMovieSceneTrackEditor
{
	
public:
	FAlterMeshTrackMenuBuilder(TSharedRef<ISequencer> InSequencer)
		: FMovieSceneTrackEditor(InSequencer)
	{ }
	
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer)
	{
		return MakeShareable(new FAlterMeshTrackMenuBuilder(OwningSequencer));
	}

	virtual bool SupportsType(TSubclassOf<UMovieSceneTrack> TrackClass) const override
	{
		return false;
	}
	
	static void NewTrack(FAlterMeshStructView Param, TSharedPtr<ISequencer> InSequencer, TArray<FGuid> Bindings);
	
protected:
	
	void ShowPickerSubMenu(FMenuBuilder& MenuBuilder, TArray<FGuid> ObjectBinding, UMovieSceneSection* Section);
	virtual void BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const TArray<FGuid>& ObjectBindings, const UClass* ObjectClass) override;
	
};

class FAlterMeshBoolTrackEditor
	: public FPropertyTrackEditor<UAlterMeshBoolTrack>
{
public:
	
	FAlterMeshBoolTrackEditor(TSharedRef<ISequencer> InSequencer)
		: FPropertyTrackEditor(InSequencer, { FAnimatedPropertyKey::FromPropertyTypeName(NAME_BoolProperty) })
	{ }
	
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer)
	{
		return MakeShareable(new FAlterMeshBoolTrackEditor(OwningSequencer));
	}
	
protected:
	
	//~ FPropertyTrackEditor interface
	virtual void GenerateKeysFromPropertyChanged(const FPropertyChangedParams& PropertyChangedParams, UMovieSceneSection* SectionToKey, FGeneratedTrackKeys& OutGeneratedKeys) override
	{
		const bool KeyedValue = PropertyChangedParams.GetPropertyValue<bool>();
		OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneBoolChannel>(0, KeyedValue, true));
	}
	
	virtual bool ForCustomizedUseOnly() override { return true; };
	
};

class FAlterMeshFloatTrackEditor
	: public FMovieSceneTrackEditor
{
public:

	FAlterMeshFloatTrackEditor(TSharedRef<ISequencer> InSequencer)
		: FMovieSceneTrackEditor(InSequencer)
	{
		
	}
	
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer)
	{
		return MakeShareable(new FAlterMeshFloatTrackEditor(OwningSequencer));
	}

	virtual bool SupportsType(TSubclassOf<UMovieSceneTrack> TrackClass) const override;
	virtual UMovieSceneTrack* AddTrack(UMovieScene* FocusedMovieScene, const FGuid& ObjectHandle, TSubclassOf<UMovieSceneTrack> TrackClass, FName UniqueTypeName) override;
};

class FAlterMeshIntegerTrackEditor
	: public FPropertyTrackEditor<UAlterMeshIntegerTrack>
{
public:

	FAlterMeshIntegerTrackEditor(TSharedRef<ISequencer> InSequencer)
		: FPropertyTrackEditor(InSequencer, { FAnimatedPropertyKey::FromPropertyTypeName(NAME_IntProperty) })
	{ }
	
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer)
	{
		return MakeShareable(new FAlterMeshIntegerTrackEditor(OwningSequencer));
	}

protected:
	
	//~ FPropertyTrackEditor interface
	virtual void GenerateKeysFromPropertyChanged(const FPropertyChangedParams& PropertyChangedParams, UMovieSceneSection* SectionToKey, FGeneratedTrackKeys& OutGeneratedKeys) override
	{
		const int32 KeyedValue = PropertyChangedParams.GetPropertyValue<int32>();
		OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneIntegerChannel>(0, KeyedValue, true));
	}
	virtual bool ForCustomizedUseOnly() override { return true; };
	
};

class FAlterMeshColorTrackEditor
	: public FPropertyTrackEditor<UAlterMeshColorTrack>
{
public:

	FAlterMeshColorTrackEditor(TSharedRef<ISequencer> InSequencer)
		: FPropertyTrackEditor(InSequencer, { FAnimatedPropertyKey::FromStructType("LinearColor") })
	{ }
	
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer)
	{
		return MakeShareable(new FAlterMeshColorTrackEditor(OwningSequencer));
	}

protected:
	
	//~ FPropertyTrackEditor interface
	virtual void GenerateKeysFromPropertyChanged(const FPropertyChangedParams& PropertyChangedParams, UMovieSceneSection* SectionToKey, FGeneratedTrackKeys& OutGeneratedKeys) override
	{
		const FLinearColor KeyedValue = PropertyChangedParams.GetPropertyValue<FLinearColor>();
		OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneFloatChannel>(0, KeyedValue.R, true));
		OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneFloatChannel>(1, KeyedValue.G, true));
		OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneFloatChannel>(2, KeyedValue.B, true));
		OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneFloatChannel>(3, KeyedValue.A, true));
	}
	virtual bool ForCustomizedUseOnly() override { return true; };
	
};

class FAlterMeshStringTrackEditor
	: public FPropertyTrackEditor<UAlterMeshStringTrack>
{
public:

	FAlterMeshStringTrackEditor(TSharedRef<ISequencer> InSequencer)
		: FPropertyTrackEditor(InSequencer, { FAnimatedPropertyKey::FromPropertyTypeName(NAME_StrProperty) })
	{ }
	
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer)
	{
		return MakeShareable(new FAlterMeshStringTrackEditor(OwningSequencer));
	}

protected:
	
	//~ FPropertyTrackEditor interface
	virtual void GenerateKeysFromPropertyChanged(const FPropertyChangedParams& PropertyChangedParams, UMovieSceneSection* SectionToKey, FGeneratedTrackKeys& OutGeneratedKeys) override
	{
		const FString KeyedValue = PropertyChangedParams.GetPropertyValue<FString>();
		OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneStringChannel>(0, KeyedValue, true));
	}
	virtual bool ForCustomizedUseOnly() override { return true; };
	
};

class FAlterMeshVectorTrackEditor
	: public FPropertyTrackEditor<UAlterMeshVectorTrack>
{
public:

	FAlterMeshVectorTrackEditor(TSharedRef<ISequencer> InSequencer)
	: FPropertyTrackEditor(InSequencer, {FAnimatedPropertyKey::FromStructType(NAME_Vector)})
	{
		
	}
	
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer)
	{
		return MakeShareable(new FAlterMeshVectorTrackEditor(OwningSequencer));
	}

protected:
	
	//~ FPropertyTrackEditor interface
	virtual void GenerateKeysFromPropertyChanged(const FPropertyChangedParams& PropertyChangedParams, UMovieSceneSection* SectionToKey, FGeneratedTrackKeys& OutGeneratedKeys) override
	{
		const FVector KeyedValue = PropertyChangedParams.GetPropertyValue<FVector>();
		OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneDoubleChannel>(0, KeyedValue.X, true));
		OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneDoubleChannel>(1, KeyedValue.Y, true));
		OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneDoubleChannel>(2, KeyedValue.Z, true));
	}
	virtual bool ForCustomizedUseOnly() override { return true; };
	
};

class FAlterMeshRotationTrackEditor
	: public FPropertyTrackEditor<UAlterMeshRotationTrack>
{
public:

	FAlterMeshRotationTrackEditor(TSharedRef<ISequencer> InSequencer)
	: FPropertyTrackEditor(InSequencer, {FAnimatedPropertyKey::FromStructType(NAME_Rotation)})
	{
		
	}
	
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer)
	{
		return MakeShareable(new FAlterMeshRotationTrackEditor(OwningSequencer));
	}

protected:
	
	//~ FPropertyTrackEditor interface
	virtual void GenerateKeysFromPropertyChanged(const FPropertyChangedParams& PropertyChangedParams, UMovieSceneSection* SectionToKey, FGeneratedTrackKeys& OutGeneratedKeys) override
	{
		const FRotator KeyedValue = PropertyChangedParams.GetPropertyValue<FRotator>();
		OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneDoubleChannel>(0, KeyedValue.Roll, true));
		OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneDoubleChannel>(1, KeyedValue.Pitch, true));
		OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneDoubleChannel>(2, KeyedValue.Yaw, true));
	}
	virtual bool ForCustomizedUseOnly() override { return true; };
	
};