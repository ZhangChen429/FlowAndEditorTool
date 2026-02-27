// Copyright 2023 Aechmea

#include "AlterMeshEditor.h"

#include "AlterMeshActor.h"
#include "AlterMeshAsset.h"
#include "AlterMeshAssetTypeActions.h"
#include "AlterMeshBrowser.h"
#include "AlterMeshConverterStaticMesh.h"
#include "AlterMeshSplineComponent.h"
#include "AlterMeshSplineComponentVisualizer.h"
#include "AlterMeshTracksEditor.h"
#include "AlterMeshTransformComponent.h"
#include "AlterMeshTransformComponentVisualizer.h"
#include "AssetToolsModule.h"
#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"
#include "DetailsViewObjectFilter.h"
#include "ISequencerModule.h"
#include "Landscape.h"
#include "LevelEditor.h"
#include "NiagaraComponent.h"
#include "ToolMenus.h"
#include "UnrealEdGlobals.h"
#include "Blueprint/UserWidget.h"
#include "CommonParams/AlterMeshBoolParam.h"
#include "CommonParams/AlterMeshCollectionParam.h"
#include "CommonParams/AlterMeshColorParam.h"
#include "CommonParams/AlterMeshFloatParam.h"
#include "CommonParams/AlterMeshIntParam.h"
#include "CommonParams/AlterMeshMenuParam.h"
#include "CommonParams/AlterMeshStringParam.h"
#include "CommonParams/AlterMeshTextureParam.h"
#include "CommonParams/AlterMeshVectorParam.h"
#include "Customization/AlterMeshActorDetailCustomization.h"
#include "Customization/AlterMeshAssetDetailCustomization.h"
#include "Customization/AlterMeshBooleanParamCustomization.h"
#include "Customization/AlterMeshCollectionParamCustomization.h"
#include "Customization/AlterMeshColorParamCustomization.h"
#include "Customization/AlterMeshFloatParamCustomization.h"
#include "Customization/AlterMeshGeometryParamCustomization.h"
#include "Customization/AlterMeshInstancedStructCustomization.h"
#include "Customization/AlterMeshIntParamCustomization.h"
#include "Customization/AlterMeshMenuParamCustomization.h"
#include "Customization/AlterMeshMaterialCustomization.h"
#include "Customization/AlterMeshParamBaseCustomization.h"
#include "Customization/AlterMeshRotationParamCustomization.h"
#include "Customization/AlterMeshStringParamCustomization.h"
#include "Customization/AlterMeshTextureParamCustomization.h"
#include "Customization/AlterMeshVectorParamCustomization.h"
#include "Editor/UnrealEdEngine.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FAlterMeshModule"

void FAlterMeshEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	// Sequencer	
	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");
	SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FAlterMeshTrackMenuBuilder::CreateTrackEditor));
	SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FAlterMeshBoolTrackEditor::CreateTrackEditor));
	SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FAlterMeshFloatTrackEditor::CreateTrackEditor));	
	SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FAlterMeshVectorTrackEditor::CreateTrackEditor));	
	SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FAlterMeshRotationTrackEditor::CreateTrackEditor));
	SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FAlterMeshStringTrackEditor::CreateTrackEditor));	
	SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FAlterMeshIntegerTrackEditor::CreateTrackEditor));
	SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FAlterMeshColorTrackEditor::CreateTrackEditor));
	
	// Custom actor layout
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout(AAlterMeshActor::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAlterMeshActorDetailCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout(UAlterMeshAsset::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAlterMeshAssetDetailCustomization::MakeInstance));

	// Visualizers
	FCoreDelegates::OnPostEngineInit.AddLambda([this]()
	{
		RegisterComponentVisualizer(UAlterMeshTransformComponent::StaticClass()->GetFName(), MakeShareable(new FAlterMeshTransformComponentVisualizer));
		RegisterComponentVisualizer(UAlterMeshSplineComponent::StaticClass()->GetFName(), MakeShareable(new FAlterMeshSplineComponentVisualizer));
	});

	// Param customization
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FAlterMeshParamBaseData::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAlterMeshParamBaseCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FAlterMeshGeometryParamData::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAlterMeshGeometryParamCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FAlterMeshFloatParamData::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAlterMeshFloatParamCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FAlterMeshIntParamData::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAlterMeshIntParamCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FAlterMeshMenuParamData::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAlterMeshMenuParamCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FAlterMeshVectorParamData::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAlterMeshVectorParamCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FAlterMeshRotationParamData::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAlterMeshRotationParamCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FAlterMeshBoolParamData::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAlterMeshBooleanParamCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FAlterMeshCollectionParamData::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAlterMeshCollectionParamCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FAlterMeshStringParamData::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAlterMeshStringParamCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FAlterMeshTextureParamData::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAlterMeshTextureParamCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FAlterMeshColorParamData::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAlterMeshColorParamCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FAlterMeshMaterial::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAlterMeshMaterialCustomization::MakeInstance));

	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FInstancedStruct::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAlterMeshInstancedStructCustomization::MakeInstance), MakeShared<FAlterMeshInstancedStructPropertyTypeIdentifier>());
	
	// Custom asset editor
	IAssetTools& AssetToolsModule =  FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TSharedPtr<FAlterMeshAssetTypeActions> AssetTypeAction = MakeShareable(new FAlterMeshAssetTypeActions());
	AssetToolsModule.RegisterAssetTypeActions(AssetTypeAction.ToSharedRef());

	// Custom style
	FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("AlterMesh"))->GetBaseDir();
	FString ResourcesDir = IPluginManager::Get().FindPlugin(TEXT("AlterMesh"))->GetBaseDir() + "/Resources/";
	PluginDir = FPaths::ConvertRelativePathToFull(PluginDir);

	AlterMeshStyleSet = MakeShareable(new FSlateStyleSet("AlterMesh"));
	AlterMeshStyleSet->SetContentRoot(ResourcesDir);
	AlterMeshStyleSet->SetCoreContentRoot(ResourcesDir);

	AlterMeshStyleSet->Set("ClassIcon.AlterMeshAsset", new FSlateImageBrush(ResourcesDir + "Icon32.png", FVector2D(16.f,16.f)));
	AlterMeshStyleSet->Set("ClassIcon.AlterMeshAssetMedium", new FSlateImageBrush(ResourcesDir + "Icon32.png", FVector2D(32.f,32.f)));
	AlterMeshStyleSet->Set("ClassIcon.AlterMeshActor", new FSlateImageBrush(ResourcesDir + "Actor32.png", FVector2D(16.f,16.f)));
	AlterMeshStyleSet->Set("ClassThumbnail.AlterMeshActor", new FSlateImageBrush(ResourcesDir + "Actor64.png", FVector2D(64.f,64.f)));
	AlterMeshStyleSet->Set("AlterMeshEditor.Refresh", new FSlateImageBrush(ResourcesDir + "Refresh.png", FVector2D(40.f,40.f)));
	AlterMeshStyleSet->Set("AlterMeshEditor.OpenParent", new FSlateImageBrush(ResourcesDir + "Parent.png", FVector2D(40.f,40.f)));
	AlterMeshStyleSet->Set("AlterMeshEditor.ReimportParams", new FSlateImageBrush(ResourcesDir + "Reimport.png", FVector2D(40.f,40.f)));
	AlterMeshStyleSet->Set("AlterMeshEditor.Reimport", new FSlateImageBrush(ResourcesDir + "Reimport.png", FVector2D(40.f,40.f)));
	AlterMeshStyleSet->Set("AlterMeshEditor.Open", new FSlateImageBrush(ResourcesDir + "Open.png", FVector2D(40.f,40.f)));
	AlterMeshStyleSet->Set("AlterMeshBrowser.Browse", new FSlateImageBrush(ResourcesDir + "Icon32.png", FVector2D(40.f,40.f)));
	AlterMeshStyleSet->Set("AlterMeshBrowser.Download", new FSlateImageBrush(ResourcesDir + "Download64.png", FVector2D(40.f,40.f)));

	AlterMeshStyleSet->Set("ClassIcon.AlterMeshGeometryActor", new FSlateImageBrush(ResourcesDir + "Actor_16x.png", FVector2D(16.f,16.f)));
	AlterMeshStyleSet->Set("ClassIcon.AlterMeshGeometryAsset", new FSlateImageBrush(ResourcesDir + "StaticMesh_16x.png", FVector2D(16.f,16.f)));
	AlterMeshStyleSet->Set("ClassIcon.AlterMeshGeometryLandscape", new FSlateImageBrush(ResourcesDir + "Landscape_16x.png", FVector2D(16.f,16.f)));
	AlterMeshStyleSet->Set("ClassIcon.AlterMeshGeometryParticleData", new FSlateImageBrush(ResourcesDir + "ParticleSystem_16x.png", FVector2D(16.f,16.f)));
	AlterMeshStyleSet->Set("ClassIcon.AlterMeshGeometrySpline", new FSlateImageBrush(ResourcesDir + "Spline_16px.png", FVector2D(16.f,16.f)));
	AlterMeshStyleSet->Set("ClassIcon.AlterMeshGeometryTransform", new FSlateImageBrush(ResourcesDir + "SceneComponent_16x.png", FVector2D(16.f,16.f)));

	FSlateStyleRegistry::RegisterSlateStyle(*AlterMeshStyleSet.Get());

	Browser = FAlterMeshBrowser::MakeInstance();
	Browser->AddEditorToolbarButton();
	Browser->RequestFileList();

	if (FSlateApplication::IsInitialized())
	{
		UToolMenu* WindowMenu = UToolMenus::Get()->ExtendMenu("MainFrame.MainMenu.Window");
		FToolMenuSection& Section = WindowMenu->AddSection("AlterMesh", LOCTEXT("AlterMeshDetailsPanel", "AlterMesh"));
		
		Section.AddMenuEntry("AlterMeshDetailsPanel",
			LOCTEXT("AlterMeshDetailsPanel", "AlterMesh Details Panel"),
			LOCTEXT("AlterMeshDetailsPanelTooltip", "Open a Details panel with only AlterMesh parameters"),
			FSlateIcon("AlterMesh", "ClassIcon.AlterMeshAsset"),
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(FTabId("AlterMeshDetailsPanel"));
			}), FCanExecuteAction()));
	}
	
	TSharedRef<FWorkspaceItem> AppMenuGroup = FGlobalTabmanager::Get()->AddLocalWorkspaceMenuCategory(LOCTEXT("AlterMeshMenuGroupName", "AlterMesh Details Panel"));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AlterMeshDetailsPanel"), FOnSpawnTab::CreateRaw(this, &FAlterMeshEditorModule::SpawnAlterMeshDetailsPanel))
	.SetIcon(FSlateIcon("AlterMesh", "ClassIcon.AlterMeshAsset"))
	.SetGroup(AppMenuGroup);
	
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.OnActorSelectionChanged().AddRaw(this, &FAlterMeshEditorModule::SelectionChanged);
	
	auto& MenuExtenders = LevelEditorModule.GetAllLevelViewportContextMenuExtenders();
	MenuExtenders.Add(FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateStatic(&FAlterMeshEditorModule::OnExtendLevelEditorActorContextMenu));
	LevelViewportExtenderHandle = MenuExtenders.Last().GetHandle();
	
