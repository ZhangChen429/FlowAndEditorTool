// Copyright 2023 Aechmea

#include "..\Public\AlterMeshTracks.h"

#include "AlterMeshActor.h"
#include "AlterMeshParam.h"
#include "Evaluation/MovieSceneEvalTemplate.h"
#include "Sections/MovieSceneBoolSection.h"

void TAlterMeshRefreshExecutionToken::Execute(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player)
{
	TArray<AAlterMeshActor*> Actors;
	for (FMovieSceneEvaluationOperand& Operand : Operands)
	{
		for (TWeakObjectPtr<> WeakObject : Player.FindBoundObjects(Operand))
		{
			if (AAlterMeshActor* Actor = Cast<AAlterMeshActor>(WeakObject))
			{
				Actors.AddUnique(Actor);
			}
		}
	}

	for (auto* Actor : Actors)
	{
		Actor->RefreshSync();
	}

	Operands.Empty();
}

void FAlterMeshParamSectionTemplate::Evaluate(const FMovieSceneEvaluationOperand& Operand,
                                                 const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData,
                                                 FMovieSceneExecutionTokens& ExecutionTokens) const
{
	IMovieSceneSharedExecutionToken* Token = ExecutionTokens.FindShared(FAlterMeshExecutionTokenStaticId);
	TAlterMeshRefreshExecutionToken* RefreshToken = static_cast<TAlterMeshRefreshExecutionToken*>(Token);
	if (RefreshToken && !Context.IsPostRoll() && !Context.IsPreRoll())
	{
		RefreshToken->Operands.Add(Operand);
	}
	else
	{
		ExecutionTokens.AddShared(FAlterMeshExecutionTokenStaticId, TAlterMeshRefreshExecutionToken(Operand));				
	}
}
