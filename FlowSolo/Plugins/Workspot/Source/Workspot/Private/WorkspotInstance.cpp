// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkspotInstance.h"
#include "WorkspotTree.h"
#include "WorkspotIterator.h"
#include "WorkspotEntry.h"
#include "Workspot.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

bool UWorkspotInstance::Setup(AActor* InActor, UWorkspotTree* InTree, FName EntryPointTag)
{
	if (!InActor)
	{
		UE_LOG(LogWorkspot, Error, TEXT("WorkspotInstance::Setup - Invalid actor"));
		return false;
	}

	if (!InTree || !InTree->IsValid())
	{
		UE_LOG(LogWorkspot, Error, TEXT("WorkspotInstance::Setup - Invalid workspot tree"));
		return false;
	}

	// Check for skeletal mesh component (required for playing animations)
	USkeletalMeshComponent* MeshComp = InActor->FindComponentByClass<USkeletalMeshComponent>();
	if (!MeshComp)
	{
		UE_LOG(LogWorkspot, Error, TEXT("WorkspotInstance::Setup - No SkeletalMeshComponent on actor '%s'"), *InActor->GetName());
		return false;
	}

	// Store references
	Actor = InActor;
	WorkspotTree = InTree;
	State = EWorkspotState::Starting;
	CurrentIdleAnim = "stand";
	PreviousIdleAnim = "stand";
	CurrentPlayTime = 0.0f;

	// Create context (pure data context for Iterator traversal)
	FWorkspotContext Context;
	Context.User = InActor;
	Context.Tree = InTree;
	Context.CurrentIdle = CurrentIdleAnim;
	Context.PreviousIdle = PreviousIdleAnim;

	// Create iterator
	Iterator = InTree->RootEntry->CreateIterator(Context);
	if (!Iterator.IsValid())
	{
		UE_LOG(LogWorkspot, Error, TEXT("WorkspotInstance::Setup - Failed to create iterator"));
		State = EWorkspotState::Inactive;
		return false;
	}

	// Start first entry
	if (Iterator->Next(Context))
	{
		// Log initial Entry type
		UE_LOG(LogWorkspot, Log, TEXT("  ➡️  WorkspotInstance - Initial Entry: %s"),
			*Iterator->GetDebugString());

		if (Iterator->GetData(CurrentEntryData))
		{
			PlayCurrentAnimation();
			State = EWorkspotState::Playing;

			UE_LOG(LogWorkspot, Log, TEXT("WorkspotInstance::Setup - Started workspot '%s' on '%s'"),
				*InTree->GetName(), *InActor->GetName());

			return true;
		}
	}

	UE_LOG(LogWorkspot, Error, TEXT("WorkspotInstance::Setup - No entries to play"));
	State = EWorkspotState::Inactive;
	return false;
}

void UWorkspotInstance::Tick(float DeltaTime)
{
	if (State != EWorkspotState::Playing || !Iterator.IsValid())
	{
		return;
	}

	AActor* CurrentActor = Actor.Get();
	if (!CurrentActor)
	{
		Stop(true);
		return;
	}

	// Check if current animation finished
	if (IsCurrentAnimationFinished())
	{
		// Create context for next iteration
		FWorkspotContext Context;
		Context.User = CurrentActor;
		Context.Tree = WorkspotTree.Get();
		Context.CurrentIdle = CurrentIdleAnim;
		Context.PreviousIdle = PreviousIdleAnim;

		// Advance to next entry
		if (Iterator->Next(Context))
		{
			// Log current Entry type for debugging
			UE_LOG(LogWorkspot, Log, TEXT("  ➡️  WorkspotInstance - Executing Entry: %s"),
				*Iterator->GetDebugString());

			FWorkspotEntryData NewEntryData;
			if (Iterator->GetData(NewEntryData))
			{
				// Check for idle change
				if (NewEntryData.IdleAnim != CurrentIdleAnim)
				{
					HandleIdleChange(NewEntryData.IdleAnim);
				}

				CurrentEntryData = NewEntryData;
				PlayCurrentAnimation();
			}
			else
			{
				UE_LOG(LogWorkspot, Warning, TEXT("WorkspotInstance::Tick - Iterator returned true but no data"));
				Stop(false);
			}
		}
		else
		{
			// Iterator finished
			UE_LOG(LogWorkspot, Log, TEXT("WorkspotInstance::Tick - Workspot completed"));
			Stop(false);
			OnCompleted.Broadcast(this);
		}
	}

	CurrentPlayTime += DeltaTime;
}

