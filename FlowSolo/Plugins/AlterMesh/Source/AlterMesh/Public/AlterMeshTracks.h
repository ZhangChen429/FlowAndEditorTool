// Copyright 2023 Aechmea

#pragma once

#include "AlterMeshActor.h"
#include "PropertyPathHelpers.h"
#include "CommonParams/AlterMeshBoolParam.h"
#include "CommonParams/AlterMeshColorParam.h"
#include "CommonParams/AlterMeshFloatParam.h"
#include "CommonParams/AlterMeshIntParam.h"
#include "CommonParams/AlterMeshStringParam.h"
#include "CommonParams/AlterMeshVectorParam.h"
#include "Compilation/IMovieSceneTemplateGenerator.h"
#include "Evaluation/MovieScenePropertyTemplate.h"
#include "Tracks/MovieSceneBoolTrack.h"
#include "Sections/MovieSceneBoolSection.h"
#include "Sections/MovieSceneColorSection.h"
#include "Sections/MovieSceneFloatSection.h"
#include "Sections/MovieSceneIntegerSection.h"
#include "Sections/MovieSceneStringSection.h"
#include "Sections/MovieSceneVectorSection.h"
#include "Tracks/MovieSceneColorTrack.h"
#include "Tracks/MovieSceneFloatTrack.h"
#include "Tracks/MovieSceneIntegerTrack.h"
#include "Tracks/MovieSceneStringTrack.h"
#include "Tracks/MovieSceneVectorTrack.h"
#include "AlterMeshStructArrayView.h"
#include "CommonParams/AlterMeshRotationParam.h"
#include "AlterMeshTracks.generated.h"

static FMovieSceneSharedDataId FAlterMeshExecutionTokenStaticId = FMovieSceneSharedDataId::Allocate();

struct TAlterMeshRefreshExecutionToken : IMovieSceneSharedExecutionToken
{
	TAlterMeshRefreshExecutionToken(const FMovieSceneEvaluationOperand& InOperand)
	{
		Operands.Add(InOperand);
	}
	
	virtual void Execute(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) override;
	
	TSet<FMovieSceneEvaluationOperand> Operands;
};

template <typename StructType, typename ValueType>
struct TAlterMeshParamExecutionToken : IMovieSceneExecutionToken
{	
	TAlterMeshParamExecutionToken(ValueType InValue, FAlterMeshParamId InParamId)
	{
		ParamId = InParamId;
		Value = InValue;
	}

	virtual void Execute(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) override
	{		
		for (TWeakObjectPtr<> WeakObject : Player.FindBoundObjects(Operand))
		{
			if (AAlterMeshActor* Actor = Cast<AAlterMeshActor>(WeakObject))
			{
				for (FAlterMeshStructView Param : FAlterMeshStructArrayView(Actor->InputParams.Params))
				{
					if (Param.Get<FInstancedStruct>().GetMutable<FAlterMeshParamBase>().BaseData.Id == ParamId)
					{
						Param.Get<FInstancedStruct>().GetMutable<StructType>().Data.Value = Value;
					}
				}				
			}
		}
	}

	FAlterMeshParamId ParamId;
	ValueType Value;
};

USTRUCT()
struct FAlterMeshParamSectionTemplate : public FMovieSceneEvalTemplate
{
	GENERATED_BODY()
public:
	
	FAlterMeshParamSectionTemplate() {};
	FAlterMeshParamSectionTemplate(const UMovieSceneSection& Section, const UMovieScenePropertyTrack& Track, FAlterMeshParamId InParamId)
	{
		ParamId = InParamId;
	}

	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); };
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;

	UPROPERTY()
	FAlterMeshParamId ParamId;
};

USTRUCT()
struct FAlterMeshFloatSectionTemplate : public FAlterMeshParamSectionTemplate
{
	GENERATED_BODY()
public:
	FAlterMeshFloatSectionTemplate() {};
	FAlterMeshFloatSectionTemplate(const UMovieSceneFloatSection& Section, const UMovieScenePropertyTrack& Track, FAlterMeshParamId InParamId)
	: FAlterMeshParamSectionTemplate(Section, Track, InParamId)
	, FloatCurve(Section.GetChannel())
	{
		
	}

	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); };
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override
	{
		Super::Evaluate(Operand, Context, PersistentData, ExecutionTokens);
		
		float Result = 0;
		if (FloatCurve.Evaluate(Context.GetTime(), Result))
		{
			ExecutionTokens.Add(TAlterMeshParamExecutionToken<FAlterMeshFloatParam, float>(Result, ParamId));
		}
	}
	
	UPROPERTY()
	FMovieSceneFloatChannel FloatCurve;
};

