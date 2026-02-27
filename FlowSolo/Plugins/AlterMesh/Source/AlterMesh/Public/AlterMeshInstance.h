// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshAsset.h"
#include "AlterMeshExport.h"
#include "AlterMeshHandle.h"
#include "AlterMeshImport.h"
#include "AlterMeshParam.h"
#include "AlterMeshSection.h"
#include "Tickable.h"
#include "Misc/MonitoredProcess.h"
#include "Async/Future.h"
#include "Templates/SharedPointer.h"

#if WITH_EDITOR
#include "DirectoryWatcherModule.h"
#include "IDirectoryWatcher.h"
#endif

#include "AlterMeshInstance.generated.h"

class UAlterMeshAsset;
class AAlterMeshActor;

static TAutoConsoleVariable<int32> CVarAlterMeshDebugInteractive(
	TEXT("AlterMesh.DebugInteractive"),
	0,
	TEXT("Opens blender for interactive debugging"));

UENUM()
enum class EAlterMeshInstanceState : uint8
{
	Closed,
	Working,
	Idle,
};

DECLARE_DELEGATE_OneParam(FImportMeshCallback, TArray<TSharedPtr<FAlterMeshPrimitive>>);
DECLARE_MULTICAST_DELEGATE(FOnRefreshDelegate);

using TMeshPromise = TPromise<TArray<TSharedPtr<FAlterMeshPrimitive>>>;

struct FAlterMeshRefreshPreExportTask
{
	FAlterMeshRefreshPreExportTask(FAlterMeshExport Exporter)
		: Exporter(Exporter)
	{

	}

	FORCEINLINE TStatId GetStatId() const { return TStatId(); }
	ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
	{
		Exporter.PreExport();
	}

	FAlterMeshExport Exporter;
};

struct FAlterMeshRefreshExportTask
{
	FAlterMeshRefreshExportTask(FAlterMeshExport Exporter)
		: Exporter(Exporter)
	{

	}

	FORCEINLINE TStatId GetStatId() const { return TStatId(); }
	ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
	{
		Exporter.Export();
	}

	FAlterMeshExport Exporter;
};

struct FAlterMeshRefreshImportTask
{
	FAlterMeshRefreshImportTask(FAlterMeshImport Importer, TSharedPtr<TMeshPromise> Promise)
		: Promise(Promise), Importer(Importer)
	{

	}

	FORCEINLINE TStatId GetStatId() const { return TStatId(); }
	ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
	{
		TArray<TSharedPtr<FAlterMeshPrimitive>> OutMeshes;
		Importer.ImportMeshes(OutMeshes);
		Promise->SetValue(OutMeshes);
	}
	
	const TSharedPtr<TMeshPromise> Promise;
	FAlterMeshImport Importer;
};

struct FAlterMeshRefreshCallbackTask
{	FAlterMeshRefreshCallbackTask(FImportMeshCallback Callback, FOnRefreshDelegate OnRefreshDelegate, uint8& NumActions, const TSharedPtr<TMeshPromise> Promise)
		: OnRefreshDelegate(OnRefreshDelegate), Callback(Callback), Promise(Promise), ActionCounterRef(NumActions)
	{
		
	}

	FORCEINLINE TStatId GetStatId() const { return TStatId(); }
	ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
	{
		Callback.ExecuteIfBound(Promise->GetFuture().Get());
		OnRefreshDelegate.Broadcast();
		ActionCounterRef--;
	}
	
	FOnRefreshDelegate OnRefreshDelegate;
	FImportMeshCallback Callback;
	
	const TSharedPtr<TMeshPromise> Promise;
	
	uint8& ActionCounterRef;
};

// A instance of blender
UCLASS(BlueprintType, Blueprintable)
class ALTERMESH_API UAlterMeshInstance : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UAlterMeshInstance();
	virtual ~UAlterMeshInstance() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; };
	virtual bool IsTickableWhenPaused() const override { return false; }
	virtual bool IsTickableInEditor() const override { return true; }

	// We will allow at most 2 refresh actions stacked, just yeet the rest
	// for animations use a Sync action
	uint8 NumRefreshActions;	
	FGraphEventRef PreviousTask;

	void RefreshSync(const FAlterMeshInputParams& InputParams, UAlterMeshAssetInterface* Asset, TArray<TSharedPtr<FAlterMeshPrimitive>>& OutMeshes);
	void RefreshAsync(const FAlterMeshInputParams& InputParams, UAlterMeshAssetInterface* Asset, FImportMeshCallback Callback);

	void CleanupProcess();
	bool IsValid();
	void OnProcOutput(FString Output) const;

	TSharedPtr<FMonitoredProcess> BlenderProcess;
	
	TSharedPtr<FAlterMeshHandle, ESPMode::ThreadSafe> AlterMeshHandle;

	float IdleTime;

	FOnRefreshDelegate OnRefreshDelegate;

	bool Initialize(FString FilePath, FString ScriptPath);

	EAlterMeshInstanceState State;
private:
	
	FDelegateHandle DirectoryWatcherHandle;
	FString CachedFileName;

#if	WITH_EDITOR
	void OnAssetChanged(const TArray<FFileChangeData>& FileChangeDatas);
#endif
	
	FCriticalSection CriticalSection;
	
	void EnqueueRefreshTasks(const FAlterMeshInputParams& InputParams, UAlterMeshAssetInterface* Asset, FImportMeshCallback Callback, bool bAsync);
};