void UWorkspotInstance::Stop(bool bForceStop)
{
	if (State == EWorkspotState::Inactive)
	{
		return;
	}

	UE_LOG(LogWorkspot, Log, TEXT("WorkspotInstance::Stop - Stopping workspot (Force: %s)"),
		bForceStop ? TEXT("Yes") : TEXT("No"));

	// Stop animation
	USkeletalMeshComponent* MeshComp = GetSkeletalMeshComponent();
	if (MeshComp && CurrentEntryData.AnimMontage)
	{
		UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
		if (AnimInst)
		{
			// Stop through AnimInstance
			AnimInst->Montage_Stop(bForceStop ? 0.1f : CurrentEntryData.BlendOutTime, CurrentEntryData.AnimMontage);
		}
		else
		{
			// Stop direct playback
			MeshComp->Stop();
		}
	}

	// Despawn props
	DespawnAllProps();

	// Reset state
	Iterator.Reset();
	CurrentEntryData = FWorkspotEntryData();
	State = EWorkspotState::Finished;

	// Broadcast completion
	OnCompleted.Broadcast(this);
}

void UWorkspotInstance::TriggerReaction(FName ReactionType)
{
	UE_LOG(LogWorkspot, Log, TEXT("WorkspotInstance::TriggerReaction - %s"), *ReactionType.ToString());

	// TODO: Implement reaction system
	// This would:
	// 1. Find ReactionSequence entry in tree
	// 2. Interrupt current animation
	// 3. Play reaction
	// 4. Resume previous animation
}

UAnimInstance* UWorkspotInstance::GetAnimInstance() const
{
	if (USkeletalMeshComponent* MeshComp = GetSkeletalMeshComponent())
	{
		return MeshComp->GetAnimInstance();
	}
	return nullptr;
}

USkeletalMeshComponent* UWorkspotInstance::GetSkeletalMeshComponent() const
{
	if (AActor* CurrentActor = Actor.Get())
	{
		return CurrentActor->FindComponentByClass<USkeletalMeshComponent>();
	}
	return nullptr;
}

void UWorkspotInstance::PlayCurrentAnimation()
{
	USkeletalMeshComponent* MeshComp = GetSkeletalMeshComponent();
	if (!MeshComp)
	{
		UE_LOG(LogWorkspot, Error, TEXT("WorkspotInstance::PlayCurrentAnimation - No SkeletalMeshComponent"));
		return;
	}

	if (!CurrentEntryData.AnimMontage)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("WorkspotInstance::PlayCurrentAnimation - No montage to play"));
		return;
	}

	// Try to play on AnimInstance first (if available)
	UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
	if (AnimInst)
	{
		// Play montage through AnimInstance
		float PlayLength = AnimInst->Montage_Play(
			CurrentEntryData.AnimMontage,
			1.0f, // Play rate
			EMontagePlayReturnType::MontageLength,
			0.0f, // Start position
			true  // Stop all montages
		);

		CurrentPlayTime = 0.0f;

		UE_LOG(LogWorkspot, Log, TEXT("WorkspotInstance::PlayCurrentAnimation - Playing '%s' via AnimInstance (Idle: %s, Length: %.2f)"),
			*CurrentEntryData.AnimMontage->GetName(),
			*CurrentEntryData.IdleAnim.ToString(),
			PlayLength);
	}
	else
	{
		// No AnimInstance, play animation directly on SkeletalMeshComponent
		MeshComp->PlayAnimation(CurrentEntryData.AnimMontage, false);

		CurrentPlayTime = 0.0f;

		UE_LOG(LogWorkspot, Log, TEXT("WorkspotInstance::PlayCurrentAnimation - Playing '%s' directly on SkeletalMeshComponent (Idle: %s)"),
			*CurrentEntryData.AnimMontage->GetName(),
			*CurrentEntryData.IdleAnim.ToString());
	}
}

bool UWorkspotInstance::IsCurrentAnimationFinished() const
{
	if (!CurrentEntryData.AnimMontage)
	{
		return true;
	}

	USkeletalMeshComponent* MeshComp = GetSkeletalMeshComponent();
	if (!MeshComp)
	{
		return true;
	}

	// Check if using AnimInstance
	UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
	if (AnimInst)
	{
		// Check if montage is still playing through AnimInstance
		return !AnimInst->Montage_IsPlaying(CurrentEntryData.AnimMontage);
	}
	else
	{
		return MeshComp->IsPlaying();
		// Direct playback - check if animation is still playing on component
	//UAnimationAsset* CurrentAnim = MeshComp->GetAnimationAsset();
	//if (CurrentAnim == CurrentEntryData.AnimMontage)
	//{
	//	// Animation is still playing
	//	return !MeshComp->IsPlaying();
	//}
	//else
	//{
	//	// Different animation is playing (or none)
	//	return true;
	//}
	}
}