USTRUCT()
struct FAlterMeshBoolSectionTemplate : public FAlterMeshParamSectionTemplate
{
	GENERATED_BODY()
public:
	FAlterMeshBoolSectionTemplate() {};
	FAlterMeshBoolSectionTemplate(const UMovieSceneBoolSection& Section, const UMovieScenePropertyTrack& Track, FAlterMeshParamId InParamId)
	: FAlterMeshParamSectionTemplate(Section, Track, InParamId)
	, BoolCurve(Section.GetChannel())
	{
		
	}
	
	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); };
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override
	{
		Super::Evaluate(Operand, Context, PersistentData, ExecutionTokens);
		
		bool Result = false;
		if (BoolCurve.Evaluate(Context.GetTime(), Result))
		{
			ExecutionTokens.Add(TAlterMeshParamExecutionToken<FAlterMeshBoolParam, bool>(Result, ParamId));
		}
	}
	
	UPROPERTY()
	FMovieSceneBoolChannel BoolCurve;
};


USTRUCT()
struct FAlterMeshStringSectionTemplate : public FAlterMeshParamSectionTemplate
{
	GENERATED_BODY()
public:
	FAlterMeshStringSectionTemplate() {};
	FAlterMeshStringSectionTemplate(const UMovieSceneStringSection& Section, const UMovieScenePropertyTrack& Track, FAlterMeshParamId InParamId)
	: FAlterMeshParamSectionTemplate(Section, Track, InParamId)
	, StringCurve(Section.GetChannel())
	{
		
	}
	
	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); };
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override
	{
		Super::Evaluate(Operand, Context, PersistentData, ExecutionTokens);
		
		const FString* Result = StringCurve.Evaluate(Context.GetTime());
		if (Result)
		{
			ExecutionTokens.Add(TAlterMeshParamExecutionToken<FAlterMeshStringParam, FString>(*Result, ParamId));
		}
	}
	
	UPROPERTY()
	FMovieSceneStringChannel StringCurve;
};

USTRUCT()
struct FAlterMeshVectorSectionTemplate : public FAlterMeshParamSectionTemplate
{
	GENERATED_BODY()
public:
	FAlterMeshVectorSectionTemplate() {};
	FAlterMeshVectorSectionTemplate(const UMovieSceneDoubleVectorSection& Section, const UMovieScenePropertyTrack& Track, FAlterMeshParamId InParamId)
	: FAlterMeshParamSectionTemplate(Section, Track, InParamId)
	{
		VectorCurve[0] = Section.GetChannel(0);
		VectorCurve[1] = Section.GetChannel(1);
		VectorCurve[2] = Section.GetChannel(2);	
	};
	
	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); };
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override
	{
		Super::Evaluate(Operand, Context, PersistentData, ExecutionTokens);
		
		double X, Y, Z = 0.f;
		if (VectorCurve[0].Evaluate(Context.GetTime(), X)
			&& VectorCurve[1].Evaluate(Context.GetTime(), Y)
			&& VectorCurve[2].Evaluate(Context.GetTime(), Z))
		{
			ExecutionTokens.Add(TAlterMeshParamExecutionToken<FAlterMeshVectorParam, FVector>(FVector(X,Y,Z), ParamId));
		}
	}
	
	UPROPERTY()
	FMovieSceneDoubleChannel VectorCurve[3];
};

USTRUCT()
struct FAlterMeshRotationSectionTemplate : public FAlterMeshParamSectionTemplate
{
	GENERATED_BODY()
public:
	FAlterMeshRotationSectionTemplate() {};
	FAlterMeshRotationSectionTemplate(const UMovieSceneDoubleVectorSection& Section, const UMovieScenePropertyTrack& Track, FAlterMeshParamId InParamId)
	: FAlterMeshParamSectionTemplate(Section, Track, InParamId)
	{
		RotationCurve[0] = Section.GetChannel(0);
		RotationCurve[1] = Section.GetChannel(1);
		RotationCurve[2] = Section.GetChannel(2);	
	};
	
	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); };
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override
	{
		Super::Evaluate(Operand, Context, PersistentData, ExecutionTokens);
		
		double X, Y, Z = 0.f;
		if (RotationCurve[0].Evaluate(Context.GetTime(), X)
			&& RotationCurve[1].Evaluate(Context.GetTime(), Y)
			&& RotationCurve[2].Evaluate(Context.GetTime(), Z))
		{
			ExecutionTokens.Add(TAlterMeshParamExecutionToken<FAlterMeshRotationParam, FRotator>(FRotator(Y,Z,X), ParamId));
		}
	}
	
	UPROPERTY()
	FMovieSceneDoubleChannel RotationCurve[3];
};

