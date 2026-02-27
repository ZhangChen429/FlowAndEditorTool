// Copyright 2023 Aechmea

#include "AlterMeshInstance.h"

#include "AlterMesh/AlterMesh.h"
#include "AlterMeshAsset.h"
#include "AlterMeshComponent.h"
#include "AlterMeshExport.h"
#include "AlterMeshImport.h"
#include "AlterMeshSettings.h"
#include "StructView.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "AlterMeshHandle.h"

#include <Extern/AlterMesh.h>
#pragma comment(lib, "AlterMesh.lib")

#if WITH_EDITOR
#include "DirectoryWatcherModule.h"
#include "IDirectoryWatcher.h"
#endif

UAlterMeshInstance::UAlterMeshInstance()
{
}

UAlterMeshInstance::~UAlterMeshInstance()
{
	CleanupProcess();
}

void UAlterMeshInstance::Tick(float DeltaTime)
{
	IdleTime += DeltaTime;
	const float Timeout = GetDefault<UAlterMeshSettings>()->MaxIdleTime;

	if (IdleTime > Timeout)
	{
		CleanupProcess();
	}

	if (State != EAlterMeshInstanceState::Closed)
	{
		State = NumRefreshActions > 0 ? EAlterMeshInstanceState::Working : EAlterMeshInstanceState::Idle;
	}
}

TStatId UAlterMeshInstance::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAlterMeshInstance, STATGROUP_Tickables);
}

void UAlterMeshInstance::RefreshSync(const FAlterMeshInputParams& InputParams, UAlterMeshAssetInterface* Asset, TArray<TSharedPtr<FAlterMeshPrimitive>>& OutMeshes)
{
	EnqueueRefreshTasks(InputParams, Asset,
	                    FImportMeshCallback::CreateLambda(
		                    [&OutMeshes](const TArray<TSharedPtr<FAlterMeshPrimitive>>& Meshes)
		                    {
			                    OutMeshes = Meshes;
		                    }), false);
}

void UAlterMeshInstance::RefreshAsync(const FAlterMeshInputParams& InputParams, UAlterMeshAssetInterface* Asset, FImportMeshCallback Callback)
{
	EnqueueRefreshTasks(InputParams, Asset, Callback, true);
}

#if	WITH_EDITOR
void UAlterMeshInstance::OnAssetChanged(const TArray<FFileChangeData>& FileChangeDatas)
{
	for (const FFileChangeData& FileChangeData : FileChangeDatas)
	{
		if (FileChangeData.Filename == CachedFileName)
		{
			CleanupProcess();
		}
	}
}
#endif

bool UAlterMeshInstance::Initialize(FString FilePath, FString ScriptPath)
{
	auto Quote = [](FString& In)
	{
		return TEXT("\"") + In + TEXT("\"");
	};

	// Cleanup last proc
	CleanupProcess();

	const UAlterMeshSettings* Settings = GetDefault<UAlterMeshSettings>();
	if (Settings->ExecutablePath.FilePath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Executable path was not set. Project settings -> AlterMesh -> Executable Path"));
		return false;
	}

	FString Guid = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensInBraces);
	FString Guid2 = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensInBraces);
	AlterMeshHandle = MakeShared<FAlterMeshHandle>();
	AlterMeshHandle->Set(Init(*Guid, *Guid2));

	const FString URL = FPaths::ConvertRelativePathToFull(Settings->ExecutablePath.FilePath);
	if (!FPaths::FileExists(FilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not open file: %s"), *FilePath);
		return false;
	}

#if WITH_EDITOR
	CachedFileName = FilePath;
	FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>("DirectoryWatcher");
	if (IDirectoryWatcher* DirectoryWatcher = DirectoryWatcherModule.Get())
	{
		// Watches for file change and restart instance
		DirectoryWatcher->RegisterDirectoryChangedCallback_Handle(FPaths::GetPath(FilePath),
		                                                          IDirectoryWatcher::FDirectoryChanged::CreateUObject(this, &UAlterMeshInstance::OnAssetChanged),
		                                                          DirectoryWatcherHandle);
	}
#endif

	const bool bInteractive = CVarAlterMeshDebugInteractive.GetValueOnGameThread() == 1;

	const FString ProcParams = FString("--factory-startup ")
		+ (bInteractive ? TEXT(" ") : TEXT(" -b "))
		+ Quote(FilePath)
		+ TEXT(" -P ") + Quote(ScriptPath)
		+ TEXT(" -- ") + Guid2
		+ TEXT(" ") + Guid;

	BlenderProcess = MakeShareable(new FMonitoredProcess(URL, ProcParams, true));
	BlenderProcess->OnOutput().BindUObject(this, &UAlterMeshInstance::OnProcOutput);
	BlenderProcess->Launch();
	BlenderProcess->SetSleepInterval(0.01f);

	State = EAlterMeshInstanceState::Idle;
	return true;
}