void UWorkspotInstance::HandleIdleChange(FName NewIdle)
{
	if (NewIdle == CurrentIdleAnim)
	{
		return;
	}

	UE_LOG(LogWorkspot, Log, TEXT("WorkspotInstance::HandleIdleChange - %s -> %s"),
		*CurrentIdleAnim.ToString(),
		*NewIdle.ToString());

	FName OldIdle = CurrentIdleAnim;
	PreviousIdleAnim = CurrentIdleAnim;
	CurrentIdleAnim = NewIdle;

	// Broadcast idle changed
	OnIdleChanged.Broadcast(OldIdle, NewIdle);

	// Try to play transition animation
	if (!PlayTransitionAnimation(OldIdle, NewIdle))
	{
		// No transition animation found
		UE_LOG(LogWorkspot, Verbose, TEXT("WorkspotInstance::HandleIdleChange - No transition animation"));
	}

	// Handle props based on idle change
	if (WorkspotTree)
	{
		for (const FWorkspotGlobalProp& Prop : WorkspotTree->GlobalProps)
		{
			// Spawn prop if condition matches
			if (Prop.bSpawnOnIdleChange)
			{
				if (Prop.SpawnOnIdle.IsNone() || Prop.SpawnOnIdle == NewIdle)
				{
					SpawnProp(Prop);
				}
			}

			// Despawn prop if condition matches
			if (Prop.bDespawnOnIdleChange)
			{
				DespawnProp(Prop.PropId);
			}
		}
	}
}

bool UWorkspotInstance::PlayTransitionAnimation(FName FromIdle, FName ToIdle)
{
	if (!WorkspotTree)
	{
		return false;
	}

	UAnimMontage* TransitionMontage = WorkspotTree->FindTransitionAnim(FromIdle, ToIdle);
	if (!TransitionMontage)
	{
		return false;
	}

	UAnimInstance* AnimInst = GetAnimInstance();
	if (!AnimInst)
	{
		return false;
	}

	// Play transition montage
	AnimInst->Montage_Play(TransitionMontage, 1.0f);

	UE_LOG(LogWorkspot, Log, TEXT("WorkspotInstance::PlayTransitionAnimation - Playing '%s'"),
		*TransitionMontage->GetName());

	return true;
}

void UWorkspotInstance::SpawnProp(const FWorkspotGlobalProp& PropDef)
{
	if (!PropDef.PropClass)
	{
		return;
	}

	// Check if already spawned
	if (SpawnedProps.Contains(PropDef.PropId))
	{
		return;
	}

	AActor* CurrentActor = Actor.Get();
	USkeletalMeshComponent* MeshComp = GetSkeletalMeshComponent();
	if (!CurrentActor || !MeshComp)
	{
		return;
	}

	// Spawn prop actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = CurrentActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* PropActor = GetWorld()->SpawnActor<AActor>(
		PropDef.PropClass,
		FTransform::Identity,
		SpawnParams);

	if (PropActor)
	{
		// Attach to socket
		PropActor->AttachToComponent(
			MeshComp,
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			PropDef.AttachSocketName);

		SpawnedProps.Add(PropDef.PropId, PropActor);

		UE_LOG(LogWorkspot, Log, TEXT("WorkspotInstance::SpawnProp - Spawned '%s' on socket '%s'"),
			*PropDef.PropId.ToString(),
			*PropDef.AttachSocketName.ToString());
	}
}

void UWorkspotInstance::DespawnProp(FName PropId)
{
	TObjectPtr<AActor>* FoundProp = SpawnedProps.Find(PropId);
	if (FoundProp && *FoundProp)
	{
		(*FoundProp)->Destroy();
		SpawnedProps.Remove(PropId);

		UE_LOG(LogWorkspot, Log, TEXT("WorkspotInstance::DespawnProp - Despawned '%s'"),
			*PropId.ToString());
	}
}

void UWorkspotInstance::DespawnAllProps()
{
	for (auto& Pair : SpawnedProps)
	{
		if (Pair.Value)
		{
			Pair.Value->Destroy();
		}
	}
	SpawnedProps.Empty();

	UE_LOG(LogWorkspot, Verbose, TEXT("WorkspotInstance::DespawnAllProps - Cleared all props"));
}