USTRUCT()
struct FAlterMeshColorSectionTemplate : public FAlterMeshParamSectionTemplate
{
	GENERATED_BODY()
public:
	FAlterMeshColorSectionTemplate() {};
	FAlterMeshColorSectionTemplate(const UMovieSceneColorSection& Section, const UMovieScenePropertyTrack& Track, FAlterMeshParamId InParamId)
	: FAlterMeshParamSectionTemplate(Section, Track, InParamId)
	{
		ColorCurve[0] = Section.GetRedChannel();
		ColorCurve[1] = Section.GetGreenChannel();
		ColorCurve[2] = Section.GetBlueChannel();
		ColorCurve[3] = Section.GetAlphaChannel();		
	}
	
	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); };
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override
	{
		Super::Evaluate(Operand, Context, PersistentData, ExecutionTokens);
		
		float R,G,B,A = 0.f;
		if (ColorCurve[0].Evaluate(Context.GetTime(), R)
			&& ColorCurve[1].Evaluate(Context.GetTime(), G)
			&& ColorCurve[2].Evaluate(Context.GetTime(), B)
			&& ColorCurve[3].Evaluate(Context.GetTime(), A))
		{
			ExecutionTokens.Add(TAlterMeshParamExecutionToken<FAlterMeshColorParam, FLinearColor>(FLinearColor(R,G,B,A), ParamId));
		}
	}
	
	UPROPERTY()
	FMovieSceneFloatChannel ColorCurve[4];
};

USTRUCT()
struct FAlterMeshIntSectionTemplate : public FAlterMeshParamSectionTemplate
{
	GENERATED_BODY()
public:
	FAlterMeshIntSectionTemplate() {};
	FAlterMeshIntSectionTemplate(const UMovieSceneIntegerSection& Section, const UMovieScenePropertyTrack& Track, FAlterMeshParamId InParamId)
	: FAlterMeshParamSectionTemplate(Section, Track, InParamId)
	, IntCurve(Section.GetChannel())
	{
		
	}
	
	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); };
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override
	{
		Super::Evaluate(Operand, Context, PersistentData, ExecutionTokens);
		
		int32 Result = 0;
		if (IntCurve.Evaluate(Context.GetTime(), Result))
		{
			ExecutionTokens.Add(TAlterMeshParamExecutionToken<FAlterMeshIntParam, int32>(Result, ParamId));
		}
	}
	
	UPROPERTY()
	FMovieSceneIntegerChannel IntCurve;
};


UINTERFACE(MinimalAPI)
class UAlterMeshParamSequencerInterface : public UInterface
{
	GENERATED_BODY()
};

class IAlterMeshParamSequencerInterface 
{
	GENERATED_BODY()
	
public:
	virtual FName GetTrackId() const = 0;
	virtual void SetParamId(FAlterMeshParamId InParamId) = 0;
};

UCLASS(MinimalAPI)
class UAlterMeshBoolTrack : public UMovieSceneBoolTrack, public IAlterMeshParamSequencerInterface
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UAlterMeshBoolTrack()
	{
#if WITH_EDITORONLY_DATA
		TrackTint = FColor(0, 255, 255, 255);
#endif
	}

	virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override
	{
		return FAlterMeshBoolSectionTemplate((UMovieSceneBoolSection&)InSection, *this, ParamId);
	}

	virtual bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override
	{
		return SectionClass == UMovieSceneBoolSection::StaticClass();
	}

	virtual UMovieSceneSection* CreateNewSection() override
	{
		return NewObject<UMovieSceneBoolSection>(this, NAME_None, RF_Transactional);
	}

	virtual FName GetTrackName() const override
	{
		return GetTrackId();
	}

	virtual FName GetTrackId() const override
	{
		return FName(ParamId.NodeGroup.ToString() + ParamId.Input.ToString());
	}

	virtual void SetParamId(FAlterMeshParamId InParamId) override
	{
		ParamId = InParamId;
	}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7
	// UE 5.7+ requires these functions for the new ECS-based evaluation system
	// Since we use the legacy template system, provide empty implementations
	virtual void ImportEntityImpl(UMovieSceneEntitySystemLinker* EntityLinker, const FEntityImportParams& Params, FImportedEntity* OutImportedEntity) override
	{
		// Empty implementation - we use the legacy template evaluation system
	}

	virtual bool PopulateEvaluationFieldImpl(const TRange<FFrameNumber>& EffectiveRange, const FMovieSceneEvaluationFieldEntityMetaData& InMetaData, FMovieSceneEntityComponentFieldBuilder* OutFieldBuilder) override
	{
		// Return false - we use the legacy template evaluation system
		return false;
	}
