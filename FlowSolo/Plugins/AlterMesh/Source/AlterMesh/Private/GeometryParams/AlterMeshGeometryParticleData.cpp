// Copyright 2023 Aechmea

#include "GeometryParams/AlterMeshGeometryParticleData.h"

#include "AlterMeshActor.h"
#include "AlterMeshExport.h"
#include "AlterMeshSettings.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceExport.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraGpuComputeDispatchInterface.h"
#include "NiagaraSystemInstanceController.h"

#if WITH_EDITOR
#include "DragAndDrop/AssetDragDropOp.h"
#include "NiagaraGraph.h"
#include "ViewModels/NiagaraEmitterViewModel.h"
#include "ViewModels/NiagaraSystemViewModel.h"
#include "ViewModels/Stack/NiagaraStackItemGroup.h"
#include "ViewModels/Stack/NiagaraStackPropertyRow.h"
#include "ViewModels/Stack/NiagaraStackViewModel.h"
#include "ViewModels/Stack/NiagaraStackFunctionInput.h"
#include "NiagaraNodeFunctionCall.h"
#include "IDetailTreeNode.h"
#include "NiagaraScriptSource.h"
#include "NiagaraScriptSourceBase.h"
#endif

#if WITH_EDITOR
void UAlterMeshGeometryParticleData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property
		&& PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UAlterMeshGeometryParticleData, ParticleSystem))
	{
		if (IsValid(NiagaraComponent))
		{
			NiagaraComponent->DestroyComponent();
		}

		if (ParticleSystem)
		{
			if (AActor* OwnerActor = GetTypedOuter<AActor>())
			{				
				if (NiagaraComponent && OwnerActor->GetInstanceComponents().Contains(NiagaraComponent))
				{
					GetTypedOuter<AActor>()->RemoveInstanceComponent(NiagaraComponent);
				}
				
				NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(ParticleSystem, OwnerActor->GetRootComponent(),
				NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTargetIncludingScale, false, false);
				NiagaraComponent->SetVisibility(false);
				OwnerActor->AddInstanceComponent(NiagaraComponent);
			}
		}
	}
}

#endif

void UAlterMeshGeometryParticleData::ReceiveParticleData_Implementation(const TArray<FBasicParticleData>& Data, UNiagaraSystem* NiagaraSystem,
	const FVector& SimulationPositionOffset)
{
	INiagaraParticleCallbackHandler::ReceiveParticleData_Implementation(Data, NiagaraSystem, SimulationPositionOffset);

	ParticleLocations.Empty(Data.Num());
	for (const FBasicParticleData& Particle : Data)
	{
		if (!Particle.Position.IsNearlyZero(0.01f))
		{
			ParticleLocations.Add(FVector3f(Particle.Position));
		}
	}
}

void UAlterMeshGeometryParticleData::Initialize(AActor* OwnerActor)
{
	Super::Initialize(OwnerActor);

	const UAlterMeshSettings* Settings = GetDefault<UAlterMeshSettings>();

	if (ParticleSystem && !NiagaraComponent)
	{
		NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(ParticleSystem, OwnerActor->GetRootComponent(),
			NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTargetIncludingScale, false, false);
		
		NiagaraComponent->SetVisibility(false);
		// Add to component list so user can change niagara params
		OwnerActor->AddInstanceComponent(NiagaraComponent);
	}
}

void UAlterMeshGeometryParticleData::Cleanup()
{
	if (NiagaraComponent)
	{
		if (AActor* Actor = GetTypedOuter<AActor>())
		{
			if (Actor->GetInstanceComponents().Contains(NiagaraComponent))
			{
				GetTypedOuter<AActor>()->RemoveInstanceComponent(NiagaraComponent);
			}
		}

		NiagaraComponent->DestroyComponent();
		NiagaraComponent = nullptr;
	}

	Super::Cleanup();
}

void UAlterMeshGeometryParticleData::PreExport(FAlterMeshExport& Exporter)
{
	Super::PreExport(Exporter);
	
	NiagaraComponent->DestroyInstance();
	NiagaraComponent->SetForceSolo(true);
	NiagaraComponent->SetVisibility(true);

	SetupAsset();
	
	NiagaraComponent->SetVariableObject(CallbackObjectParameterName, this);
	NiagaraComponent->ResetSystem();

	if (NiagaraComponent->GetSystemInstanceController())
	{
		NiagaraComponent->GetSystemInstanceController()->AdvanceSimulation(WarmupTime/TickDelta, TickDelta);
	}
	
	if (UWorld* World = GetWorld())
	{
		if (auto* DispatchInterface = FNiagaraGpuComputeDispatchInterface::Get(World))
		{
			DispatchInterface->FlushAndWait_GameThread();
		}
	}
}

void UAlterMeshGeometryParticleData::Export(FAlterMeshExport& Exporter)
{
	Super::Export(Exporter);

	for (FVector3f& Pos : ParticleLocations)
	{
		Pos -= FVector3f(Exporter.Location);
		Pos = Exporter.ToBlenderMatrix.TransformPosition(Pos);
	}

	Exporter.WriteArray(ParticleLocations);
	
	NiagaraComponent->Deactivate();
	NiagaraComponent->SetVisibility(false);
}

