// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkspotSubsystem.h"
#include "WorkspotInstance.h"
#include "WorkspotTree.h"
#include "WorkspotDebugger.h"
#include "Workspot.h"

//////////////////////////////////////////////////////////////////////////
// Console Variables & Commands
//////////////////////////////////////////////////////////////////////////

namespace UE::Workspot
{
	// Debug display toggle
	static bool bShowDebug = false;
	static FAutoConsoleVariableRef CVarShowDebug(
		TEXT("workspot.ShowDebug"),
		bShowDebug,
		TEXT("Toggle workspot debug display.\n")
		TEXT("0: Disabled (default)\n")
		TEXT("1: Enabled - Shows active workspot instances on screen"),
		ECVF_Default);

	// Debug verbosity level
	static int32 DebugVerbosity = 1;
	static FAutoConsoleVariableRef CVarDebugVerbosity(
		TEXT("workspot.DebugVerbosity"),
		DebugVerbosity,
		TEXT("Set workspot debug log verbosity.\n")
		TEXT("0: Off\n")
		TEXT("1: Basic (default)\n")
		TEXT("2: Detailed\n")
		TEXT("3: VeryVerbose"),
		ECVF_Default);

	// Dump all active instances
	static FAutoConsoleCommandWithWorldArgsAndOutputDevice CmdDumpInstances(
		TEXT("workspot.Dump"),
		TEXT("Logs all active workspot instances to console."),
		FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* World, FOutputDevice& OutputDevice)
			{
				if (const UWorkspotSubsystem* Subsystem = World->GetSubsystem<UWorkspotSubsystem>())
				{
					const auto& Instances = Subsystem->GetActiveInstances();

					OutputDevice.Logf(ELogVerbosity::Log, TEXT(""));
					OutputDevice.Logf(ELogVerbosity::Log, TEXT("═══════════════════════════════════════════"));
					OutputDevice.Logf(ELogVerbosity::Log, TEXT("  WORKSPOT ACTIVE INSTANCES: %d"), Instances.Num());
					OutputDevice.Logf(ELogVerbosity::Log, TEXT("═══════════════════════════════════════════"));

					int32 Index = 0;
					for (const auto& Pair : Instances)
					{
						AActor* Actor = Pair.Key.Get();
						UWorkspotInstance* Instance = Pair.Value;

						if (Actor && Instance)
						{
							UWorkspotTree* Tree = Instance->GetWorkspotTree();
							OutputDevice.Logf(ELogVerbosity::Log, TEXT("[%d] Actor: %s"), Index++, *Actor->GetName());
							OutputDevice.Logf(ELogVerbosity::Log, TEXT("    Tree: %s"), Tree ? *Tree->GetName() : TEXT("NULL"));
							OutputDevice.Logf(ELogVerbosity::Log, TEXT("    State: %d | Idle: %s | PlayTime: %.2fs"),
								(int32)Instance->GetState(),
								*Instance->GetCurrentIdleAnim().ToString(),
								Instance->GetCurrentPlayTime());
							OutputDevice.Logf(ELogVerbosity::Log, TEXT(""));
						}
					}

					if (Instances.Num() == 0)
					{
						OutputDevice.Logf(ELogVerbosity::Log, TEXT("  (No active instances)"));
					}

					OutputDevice.Logf(ELogVerbosity::Log, TEXT("═══════════════════════════════════════════"));
					OutputDevice.Logf(ELogVerbosity::Log, TEXT(""));
				}
				else
				{
					OutputDevice.Log(ELogVerbosity::Error, TEXT("Unable to access WorkspotSubsystem"));
				}
			})
	);

	// Print tree structure
	static FAutoConsoleCommandWithWorld CmdPrintTree(
		TEXT("workspot.PrintTree"),
		TEXT("Print structure of a WorkspotTree asset. Usage: workspot.PrintTree <TreeName>"),
		FConsoleCommandWithWorldDelegate::CreateLambda([](const UWorld* InWorld)
			{
				UE_LOG(LogWorkspot, Log, TEXT("Usage: workspot.PrintTree requires tree asset path"));
				UE_LOG(LogWorkspot, Log, TEXT("Alternatively, use blueprint node 'Print Tree Structure'"));
			})
	);

	// Stop all workspots
	static FAutoConsoleCommandWithWorld CmdStopAll(
		TEXT("workspot.StopAll"),
		TEXT("Stop all active workspot instances."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](const UWorld* InWorld)
			{
				if (UWorkspotSubsystem* Subsystem = InWorld->GetSubsystem<UWorkspotSubsystem>())
				{
					int32 Count = Subsystem->GetActiveInstances().Num();
					Subsystem->StopAllWorkspots(true);
					UE_LOG(LogWorkspot, Log, TEXT("Stopped %d workspot instance(s)"), Count);
				}
			})
	);
}

//////////////////////////////////////////////////////////////////////////
// Subsystem Implementation
//////////////////////////////////////////////////////////////////////////

#include "GameFramework/Actor.h"

void UWorkspotSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogWorkspot, Log, TEXT("WorkspotSubsystem::Initialize"));
}

void UWorkspotSubsystem::Deinitialize()
{
	UE_LOG(LogWorkspot, Log, TEXT("WorkspotSubsystem::Deinitialize - Stopping %d active workspots"), ActiveInstances.Num());

	// Stop all active workspots
	StopAllWorkspots(true);

	Super::Deinitialize();
}

void UWorkspotSubsystem::Tick(float DeltaTime)
{
	// Tick all active instances
	for (auto& Pair : ActiveInstances)
	{
		if (UWorkspotInstance* Instance = Pair.Value)
		{
			Instance->Tick(DeltaTime);
		}
	}

	// Cleanup completed instances
	CleanupCompletedInstances();

	// Draw debug info if enabled via console variable
	if (UE::Workspot::bShowDebug)
	{
		UWorkspotDebugger::DrawDebugInfo(GetWorld());
	}
}

TStatId UWorkspotSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UWorkspotSubsystem, STATGROUP_Tickables);
}

UWorkspotInstance* UWorkspotSubsystem::StartWorkspot(AActor* Actor, UWorkspotTree* WorkspotTree, FName EntryPointTag)
{
	// Stop existing workspot on this actor if any
	StopWorkspot(Actor, true);

	// Create new instance
	UWorkspotInstance* Instance = NewObject<UWorkspotInstance>(this);

	// Setup instance
	if (!Instance->Setup(Actor, WorkspotTree, EntryPointTag))
	{
		UE_LOG(LogWorkspot, Error, TEXT("WorkspotSubsystem::StartWorkspot - Failed to setup instance"));
		return nullptr;
	}

	// Bind completion callback
	Instance->OnCompleted.AddUObject(this, &UWorkspotSubsystem::OnInstanceCompleted);

	// Store in active instances
	ActiveInstances.Add(Actor, Instance);

	UE_LOG(LogWorkspot, Log, TEXT("WorkspotSubsystem::StartWorkspot - Started workspot '%s' on '%s'"),
		*WorkspotTree->GetName(), *Actor->GetName());

	return Instance;
}

void UWorkspotSubsystem::StopWorkspot(AActor* Actor, bool bForceStop)
{
	if (!Actor)
	{
		return;
	}

	if (UWorkspotInstance* Instance = GetActiveWorkspot(Actor))
	{
		Instance->Stop(bForceStop);
		// Will be removed in CleanupCompletedInstances
		PendingRemoval.Add(Actor);
	}
}

UWorkspotInstance* UWorkspotSubsystem::GetActiveWorkspot(AActor* Actor) const
{
	if (!Actor)
	{
		return nullptr;
	}

	if (const TObjectPtr<UWorkspotInstance>* InstancePtr = ActiveInstances.Find(Actor))
	{
		return *InstancePtr;
	}

	return nullptr;
}

bool UWorkspotSubsystem::IsActorInWorkspot(AActor* Actor) const
{
	UWorkspotInstance* Instance = GetActiveWorkspot(Actor);
	return Instance && Instance->IsActive();
}

void UWorkspotSubsystem::StopAllWorkspots(bool bForceStop)
{
	TArray<TWeakObjectPtr<AActor>> ActorsToStop;
	for (const auto& Pair : ActiveInstances)
	{
		ActorsToStop.Add(Pair.Key);
	}

	for (TWeakObjectPtr<AActor> ActorPtr : ActorsToStop)
	{
		if (AActor* Actor = ActorPtr.Get())
		{
			StopWorkspot(Actor, bForceStop);
		}
	}

	// Force cleanup
	CleanupCompletedInstances();
}

void UWorkspotSubsystem::CleanupCompletedInstances()
{
	// Remove instances marked for removal
	for (TWeakObjectPtr<AActor> ActorPtr : PendingRemoval)
	{
		if (ActorPtr.IsValid())
		{
			ActiveInstances.Remove(ActorPtr);
		}
	}
	PendingRemoval.Empty();

	// Remove instances with invalid actors or finished state
	TArray<TWeakObjectPtr<AActor>> ToRemove;
	for (const auto& Pair : ActiveInstances)
	{
		if (!Pair.Key.IsValid() || !Pair.Value || Pair.Value->IsFinished())
		{
			ToRemove.Add(Pair.Key);
		}
	}

	for (TWeakObjectPtr<AActor> ActorPtr : ToRemove)
	{
		ActiveInstances.Remove(ActorPtr);
	}
}

void UWorkspotSubsystem::OnInstanceCompleted(UWorkspotInstance* Instance)
{
	if (!Instance)
	{
		return;
	}

	AActor* Actor = Instance->GetActor();
	if (Actor)
	{
		UE_LOG(LogWorkspot, Log, TEXT("WorkspotSubsystem::OnInstanceCompleted - Workspot completed on '%s'"),
			*Actor->GetName());

		PendingRemoval.AddUnique(Actor);
	}
}