#endif

	UPROPERTY()
	FAlterMeshParamId ParamId;
};

UCLASS(MinimalAPI)
class UAlterMeshFloatTrack : public UMovieSceneFloatTrack, public IMovieSceneTrackTemplateProducer, public IAlterMeshParamSequencerInterface
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UAlterMeshFloatTrack()
	{
#if WITH_EDITORONLY_DATA
		TrackTint = FColor(0, 255, 255, 255);
#endif
	}
	
	virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override
	{
		return FAlterMeshFloatSectionTemplate((UMovieSceneFloatSection&)InSection, *this, ParamId);
	}
	
	virtual bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override
	{
		return SectionClass == UMovieSceneFloatSection::StaticClass();
	}

	virtual UMovieSceneSection* CreateNewSection() override
	{
		return NewObject<UMovieSceneFloatSection>(this, NAME_None, RF_Transactional);
	}

	virtual FName GetTrackName() const override
	{
		return GetTrackId();
	}

	virtual FName GetTrackId() const override
	{
		return FName(ParamId.NodeGroup.ToString() + ParamId.Input.ToString());
	}

	virtual void SetParamId(FAlterMeshParamId InParamId) override
	{
		ParamId = InParamId;
	}

	UPROPERTY()
	FAlterMeshParamId ParamId;
};

UCLASS(MinimalAPI)
class UAlterMeshIntegerTrack : public UMovieSceneIntegerTrack, public IMovieSceneTrackTemplateProducer, public IAlterMeshParamSequencerInterface
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UAlterMeshIntegerTrack()
	{
#if WITH_EDITORONLY_DATA
		TrackTint = FColor(0, 255, 255, 255);
#endif
	}
	
	virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override
	{		
		return FAlterMeshIntSectionTemplate((UMovieSceneIntegerSection&)InSection, *this, ParamId);
	}
	
	virtual bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override
	{
		return SectionClass == UMovieSceneIntegerSection::StaticClass();
	}

	virtual UMovieSceneSection* CreateNewSection() override
	{
		return NewObject<UMovieSceneIntegerSection>(this, NAME_None, RF_Transactional);
	}
	
	virtual FName GetTrackName() const override
	{
		return GetTrackId();
	}

	virtual FName GetTrackId() const override
	{
		return FName(ParamId.NodeGroup.ToString() + ParamId.Input.ToString());
	}

	virtual void SetParamId(FAlterMeshParamId InParamId) override
	{
		ParamId = InParamId;
	}

	UPROPERTY()
	FAlterMeshParamId ParamId;
};

UCLASS(MinimalAPI)
class UAlterMeshVectorTrack : public UMovieSceneDoubleVectorTrack, public IMovieSceneTrackTemplateProducer, public IAlterMeshParamSequencerInterface
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UAlterMeshVectorTrack()
	{
#if WITH_EDITORONLY_DATA
		TrackTint = FColor(0, 255, 255, 255);
#endif
	}

	virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override
	{
		return FAlterMeshVectorSectionTemplate((UMovieSceneDoubleVectorSection&)InSection, *this, ParamId);
	}

	virtual bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override
	{
		return SectionClass == UMovieSceneDoubleVectorSection::StaticClass();
	}

	virtual UMovieSceneSection* CreateNewSection() override
	{
		SetNumChannelsUsed(3);
		UMovieSceneDoubleVectorSection* NewSection = NewObject<UMovieSceneDoubleVectorSection>(this, NAME_None, RF_Transactional);
		NewSection->SetChannelsUsed(3);
		return NewSection;
	}

	virtual FName GetTrackName() const override
	{
		return GetTrackId();
	}

	virtual FName GetTrackId() const override
	{
		return FName(ParamId.NodeGroup.ToString() + ParamId.Input.ToString());
	}

	virtual void SetParamId(FAlterMeshParamId InParamId) override
	{
		ParamId = InParamId;
	}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7
	virtual void InitializeFromProperty(const FProperty* InProperty, const UE::MovieScene::FPropertyDefinition* InDefinition) override
	{
		// Empty implementation - we use custom param system
	}