bool UAlterMeshGeometryParticleData::ShouldExport()
{
	return IsValid(NiagaraComponent) && IsValid(ParticleSystem);
}

#if WITH_EDITOR
bool UAlterMeshGeometryParticleData::CanDropOperation(TSharedPtr<FDragDropOperation> DragDropOperation, int32 OperationIndex)
{
	if (DragDropOperation->IsOfType<FAssetDragDropOp>())
	{
		TSharedPtr<FAssetDragDropOp> AssetDrop = StaticCastSharedPtr<FAssetDragDropOp>(DragDropOperation);
		if (AssetDrop->GetAssets().IsValidIndex(OperationIndex))
		{
			FAssetData AssetData = AssetDrop->GetAssets()[OperationIndex];
			if ( AssetData.GetAsset()->IsA(UNiagaraSystem::StaticClass()))
			{
				return true;
			}
		}
	}

	return false;
}

void UAlterMeshGeometryParticleData::OnDropOperation(TSharedPtr<FDragDropOperation> DragDropOperation, int32 OperationIndex)
{
	Super::OnDropOperation(DragDropOperation, OperationIndex);

	if (DragDropOperation->IsOfType<FAssetDragDropOp>())
	{
		TSharedPtr<FAssetDragDropOp> AssetDrop = StaticCastSharedPtr<FAssetDragDropOp>(DragDropOperation);

		FAssetData AssetData = AssetDrop->GetAssets()[OperationIndex];
		if ( AssetData.GetAsset()->IsA(UNiagaraSystem::StaticClass()))
		{
			if (IsValid(NiagaraComponent))
			{
				NiagaraComponent->DestroyComponent();
			}

			ParticleSystem = Cast<UNiagaraSystem>(AssetData.GetAsset());
			if (ParticleSystem)
			{
				const AActor* OwnerActor = GetTypedOuter<AActor>();
				NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(ParticleSystem, OwnerActor->GetRootComponent(),
				NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTargetIncludingScale, false, false);
			}
		}
	}
}
#endif

