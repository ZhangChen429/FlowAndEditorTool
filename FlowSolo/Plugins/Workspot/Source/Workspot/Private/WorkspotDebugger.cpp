// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkspotDebugger.h"
#include "WorkspotInstance.h"
#include "WorkspotSubsystem.h"
#include "WorkspotTree.h"
#include "WorkspotEntry.h"
#include "WorkspotIterator.h"
#include "Workspot.h"
#include "Engine/World.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

bool UWorkspotDebugger::bDebugDisplayEnabled = false;
int32 UWorkspotDebugger::DebugVerbosity = 1;

void UWorkspotDebugger::EnableDebugDisplay(UObject* WorldContextObject, bool bEnable)
{
	bDebugDisplayEnabled = bEnable;

	UE_LOG(LogWorkspot, Log, TEXT("Workspot Debug Display: %s"), bEnable ? TEXT("ENABLED") : TEXT("DISABLED"));

	if (bEnable)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("═══════════════════════════════════════════"));
		UE_LOG(LogWorkspot, Warning, TEXT("🔍 WORKSPOT DEBUG MODE ENABLED"));
		UE_LOG(LogWorkspot, Warning, TEXT("   Watch screen for Entry traversal info"));
		UE_LOG(LogWorkspot, Warning, TEXT("   Check output log for detailed traces"));
		UE_LOG(LogWorkspot, Warning, TEXT("═══════════════════════════════════════════"));
	}
}

bool UWorkspotDebugger::IsDebugDisplayEnabled(UObject* WorldContextObject)
{
	return bDebugDisplayEnabled;
}

void UWorkspotDebugger::SetDebugVerbosity(UObject* WorldContextObject, int32 VerbosityLevel)
{
	DebugVerbosity = FMath::Clamp(VerbosityLevel, 0, 3);
	UE_LOG(LogWorkspot, Log, TEXT("Workspot Debug Verbosity set to: %d"), DebugVerbosity);
}

void UWorkspotDebugger::PrintTreeStructure(UWorkspotTree* WorkspotTree, bool bDetailed)
{
	if (!WorkspotTree)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("PrintTreeStructure: Null tree"));
		return;
	}

	UE_LOG(LogWorkspot, Warning, TEXT(""));
	UE_LOG(LogWorkspot, Warning, TEXT("╔═══════════════════════════════════════════╗"));
	UE_LOG(LogWorkspot, Warning, TEXT("║   WORKSPOT TREE: %s"), *WorkspotTree->GetName());
	UE_LOG(LogWorkspot, Warning, TEXT("╚═══════════════════════════════════════════╝"));

	if (!WorkspotTree->RootEntry)
	{
		UE_LOG(LogWorkspot, Warning, TEXT("  ⚠️  No Root Entry"));
		return;
	}

	// Recursively print entry structure
	PrintEntry(WorkspotTree->RootEntry, 0, bDetailed);

	UE_LOG(LogWorkspot, Warning, TEXT(""));
}

void UWorkspotDebugger::PrintEntry(const UWorkspotEntry* Entry, int32 Depth, bool bDetailed)
{
	if (!Entry)
	{
		return;
	}

	FString Indent;
	for (int32 i = 0; i < Depth; ++i)
	{
		Indent += TEXT("  ");
	}

	FString Icon;
	FString TypeName;
	FString ExtraInfo;

	// Determine entry type and icon
	if (const UWorkspotAnimClip* Clip = Cast<UWorkspotAnimClip>(Entry))
	{
		Icon = TEXT("🎬");
		TypeName = TEXT("AnimClip");
		if (Clip->AnimMontage)
		{
			ExtraInfo = FString::Printf(TEXT("[%s] Idle:%s"),
				*Clip->AnimMontage->GetName(),
				*Clip->IdleAnim.ToString());
		}
	}
	else if (const UWorkspotEntryAnim* Anim = Cast<UWorkspotEntryAnim>(Entry))
	{
		Icon = TEXT("🎭");
		TypeName = TEXT("EntryAnim");
		if (Anim->AnimMontage)
		{
			ExtraInfo = FString::Printf(TEXT("[%s] Idle:%s"),
				*Anim->AnimMontage->GetName(),
				*Anim->IdleAnim.ToString());
		}
	}
	else if (const UWorkspotSequence* Seq = Cast<UWorkspotSequence>(Entry))
	{
		Icon = TEXT("📦");
		TypeName = TEXT("Sequence");
		ExtraInfo = FString::Printf(TEXT("(%d children)"), Seq->Entries.Num());
	}
	else if (const UWorkspotRandomList* Rand = Cast<UWorkspotRandomList>(Entry))
	{
		Icon = TEXT("🎲");
		TypeName = TEXT("RandomList");
		ExtraInfo = FString::Printf(TEXT("(%d options)"), Rand->Entries.Num());
	}
	else if (const UWorkspotSelector* Sel = Cast<UWorkspotSelector>(Entry))
	{
		Icon = TEXT("🔀");
		TypeName = TEXT("Selector");
		ExtraInfo = FString::Printf(TEXT("(%d options)"), Sel->Entries.Num());
	}
	else
	{
		Icon = TEXT("❓");
		TypeName = Entry->GetClass()->GetName();
	}

	UE_LOG(LogWorkspot, Warning, TEXT("%s%s %s %s"),
		*Indent, *Icon, *TypeName, *ExtraInfo);

	// Print children recursively
	if (const UWorkspotSequence* Seq = Cast<UWorkspotSequence>(Entry))
	{
		for (int32 i = 0; i < Seq->Entries.Num(); ++i)
		{
			if (bDetailed)
			{
				UE_LOG(LogWorkspot, Warning, TEXT("%s  [%d]"), *Indent, i);
			}
			PrintEntry(Seq->Entries[i], Depth + 1, bDetailed);
		}
	}
	else if (const UWorkspotRandomList* Rand = Cast<UWorkspotRandomList>(Entry))
	{
		for (int32 i = 0; i < Rand->Entries.Num(); ++i)
		{
			if (bDetailed)
			{
				UE_LOG(LogWorkspot, Warning, TEXT("%s  [%d] Weight:%.1f"),
					*Indent, i, Rand->Weights[i]);
			}
			PrintEntry(Rand->Entries[i], Depth + 1, bDetailed);
		}
	}
	else if (const UWorkspotSelector* Sel = Cast<UWorkspotSelector>(Entry))
	{
		for (int32 i = 0; i < Sel->Entries.Num(); ++i)
		{
			if (bDetailed)
			{
				UE_LOG(LogWorkspot, Warning, TEXT("%s  [%d] Idle:%f"),
					*Indent, i, Sel->Weights[i]);
			}
			PrintEntry(Sel->Entries[i], Depth + 1, bDetailed);
		}
	}
	
}