#endif

	UPROPERTY()
	FAlterMeshParamId ParamId;
};

UCLASS(MinimalAPI)
class UAlterMeshRotationTrack : public UMovieSceneDoubleVectorTrack, public IMovieSceneTrackTemplateProducer, public IAlterMeshParamSequencerInterface
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UAlterMeshRotationTrack()
	{
#if WITH_EDITORONLY_DATA
		TrackTint = FColor(0, 255, 255, 255);
#endif
	}

	virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override
	{
		return FAlterMeshRotationSectionTemplate((UMovieSceneDoubleVectorSection&)InSection, *this, ParamId);
	}

	virtual bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override
	{
		return SectionClass == UMovieSceneDoubleVectorSection::StaticClass();
	}

	virtual UMovieSceneSection* CreateNewSection() override
	{
		SetNumChannelsUsed(3);
		UMovieSceneDoubleVectorSection* NewSection = NewObject<UMovieSceneDoubleVectorSection>(this, NAME_None, RF_Transactional);
		NewSection->SetChannelsUsed(3);
		return NewSection;
	}

	virtual FName GetTrackName() const override
	{
		return GetTrackId();
	}

	virtual FName GetTrackId() const override
	{
		return FName(ParamId.NodeGroup.ToString() + ParamId.Input.ToString());
	}

	virtual void SetParamId(FAlterMeshParamId InParamId) override
	{
		ParamId = InParamId;
	}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7
	virtual void InitializeFromProperty(const FProperty* InProperty, const UE::MovieScene::FPropertyDefinition* InDefinition) override
	{
		// Empty implementation - we use custom param system
	}
#endif

	UPROPERTY()
	FAlterMeshParamId ParamId;
};

UCLASS(MinimalAPI)
class UAlterMeshColorTrack : public UMovieSceneColorTrack, public IMovieSceneTrackTemplateProducer, public IAlterMeshParamSequencerInterface
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UAlterMeshColorTrack()
	{
	}
	
	virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override
	{		
		return FAlterMeshColorSectionTemplate((UMovieSceneColorSection&)InSection, *this, ParamId);
	}
	
	virtual bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override
	{
		return SectionClass == UMovieSceneColorSection::StaticClass();
	}

	virtual UMovieSceneSection* CreateNewSection() override
	{
		return NewObject<UMovieSceneColorSection>(this, NAME_None, RF_Transactional);
	}
	
	virtual FName GetTrackName() const override
	{
		return GetTrackId();
	}

	virtual FName GetTrackId() const override
	{
		return FName(ParamId.NodeGroup.ToString() + ParamId.Input.ToString());
	}

	virtual void SetParamId(FAlterMeshParamId InParamId) override
	{
		ParamId = InParamId;
	}

	UPROPERTY()
	FAlterMeshParamId ParamId;
};

UCLASS(MinimalAPI)
class UAlterMeshStringTrack : public UMovieScenePropertyTrack, public IMovieSceneTrackTemplateProducer, public IAlterMeshParamSequencerInterface
{
	GENERATED_BODY()

public:

	UAlterMeshStringTrack()
	{
#if WITH_EDITORONLY_DATA
		TrackTint = FColor(0, 255, 255, 255);
#endif
	}
	
	virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override
	{		
		return FAlterMeshStringSectionTemplate((UMovieSceneStringSection&)InSection, *this, ParamId);
	}

	virtual bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override
	{
		return SectionClass == UMovieSceneStringSection::StaticClass();
	}

	virtual UMovieSceneSection* CreateNewSection() override
	{
		return NewObject<UMovieSceneStringSection>(this, NAME_None, RF_Transactional);
	}
	
	virtual FName GetTrackName() const override
	{
		return GetTrackId();
	}

	virtual FName GetTrackId() const override
	{
		return FName(ParamId.NodeGroup.ToString() + ParamId.Input.ToString());
	}

	virtual void SetParamId(FAlterMeshParamId InParamId) override
	{
		ParamId = InParamId;
	}

	UPROPERTY()
	FAlterMeshParamId ParamId;
};