#if ALTERMESH_FREE
	FCoreDelegates::OnPostEngineInit.AddLambda([]()
	{
		FText TitleMessage = LOCTEXT("AlterMeshWelcomeTitle", "Accept EULA");
		EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("AlterMeshWelcome", "Thank you for downloading AlterMesh Personal.\nPlease read the EULA on what you can and cannot do with the free version. \nhttps://altermesh.com/eula. Do you accept the EULA? "),
		&TitleMessage);

		if (Result != EAppReturnType::Yes)
		{
			FPlatformMisc::RequestExit(true);
		}
	});

	for (TObjectIterator<UClass> ClassIter; ClassIter; ++ClassIter)
	{
		UClass* Class = *ClassIter;

		if (Class && Class->HasMetaData(TEXT("AlterMeshPro")))
		{
			FString NewDisplayName = TEXT("[PRO] ") + Class->GetMetaData(TEXT("DisplayName"));
			Class->SetMetaData(TEXT("DisplayName"), *NewDisplayName);
		}
	}
	
#endif
	
}

void FAlterMeshEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Unregister styles
	FSlateStyleRegistry::UnRegisterSlateStyle(*AlterMeshStyleSet.Get());

	if (!IsRunningCommandlet())
	{
		// Unregister visualizers
		if(GUnrealEd)
		{
			for(FName ClassName : RegisteredComponentClassNames)
			{
				GUnrealEd->UnregisterComponentVisualizer(ClassName);
			}
		}
	}

	// remove extensions
	if (LevelViewportExtenderHandle.IsValid())
	{
		FLevelEditorModule* LevelEditorModule = FModuleManager::Get().GetModulePtr<FLevelEditorModule>("LevelEditor");
		if (LevelEditorModule)
		{
			typedef FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors DelegateType;
			LevelEditorModule->GetAllLevelViewportContextMenuExtenders().RemoveAll([Handle=LevelViewportExtenderHandle](const DelegateType& In) { return In.GetHandle() == Handle; });
		}
	}
}

void FAlterMeshEditorModule::RegisterComponentVisualizer(FName ComponentClassName, TSharedPtr<FComponentVisualizer> Visualizer)
{
	if (!IsRunningCommandlet())
	{
		GUnrealEd->RegisterComponentVisualizer(ComponentClassName, Visualizer);

		RegisteredComponentClassNames.Add(ComponentClassName);

		if (Visualizer.IsValid())
		{
			Visualizer->OnRegister();
		}
	}
}