void UAlterMeshGeometryParticleData::SetupAsset()
{
#if WITH_EDITOR
	UNiagaraDataInterfaceExport* ExportDataInterface = UNiagaraFunctionLibrary::GetDataInterface<UNiagaraDataInterfaceExport>(NiagaraComponent, DataInterfaceParameterName);
	
	if (!ExportDataInterface)
	{
		UE_LOG(LogAlterMeshExport, Warning, TEXT("Particle Data Exporter: Could not find export data interface... Modifying Niagara Asset"))
	
		FNiagaraVariable OutputParam(FNiagaraTypeDefinition(UNiagaraDataInterfaceExport::StaticClass()), DataInterfaceParameterName);
		NiagaraComponent->GetAsset()->GetExposedParameters().AddParameter(OutputParam, true);
		ExportDataInterface = UNiagaraFunctionLibrary::GetDataInterface<UNiagaraDataInterfaceExport>(NiagaraComponent, DataInterfaceParameterName);
		NiagaraComponent->GetAsset()->RequestCompile(false);
	}
	
	FNiagaraVariable CallbackParameter(FNiagaraTypeDefinition::GetUObjectDef(), CallbackObjectParameterName);	
	NiagaraComponent->GetAsset()->GetExposedParameters().AddParameter(CallbackParameter, true);	
	FNiagaraUserParameterBinding Binding;
	Binding.Parameter = CallbackParameter;
	ExportDataInterface->CallbackHandlerParameter = Binding;
	
	const UAlterMeshEditorSettings* Settings = GetDefault<UAlterMeshEditorSettings>(UAlterMeshEditorSettings::StaticClass());
	bool bHasExportToBlueprintScript = false;
	
	for (int32 i = 0; i < NiagaraComponent->GetAsset()->GetNumEmitters(); i++)
	{
		const FVersionedNiagaraEmitter& Emitter = NiagaraComponent->GetAsset()->GetEmitterHandle(i).GetInstance();	
		TArray<UNiagaraScript*> Scripts;
		Emitter.Emitter->GetEmitterData(Emitter.Version)->GetScripts(Scripts);
		for (UNiagaraScript* Script : Scripts)
		{
			if (UNiagaraGraph* Graph = Cast<UNiagaraGraph>(Cast<UNiagaraScriptSource>(Script->GetLatestSource())->NodeGraph))
			{
				for (UEdGraphNode* Node : Graph->Nodes)
				{
					UNiagaraNodeFunctionCall* FunctionCallNode = Cast<UNiagaraNodeFunctionCall>(Node);
					if (FunctionCallNode && FunctionCallNode->FunctionScript)
					{
						if (FunctionCallNode->FunctionScript.GetFName() == Settings->ExportToBlueprintScript.LoadSynchronous()->GetFName())
						{
							bHasExportToBlueprintScript = true;
						}
					}
				}
			}
		}
	}
	
	if (!bHasExportToBlueprintScript)
	{
		UE_LOG(LogAlterMeshExport, Warning, TEXT("Particle Data Exporter: Could not find Export to Blueprint Script in any emitters... Modifying Niagara Asset"))
			
		for (int32 i = 0; i < NiagaraComponent->GetAsset()->GetNumEmitters(); i++)
		{
			const FVersionedNiagaraEmitter& Emitter = NiagaraComponent->GetAsset()->GetEmitterHandle(i).GetInstance();
			UNiagaraScript* UpdateScript = Emitter.Emitter->GetEmitterData(Emitter.Version)->GetScript(ENiagaraScriptUsage::ParticleUpdateScript, FGuid());
	
			bool bFoundModule = false;
			UpdateScript->GetLatestSource()->AddModuleIfMissing(Settings->ExportToBlueprintScript.ToString(), ENiagaraScriptUsage::ParticleUpdateScript, bFoundModule);
		}
	
		for (int32 i = 0; i < NiagaraComponent->GetAsset()->GetNumEmitters(); i++)
		{
			const FVersionedNiagaraEmitter& Emitter = NiagaraComponent->GetAsset()->GetEmitterHandle(i).GetInstance();	
			TArray<UNiagaraScript*> Scripts;
			Emitter.Emitter->GetEmitterData(Emitter.Version)->GetScripts(Scripts);
			for (UNiagaraScript* Script : Scripts)
			{
				FNiagaraVariable ConditionParameter(FNiagaraTypeDefinition::GetBoolDef(), FName("Condition To Export Data"));
				FNiagaraVariable ExportParameter(FNiagaraTypeDefinition(UNiagaraDataInterfaceExport::StaticClass()), FName("Export Particle Data Interface"));
				FNiagaraVariable DeterminismParameter(FNiagaraTypeDefinition::GetBoolDef(), FName("bDeterminism"));
				
				FNiagaraSystemViewModelOptions SystemViewModelOptions;
				SystemViewModelOptions.bCanAutoCompile = false;
				SystemViewModelOptions.bCanSimulate = false;
				SystemViewModelOptions.MessageLogGuid = NiagaraComponent->GetAsset()->GetAssetGuid();
				
				TSharedRef<FNiagaraSystemViewModel> SystemViewModel = MakeShared<FNiagaraSystemViewModel>();
				SystemViewModel->Initialize(*NiagaraComponent->GetAsset(), SystemViewModelOptions);
				for (auto& EmitterHandleViewModel : SystemViewModel->GetEmitterHandleViewModels())
				{
					UNiagaraStackViewModel* EmitterStackViewModel = EmitterHandleViewModel->GetEmitterStackViewModel();

					TArray<UNiagaraStackEntry*> EntriesToCheck;
					EmitterStackViewModel->GetRootEntry()->GetUnfilteredChildren(EntriesToCheck);
					while (EntriesToCheck.Num() > 0)
					{
						UNiagaraStackEntry* Entry = EntriesToCheck.Pop();
						if (UNiagaraStackEntry* Group = Cast<UNiagaraStackEntry>(Entry))
						{
							TArray<UNiagaraStackEntry*> NewEntries;
							Group->GetUnfilteredChildren(NewEntries);
							EntriesToCheck.Append(NewEntries);
						}
						
						if (UNiagaraStackFunctionInput* Input = Cast<UNiagaraStackFunctionInput>(Entry))
						{
							if (Input->GetInputParameterHandle().GetName() == ConditionParameter.GetName())
							{
								TSharedRef<FStructOnScope> ValueStruct = MakeShared<FStructOnScope>(Input->GetLocalValueStruct()->GetStruct());
								*(FNiagaraBool*)ValueStruct->GetStructMemory() = FNiagaraBool(true);
								Input->SetLocalValue(ValueStruct);
							}
							
							if (Input->GetInputParameterHandle().GetName() == ExportParameter.GetName())
							{
								ExportDataInterface->CopyTo(Input->GetDataValueObject());
							}
							
							if (Input->GetInputParameterHandle().GetName() == DeterminismParameter.GetName())
							{
								TSharedRef<FStructOnScope> ValueStruct = MakeShared<FStructOnScope>(Input->GetLocalValueStruct()->GetStruct());
								*(FNiagaraBool*)ValueStruct->GetStructMemory() = FNiagaraBool(true);
								Input->SetLocalValue(ValueStruct);
							}
						}
						
						if (UNiagaraStackPropertyRow* Property = Cast<UNiagaraStackPropertyRow>(Entry))
						{
							TSharedPtr<IPropertyHandle> PropertyHandle = Property->GetDetailTreeNode()->CreatePropertyHandle();
							if (PropertyHandle.IsValid() &&PropertyHandle->GetProperty()->GetFName() == DeterminismParameter.GetName())
							{
								PropertyHandle->SetValue(true);
							}
						}
					}
				}
			}
		}

		NiagaraComponent->GetAsset()->Modify();
		NiagaraComponent->GetAsset()->RequestCompile(false);
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(NiagaraComponent->GetAsset());
	}	

#endif
}