void UAlterMeshInstance::CleanupProcess()
{
	if (BlenderProcess.IsValid())
	{
		BlenderProcess->Cancel(true);
		BlenderProcess.Reset();
	}

	State = EAlterMeshInstanceState::Closed;
}

bool UAlterMeshInstance::IsValid()
{
	return BlenderProcess.IsValid() && BlenderProcess->Update();
}

void UAlterMeshInstance::OnProcOutput(FString Output) const
{
	if (CVarAlterMeshDebugProcessOutput.GetValueOnAnyThread())
	{
		if (!Output.IsEmpty())
		{
			UE_LOG(LogAlterMeshImport, Log, TEXT("%s"), *Output);
		}
	}
}

void UAlterMeshInstance::EnqueueRefreshTasks(const FAlterMeshInputParams& InputParams, UAlterMeshAssetInterface* Asset, FImportMeshCallback Callback, bool bAsync)
{
	if (!Asset || HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient))
	{
		return;
	}

	FAlterMeshExport Exporter(AlterMeshHandle, InputParams, Asset, GetTypedOuter<AActor>());
	FAlterMeshImport Importer(AlterMeshHandle, Asset);
	
	if (!bAsync)
	{
		Exporter.PreExport();
		Exporter.Export();
		TArray<TSharedPtr<FAlterMeshPrimitive>> OutMeshes;
		Importer.ImportMeshes(OutMeshes);	
		Callback.ExecuteIfBound(OutMeshes);		
		return;
	}
	
	// Current and pending refresh
	if (NumRefreshActions >= 2)
	{
		return;
	}
	
	IdleTime = 0.f;
	
	NumRefreshActions++;
	State = EAlterMeshInstanceState::Working;	

	// Export params	
	FGraphEventArray PreExportPrerequisites;
	if (PreviousTask.IsValid())
	{
		PreExportPrerequisites.Add(PreviousTask);
	}
	
	TSharedPtr<TMeshPromise> Promise = MakeShared<TMeshPromise>();
	
	FGraphEventRef PreExportTask = TGraphTask<FAlterMeshRefreshPreExportTask>::CreateTask(&PreExportPrerequisites, ENamedThreads::GameThread).ConstructAndDispatchWhenReady(Exporter);

	if (!FTaskGraphInterface::Get().IsThreadProcessingTasks(ENamedThreads::GameThread))
	{
		FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
	}	
	
	FGraphEventArray ExportPrerequisites({PreExportTask});
	FGraphEventRef ExportTask = TGraphTask<FAlterMeshRefreshExportTask>::CreateTask(&ExportPrerequisites, ENamedThreads::GameThread).ConstructAndDispatchWhenReady(Exporter);

	// Import meshes
	FGraphEventArray ImportPrerequisites({ ExportTask });
	FGraphEventRef ImportTask = TGraphTask<FAlterMeshRefreshImportTask>::CreateTask(&ImportPrerequisites,  ENamedThreads::AnyThread).ConstructAndDispatchWhenReady(Importer, Promise);

	// Build components		
	FGraphEventArray CallbackPrerequisites({ ImportTask });
	FGraphEventRef CallbackTask = TGraphTask<FAlterMeshRefreshCallbackTask>::CreateTask(&CallbackPrerequisites, ENamedThreads::GameThread).ConstructAndDispatchWhenReady(Callback, OnRefreshDelegate, NumRefreshActions, Promise);

	PreviousTask = CallbackTask;
}