TSharedRef<SDockTab> FAlterMeshEditorModule::SpawnAlterMeshDetailsPanel(const FSpawnTabArgs& Args)
{
	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;

	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bShowPropertyMatrixButton = false;
	DetailsViewArgs.bShowSectionSelector = true;

	DetailsViewArgs.ViewIdentifier = "AlterMeshDetailsPanel";
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.bAllowFavoriteSystem = false;
	DetailsViewArgs.bShowOptions = true;
	DetailsViewArgs.bShowModifiedPropertiesOption = true;
	DetailsViewArgs.bShowKeyablePropertiesOption = false;
	DetailsViewArgs.bShowAnimatedPropertiesOption = false;
	DetailsViewArgs.bShowScrollBar = true;
	DetailsViewArgs.bForceHiddenPropertyVisibility = false;
	DetailsViewArgs.bShowCustomFilterOption = true;
			
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);	
	DetailsView->SetIsCustomRowVisibleDelegate(FIsCustomRowVisible::CreateLambda([](FName InRowName, FName InParentName)
	{
		return (InParentName == FName("Parameters"));
	}));
	
	return SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				if (AlterMeshPanelActors.Num() == 0)
				{
					return FText::FromString("No AlterMesh Actor Selected");
				}
				
				if (AlterMeshPanelActors.Num() == 1)
				{
					return FText::FromString(AlterMeshPanelActors[0]->GetActorLabel());
				}

				return FText::FromString(FString::Format(TEXT("{0} AlterMesh actors selected"), { AlterMeshPanelActors.Num() } ));
			})
		]
		+SVerticalBox::Slot()
		[
			DetailsView.ToSharedRef()			
		]
	];
}

TSharedRef<FExtender> FAlterMeshEditorModule::OnExtendLevelEditorActorContextMenu(TSharedRef<FUICommandList> FUICommands, TArray<AActor*> SelectedActors)
{
	TSharedRef<FExtender> Extender(new FExtender());
	
	if (SelectedActors.Num() != 1)
	{
		return Extender;
	}

	if (AAlterMeshActor* AMActor = Cast<AAlterMeshActor>(SelectedActors[0]))
	{
		Extender->AddMenuExtension(
			"ActorGeneral",
			EExtensionHook::Before,
			FUICommands,
			FMenuExtensionDelegate::CreateLambda([AMActor](FMenuBuilder& MenuBuilder)
			{
				MenuBuilder.AddMenuEntry(
					NSLOCTEXT("AMTypeActions", "AlterMesh_CreateInstance", "Create Instance from Actor"),
					NSLOCTEXT("AMTypeActions", "AlterMesh_CreateInstanceTooltip", "Create a child Asset Instance to save your modified params"),
					FSlateIcon("AlterMesh", "ClassIcon.AlterMeshAsset"),
					FUIAction(
					FExecuteAction::CreateLambda([AMActor]()
					{
						if (AMActor->Asset)
						{
							AMActor->Asset = UAlterMeshAssetInstance::CreateInstance(AMActor->Asset->Get(), AMActor->InputParams);
						}
					}),
					FCanExecuteAction()
					)
				);
			}));
		
		Extender->AddMenuExtension(
			"ActorGeneral",
			EExtensionHook::Before,
			FUICommands,
			FMenuExtensionDelegate::CreateLambda([AMActor](FMenuBuilder& MenuBuilder)
			{
				MenuBuilder.AddMenuEntry(
					NSLOCTEXT("AMTypeActions", "AlterMesh_CreateInstance", "Save params as default"),
					NSLOCTEXT("AMTypeActions", "AlterMesh_CreateInstanceTooltip", "Save this as the default for the used asset"),
					FSlateIcon("AlterMesh", "ClassIcon.AlterMeshAsset"),
					FUIAction(
					FExecuteAction::CreateLambda([AMActor]()
					{
						if (AMActor->Asset)
						{
							AMActor->Asset->GetParams().Copy(AMActor->InputParams, AMActor->Asset);
						}
					}),
					FCanExecuteAction()
					)
				);
			}));
	}

	return Extender;
}

void FAlterMeshEditorModule::SelectionChanged(const TArray<UObject*>& Objects, bool bArg)
{
	AlterMeshPanelActors.Empty();
	if (DetailsView)
	{
		TArray<UObject*> FilteredActors;

		for (UObject* Object : Objects)
		{
			if (Cast<AAlterMeshActor>(Object))
			{
				FilteredActors.Add(Object);
				TWeakObjectPtr<AActor> WeakActor = Cast<AActor>(Object);
				AlterMeshPanelActors.Add(WeakActor);
			}
		}

		DetailsView->SetObjects(FilteredActors);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAlterMeshEditorModule, AlterMeshEditor)