FString UWorkspotDebugger::GetInstanceDebugInfo(UWorkspotInstance* Instance)
{
	if (!Instance)
	{
		return TEXT("NULL Instance");
	}

	FString Info;
	Info += FString::Printf(TEXT("State: %d\n"), (int32)Instance->GetState());
	Info += FString::Printf(TEXT("PlayTime: %.2fs\n"), Instance->GetCurrentPlayTime());
	Info += FString::Printf(TEXT("CurrentIdle: %s\n"), *Instance->GetCurrentIdleAnim().ToString());

	return Info;
}

void UWorkspotDebugger::DrawDebugInfo(UWorld* World)
{
	if (!World || !bDebugDisplayEnabled)
	{
		return;
	}

	UWorkspotSubsystem* Subsystem = World->GetSubsystem<UWorkspotSubsystem>();
	if (!Subsystem)
	{
		return;
	}

	// Get all active instances
	const TMap<TWeakObjectPtr<AActor>, TObjectPtr<UWorkspotInstance>>& InstanceMap = Subsystem->GetActiveInstances();

	if (InstanceMap.Num() == 0)
	{
		// Draw "no active workspots" message
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow,
				TEXT("🔍 Workspot Debug: No active instances"));
		}
		return;
	}

	// Draw info for each instance
	int32 YOffset = 100;
	for (const auto& Pair : InstanceMap)
	{
		AActor* Actor = Pair.Key.Get();
		UWorkspotInstance* Instance = Pair.Value;

		if (!Actor || !Instance)
		{
			continue;
		}

		// Screen debug info
		if (GEngine)
		{
			FString DebugText = FString::Printf(TEXT("🎭 Workspot [%s]"), *Actor->GetName());
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, DebugText, true, FVector2D(1.5f, 1.5f));

			FString StateText = FString::Printf(TEXT("   State: %s | Idle: %s | Time: %.1fs"),
				*GetStateName(Instance->GetState()),
				*Instance->GetCurrentIdleAnim().ToString(),
				Instance->GetCurrentPlayTime());
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, StateText);

			// Show current Entry type (Iterator)
			TSharedPtr<FWorkspotIterator> Iterator = Instance->GetIterator();
			if (Iterator.IsValid())
			{
				FString EntryText = FString::Printf(TEXT("   📋 Entry: %s"), *Iterator->GetDebugString());
				GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, EntryText);
			}

			// Show current animation
			const FWorkspotEntryData& Data = Instance->GetCurrentEntryData();
			if (Data.AnimMontage)
			{
				FString AnimText = FString::Printf(TEXT("   🎬 Playing: %s"), *Data.AnimMontage->GetName());
				GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, AnimText);
			}
		}

		// 3D debug draw
		FVector ActorLocation = Actor->GetActorLocation();
		DrawDebugSphere(World, ActorLocation + FVector(0, 0, 200), 30.0f, 12, FColor::Cyan, false, 0.0f, 0, 2.0f);
		DrawDebugString(World, ActorLocation + FVector(0, 0, 250), TEXT("WORKSPOT"), nullptr, FColor::White, 0.0f, true, 1.5f);

		YOffset += 80;
	}
}

FString UWorkspotDebugger::GetStateName(EWorkspotState State)
{
	switch (State)
	{
	case EWorkspotState::Inactive: return TEXT("Inactive");
	case EWorkspotState::Starting: return TEXT("Starting");
	case EWorkspotState::Playing: return TEXT("Playing");
	case EWorkspotState::Stopping: return TEXT("Stopping");
	case EWorkspotState::Finished: return TEXT("Finished");
	default: return TEXT("Unknown");
	}
}
