// Copyright 2023 Aechmea


#include "AlterMeshAssetEditor.h"
#include "RHIDefinitions.h"
#include "AdvancedPreviewScene.h"
#include "AlterMeshAssetEditorCommands.h"
#include "AlterMeshComponent.h"
#include "AlterMeshLibrary.h"
#include "AlterMeshSettings.h"
#include "AssetEditorModeManager.h"
#include "CanvasTypes.h"
#include "ContentBrowserModule.h"
#include "EditorReimportHandler.h"
#include "EditorViewportTabContent.h"
#include "IContentBrowserSingleton.h"
#include "SAssetEditorViewport.h"
#include "SCommonEditorViewportToolbarBase.h"
#include "AlterMesh/AlterMesh.h"
#include "AlterMesh/Public/AlterMeshAsset.h"
#include "AlterMeshEditor/AlterMeshEditor.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/MessageDialog.h"
#include "Slate/SceneViewport.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "AlterMeshEditor"
DEFINE_LOG_CATEGORY(LogAlterMeshEditor)

bool SAlterMeshEditorViewportToolBar::IsViewModeSupported(EViewModeIndex ViewModeIndex) const
{
	return (ViewModeIndex == VMI_Lit)
	|| (ViewModeIndex == VMI_Unlit)
	|| (ViewModeIndex == VMI_Wireframe)
	|| (ViewModeIndex == VMI_Lit_DetailLighting)
	|| (ViewModeIndex == VMI_LightingOnly)
	|| (ViewModeIndex == VMI_ReflectionOverride);
}

void SAlterMeshEditorViewportToolBar::Construct(const FArguments& InArgs,
                                                TSharedPtr<class ICommonEditorViewportToolbarInfoProvider> InInfoProvider)
{
	SCommonEditorViewportToolbarBase::Construct(SCommonEditorViewportToolbarBase::FArguments(), InInfoProvider);

}

TSharedRef<SWidget> SAlterMeshEditorViewportToolBar::GenerateShowMenu() const
{
	TSharedRef<SEditorViewport> ViewportRef = GetInfoProvider().GetViewportWidget();

	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder ShowMenuBuilder(bInShouldCloseWindowAfterMenuSelection, ViewportRef->GetCommandList());
	{
		auto Commands = FAlterMeshAssetEditorCommands::Get();

		ShowMenuBuilder.AddMenuEntry(Commands.ShowUV0);
		ShowMenuBuilder.AddMenuEntry(Commands.ShowUV1);
		ShowMenuBuilder.AddMenuEntry(Commands.ShowUV2);
		ShowMenuBuilder.AddMenuEntry(Commands.ShowUV3);

		ShowMenuBuilder.AddMenuSeparator();

		ShowMenuBuilder.AddMenuEntry(Commands.ShowColors);
		ShowMenuBuilder.AddMenuEntry(Commands.ShowAlpha);

	}

	return ShowMenuBuilder.MakeWidget();
}

void SAlterMeshEditorViewport::Construct(const FArguments& InArgs,
                                         const FAssetEditorViewportConstructionArgs& InViewportConstructionArgs)
{
	SAssetEditorViewport::Construct(InArgs, InViewportConstructionArgs);
	Client->Viewport = SceneViewport.Get();
}

void SAlterMeshEditorViewport::BindCommands()
{
	SAssetEditorViewport::BindCommands();
	
	const FAlterMeshAssetEditorCommands& Commands = FAlterMeshAssetEditorCommands::Get();

	TSharedPtr<FAlterMeshEditorViewportClient> ViewportClient = StaticCastSharedPtr<FAlterMeshEditorViewportClient>(Client);
	
	CommandList->MapAction(
		Commands.ShowUV0,
		FExecuteAction::CreateRaw( ViewportClient.Get(), &FAlterMeshEditorViewportClient::ToggleShowUV0 ),
		FCanExecuteAction(),
		FIsActionChecked::CreateRaw( ViewportClient.Get(), &FAlterMeshEditorViewportClient::IsShowUV0Checked ) );
	
	CommandList->MapAction(
		Commands.ShowUV1,
		FExecuteAction::CreateRaw( ViewportClient.Get(), &FAlterMeshEditorViewportClient::ToggleShowUV1 ),
		FCanExecuteAction(),
		FIsActionChecked::CreateRaw( ViewportClient.Get(), &FAlterMeshEditorViewportClient::IsShowUV1Checked ) );
	
	CommandList->MapAction(
		Commands.ShowUV2,
		FExecuteAction::CreateRaw( ViewportClient.Get(), &FAlterMeshEditorViewportClient::ToggleShowUV2),
		FCanExecuteAction(),
		FIsActionChecked::CreateRaw( ViewportClient.Get(), &FAlterMeshEditorViewportClient::IsShowUV2Checked ) );
	
	CommandList->MapAction(
		Commands.ShowUV3,
		FExecuteAction::CreateRaw( ViewportClient.Get(), &FAlterMeshEditorViewportClient::ToggleShowUV3 ),
		FCanExecuteAction(),
		FIsActionChecked::CreateRaw( ViewportClient.Get(), &FAlterMeshEditorViewportClient::IsShowUV3Checked ) );
	
	CommandList->MapAction(
		Commands.ShowColors,
		FExecuteAction::CreateRaw( ViewportClient.Get(), &FAlterMeshEditorViewportClient::ToggleShowColorsRGB ),
		FCanExecuteAction(),
		FIsActionChecked::CreateRaw( ViewportClient.Get(), &FAlterMeshEditorViewportClient::IsShowColorsChecked ) );
	
	CommandList->MapAction(
		Commands.ShowAlpha,
		FExecuteAction::CreateRaw( ViewportClient.Get(), &FAlterMeshEditorViewportClient::ToggleShowColorsAlpha ),
		FCanExecuteAction(),
		FIsActionChecked::CreateRaw( ViewportClient.Get(), &FAlterMeshEditorViewportClient::IsShowAlphaChecked ) );
	
}

TSharedPtr<SOverlay> SAlterMeshEditorViewport::GetViewportOverlay()
{
	return ViewportOverlay;
}

TSharedPtr<SWidget> SAlterMeshEditorViewport::BuildViewportToolbar()
{
	return SNew(SAlterMeshEditorViewportToolBar, SharedThis(this));
}

void SAlterMeshEditorViewport::OnFocusViewportToSelection()
{
	if (Client)
	{
		FBox BoundingBox = StaticCastSharedPtr<FAlterMeshEditorViewportClient>(Client)->PreviewActor->GetComponentsBoundingBox();
		BoundingBox = BoundingBox.ExpandBy(FVector(0.f, 0.f, BoundingBox.GetExtent().Z));
	
		if (BoundingBox.IsValid)
		{
			Client->FocusViewportOnBox(BoundingBox, false);
		}
	}
}

TSharedRef<SEditorViewport> SAlterMeshEditorViewport::GetViewportWidget()
{
	return SharedThis(this);
}

TSharedPtr<FExtender> SAlterMeshEditorViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}

void SAlterMeshEditorViewport::OnFloatingButtonClicked()
{
}

FAlterMeshEditorViewportClient::FAlterMeshEditorViewportClient(FEditorModeTools* InModeTools,
                                                               FPreviewScene* InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewportWidget)
		: FEditorViewportClient(InModeTools, InPreviewScene, InEditorViewportWidget)
{
	bShowColors = false;
	bShowUVs = false;
	bShowAlpha = false;
	UVIndex = 0;
}

void FAlterMeshEditorViewportClient::SetViewportWidget(const TWeakPtr<SEditorViewport> OwnerWidget)
{
	EditorViewportWidget = OwnerWidget;
}

void FAlterMeshEditorViewportClient::DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas)
{
	FEditorViewportClient::DrawCanvas(InViewport, View, Canvas);
}

void FAlterMeshEditorViewportClient::UpdateShowUVs() const
{
	const UAlterMeshEditorSettings* Settings = GetDefault<UAlterMeshEditorSettings>();
	if (PreviewActor.IsValid())
	{
		int32 Section = 0;
		TArray<UAlterMeshComponent*> Components;
		PreviewActor->GetComponents<UAlterMeshComponent>(Components);
		for (UAlterMeshComponent* Component : Components)
		{
			for (int32 i = 0; i < Component->GetNumMaterials(); i++)
			{
				if (bShowUVs)
				{
					Component->SetMaterial(i, Settings->MeshPreviewUVs.LoadSynchronous());

					if (UMaterialInstanceDynamic* DynMaterial = Component->CreateDynamicMaterialInstance(i))
					{
						DynMaterial->SetScalarParameterValue(FName("UVIndex"), UVIndex);
					}
				}
				else
				{
					Component->SetMaterial(i, PreviousMaterials[Section].Get());
				}
				
				Section++;
			}
		}
	}
}

void FAlterMeshEditorViewportClient::ToggleShowUV0()
{
	bShowColors = false;
	bShowAlpha = false;
	bShowUVs = !bShowUVs || UVIndex != 0;
	UVIndex = 0;
	UpdateShowUVs();
}

void FAlterMeshEditorViewportClient::ToggleShowUV1()
{
	bShowColors = false;
	bShowAlpha = false;
	bShowUVs = !bShowUVs || UVIndex != 1;
	UVIndex = 1;
	UpdateShowUVs();
}

void FAlterMeshEditorViewportClient::ToggleShowUV2()
{
	bShowColors = false;
	bShowAlpha = false;
	bShowUVs = !bShowUVs || UVIndex != 2;
	UVIndex = 2;
	UpdateShowUVs();
}

void FAlterMeshEditorViewportClient::ToggleShowUV3()
{
	bShowColors = false;
	bShowAlpha = false;
	bShowUVs = !bShowUVs || UVIndex != 3;
	UVIndex = 3;
	UpdateShowUVs();
}

void FAlterMeshEditorViewportClient::UpdateShowColors() const
{	
	const UAlterMeshEditorSettings* Settings = GetDefault<UAlterMeshEditorSettings>();
	if (PreviewActor.IsValid())
	{
		int32 Section = 0;
		TArray<UAlterMeshComponent*> Components;
		PreviewActor->GetComponents<UAlterMeshComponent>(Components);
		for (UAlterMeshComponent* Component : Components)
		{
			for (int32 i = 0; i < Component->GetNumMaterials(); i++)
			{
				
				if (bShowColors || bShowAlpha)
				{
					Component->SetMaterial(i, Settings->MeshPreviewVertexColors.LoadSynchronous());
					
					if (UMaterialInstanceDynamic* DynMaterial = Component->CreateDynamicMaterialInstance(i))
					{
						DynMaterial->SetScalarParameterValue(FName("Alpha"), bShowAlpha);
					}
				}
				else
				{
					Component->SetMaterial(i, PreviousMaterials[Section].Get());
				}
				Section++;
			}
		}
	}
}

void FAlterMeshEditorViewportClient::ToggleShowColorsRGB()
{
	bShowColors = !bShowColors;
	bShowAlpha = false;
	bShowUVs = false;
	UpdateShowColors();
}

void FAlterMeshEditorViewportClient::ToggleShowColorsAlpha()
{
	bShowAlpha = !bShowAlpha;
	bShowColors = false;
	bShowUVs = false;
	UpdateShowColors();
}

FAlterMeshAssetEditor::~FAlterMeshAssetEditor()
{
	PreviewActor->Destroy();
	ViewportClient->Viewport->Destroy();
	ViewportClient->GetWorld()->DestroyWorld(true);
	ViewportClient.Reset();
	DetailsView.Reset();
}

AssetEditorViewportFactoryFunction FAlterMeshAssetEditor::GetViewportDelegate()
{
	// Set up functor for viewport tab
	AssetEditorViewportFactoryFunction TempViewportDelegate = [this](const FAssetEditorViewportConstructionArgs& InArgs)
	{
		TSharedRef<SAlterMeshEditorViewport> Widget = SNew(SAlterMeshEditorViewport, InArgs)
			.EditorViewportClient(ViewportClient);

		StaticCastSharedPtr<FAlterMeshEditorViewportClient>(ViewportClient)->SetViewportWidget(Widget);
		return Widget;
	};

	return TempViewportDelegate;
}

TSharedPtr<FEditorViewportClient> FAlterMeshAssetEditor::CreateEditorViewportClient() const
{
	FAlterMeshPreviewScene* PreviewScene = new FAlterMeshPreviewScene(FAdvancedPreviewScene::ConstructionValues(), 0.f);
	StaticCastSharedPtr<FAssetEditorModeManager>(EditorModeManager)->SetPreviewScene(PreviewScene);

	return MakeShared<FAlterMeshEditorViewportClient>(EditorModeManager.Get(), PreviewScene, nullptr);
}

FLinearColor FAlterMeshAssetEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.02,0.93,0.89);
}

FName FAlterMeshAssetEditor::GetToolkitFName() const
{
	return FName("AlterMeshEditor");
}

FText FAlterMeshAssetEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AlterMeshLabel", "AlterMesh Editor" );
}

FString FAlterMeshAssetEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("AlterMeshEditor");
}

void FAlterMeshAssetEditor::UpdateMeshStats()
{
	NumVerts = 0;
	NumTriangles = 0;
	NumSections = 0;
	NumMaterialSlots = PreviewActor ? PreviewActor->Asset->Get()->Materials.Num() : 0;
	NumInstances = 0;
	TotalVerts = 0;
	TotalTriangles = 0;
	Size = PreviewActor ? PreviewActor->GetComponentsBoundingBox().GetExtent() * 2.f : FVector::Zero();	
	NumUVMaps = 0;
	bHasVertColors = false;
	Attributes.Empty();
	
	if (PreviewActor.IsValid())
	{
		int32 MeshId = 0;
		for (const UAlterMeshComponent* Component : PreviewActor->AlterMeshComponents)
		{
			int32 SectionId = 0;
			for (const FAlterMeshSection& Section : Component->Sections)
			{
				NumVerts += Section.Vertices.Num();
				NumTriangles += Section.Indices.Num() / 3;
				NumSections++;
				NumInstances += Section.Instances.Num();
				TotalVerts += Section.Vertices.Num() * Section.Instances.Num();
				TotalTriangles += Section.Indices.Num() / 3 * Section.Instances.Num();
				
				NumUVMaps = FMath::Max(Section.UV0.Num() ? 1 : 0, NumUVMaps);
				NumUVMaps = FMath::Max(Section.UV1.Num() ? 2 : 0, NumUVMaps);
				NumUVMaps = FMath::Max(Section.UV2.Num() ? 3 : 0, NumUVMaps);
				NumUVMaps = FMath::Max(Section.UV3.Num() ? 4 : 0, NumUVMaps);

				bHasVertColors = bHasVertColors || Section.Colors.Num();

				for (int32 i = 0; i < Section.Vertices.Num(); i++)
				{
					TSharedPtr<FAlterMeshAttributeRow> Row = MakeShareable(new FAlterMeshAttributeRow(MeshId, SectionId, i, Section.Vertices[i],
						Section.UV0.IsValidIndex(i) ? Section.UV0[i] : FVector2f::Zero(),
						Section.UV1.IsValidIndex(i) ? Section.UV1[i] : FVector2f::Zero(),
						Section.UV2.IsValidIndex(i) ? Section.UV2[i] : FVector2f::Zero(),
						Section.UV3.IsValidIndex(i) ? Section.UV3[i] : FVector2f::Zero(),
						Section.Colors.IsValidIndex(i) ? Section.Colors[i] : FLinearColor::Black));
					Attributes.Add(Row);
				}
				
				SectionId++;
			}

			// Each section has repeating instances
			NumInstances = NumInstances / FMath::Max(Component->Sections.Num(), 1);

			MeshId++;
		}
	}

	if (AttributesListView.IsValid())
	{
		AttributesListView->RequestListRefresh();
	}
}

TSharedRef<SWidget> FAlterMeshAssetEditor::CreateOverlayWidget()
{
	const FAlterMeshEditorModule& AlterMeshEditorModule = FModuleManager::Get().LoadModuleChecked<FAlterMeshEditorModule>(TEXT("AlterMeshEditor"));
	
	return
	SNew(SHorizontalBox)
	+SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(30)
	[		
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		[		
			SNew(SBorder)
			.BorderImage( FAppStyle::Get().GetBrush( "FloatingBorder" ) )
			.Padding(10)
			.Content()
			[
				SNew(SVerticalBox)				
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
					.WidthOverride(64)
					.HeightOverride(64)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)					
					[
						SNew(SImage)
						.Image(AlterMeshEditorModule.AlterMeshStyleSet->GetBrush("ClassIcon.AlterMeshAssetMedium") )
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this](){ return FText::FromString(TEXT("Preview only, may change after baking")); })
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this](){ return FText::FromString(FString::Format(TEXT("Vertices: {0}"), { FText::AsNumber(NumVerts).ToString() } )); })
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this](){ return FText::FromString(FString::Format(TEXT("Triangles: {0}"), { FText::AsNumber(NumTriangles).ToString() } )); })
				]				
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this](){ return FText::FromString(FString::Format(TEXT("Instances: {0}"), { FText::AsNumber(NumInstances).ToString() } )); })
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this](){ return FText::FromString(FString::Format(TEXT("Total Vertices: {0}"), { FText::AsNumber(TotalVerts).ToString() } )); })
					]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this](){ return FText::FromString(FString::Format(TEXT("Total Triangles: {0}"), { FText::AsNumber(TotalTriangles).ToString() } )); })
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this](){ return FText::FromString(FString::Format(TEXT("Sections: {0}"), { FText::AsNumber(NumSections).ToString() } )); })
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this](){ return FText::FromString(FString::Format(TEXT("Material Slots: {0}"), { NumMaterialSlots } )); })
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this](){ return FText::FromString(FString::Format(TEXT("UV Maps: {0}"), { NumUVMaps } )); })
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this](){ return FText::FromString(FString::Format(TEXT("Has Vertex Colors: {0}"), { bHasVertColors ? TEXT("True") : TEXT("False") } )); })
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this](){ return FText::FromString(FString::Format(TEXT("Size: {0}"), { Size.ToString() } )); })
				]
			]
		]
	];
}

void FAlterMeshAssetEditor::InitAlterMeshAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UObject* ObjectToEdit)
{
	CreateWidgets();
	SetEditingObject(ObjectToEdit);
	EditedAsset = Cast<UAlterMeshAssetInterface>(ObjectToEdit);
	FAlterMeshAssetEditorCommands::Register();

	BindCommands();

	struct Extender
	{
		static void FillToolbar(FToolBarBuilder& ToolbarBuilder, UObject* ObjectToEdit )
		{			
			if (Cast<UAlterMeshAssetInstance>(ObjectToEdit))
			{
				ToolbarBuilder.AddToolBarButton(FAlterMeshAssetEditorCommands::Get().OpenParent);
			}			
			// Dont show reimport on instances
			else
			{
				ToolbarBuilder.AddToolBarButton(FAlterMeshAssetEditorCommands::Get().ReimportParams);
				ToolbarBuilder.AddToolBarButton(FAlterMeshAssetEditorCommands::Get().Reimport);
			}
			ToolbarBuilder.AddToolBarButton(FAlterMeshAssetEditorCommands::Get().Refresh);
			ToolbarBuilder.AddToolBarButton(FAlterMeshAssetEditorCommands::Get().Open);
		}
	};

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, GetToolkitCommands(),
						FToolBarExtensionDelegate::CreateStatic(&Extender::FillToolbar, ObjectToEdit));
	AddToolbarExtender(ToolbarExtender);

	FAssetEditorToolkit::InitAssetEditor(
		Mode,
		InitToolkitHost,
		FName("AlterMeshEditor"),
		StandaloneDefaultLayout.ToSharedRef(),
		true,
		true,
		ObjectToEdit);

	FActorSpawnParameters Params;
	Params.bNoFail = true;
	PreviewActor = TStrongObjectPtr(ViewportClient->GetWorld()->SpawnActor<AAlterMeshActor>(Params));
	PreviewActor->Asset = EditedAsset;
	PreviewActor->InitializeParamsFromAsset();
	if (PreviewActor->GetOnRefreshDelegate())
	{
		PreviewActor->GetOnRefreshDelegate()->AddRaw(this, &FAlterMeshAssetEditor::RefreshViewport);
	}
	PreviewActor->RefreshAsync();
	
	StaticCastSharedPtr<FAlterMeshEditorViewportClient>(ViewportClient)->PreviewActor = PreviewActor.Get();
	ViewportClient->MoveViewportCamera(FVector::ZeroVector, FRotator(-25.f, 25.f, 0.f));
	ViewportClient->bLockFlightCamera = true;
	ViewportClient->ToggleOrbitCamera(true);
	ViewportClient->FocusViewportOnBox(FBox(ForceInit).ExpandBy(500.f), true);
	
	ToolkitHost.Pin()->AddViewportOverlayWidget(CreateOverlayWidget());	
	DetailsView->OnFinishedChangingProperties().AddSP(this, &FAlterMeshAssetEditor::OnPropChanged);
}

void FAlterMeshAssetEditor::CreateWidgets()
{
	FBaseAssetToolkit::CreateWidgets();
}

void FAlterMeshAssetEditor::CreateThumbnail()
{
	if (ViewportClient->Viewport)
	{
		// Setup camera for thumbnail
		((FAlterMeshPreviewScene*)StaticCastSharedPtr<FAssetEditorModeManager>(EditorModeManager)->GetPreviewScene())->SetEnvironmentVisibility(false);		
		((FAlterMeshPreviewScene*)StaticCastSharedPtr<FAssetEditorModeManager>(EditorModeManager)->GetPreviewScene())->SetFloorVisibility(false);
		
		ViewportClient->Viewport->Draw();
		IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
		TArray<FAssetData> Assets;
		Assets.Add(FAssetData(EditedAsset));
	
		ContentBrowser.CaptureThumbnailFromViewport(ViewportClient->Viewport, Assets);

		// Revert preview camera
		((FAlterMeshPreviewScene*)StaticCastSharedPtr<FAssetEditorModeManager>(EditorModeManager)->GetPreviewScene())->SetFloorVisibility(true);
		((FAlterMeshPreviewScene*)StaticCastSharedPtr<FAssetEditorModeManager>(EditorModeManager)->GetPreviewScene())->SetEnvironmentVisibility(true);
	}
}

void FAlterMeshAssetEditor::OnPropChanged(const FPropertyChangedEvent& Event)
{
	RefreshActor();
}

void FAlterMeshAssetEditor::FocusObject(bool bInstant)
{
	FBox BoundingBox = PreviewActor->GetComponentsBoundingBox();
	BoundingBox = BoundingBox.ExpandBy(FVector(0.f, 0.f, BoundingBox.GetExtent().Z));
	
	if (BoundingBox.IsValid)
	{
		if (BoundingBox.GetSize().IsNearlyZero())
		{
			BoundingBox += FBox(ForceInit).ExpandBy(100.f);
		}

		if (ViewportClient->Viewport)
		{
			ViewportClient->FocusViewportOnBox(BoundingBox, bInstant);
		}
	}
}

void FAlterMeshAssetEditor::AddViewportOverlayWidget(TSharedRef<SWidget> InViewportOverlayWidget, int32 ZOrder)
{

	if (ViewportTabContent.IsValid())
	{
		TSharedPtr<SAlterMeshEditorViewport> Viewport = StaticCastSharedPtr<SAlterMeshEditorViewport>(ViewportTabContent->GetFirstViewport());

		if (Viewport.IsValid() && Viewport->GetViewportOverlay().IsValid())
		{
			Viewport->GetViewportOverlay()->AddSlot()
			.ZOrder(ZOrder)
			[
				InViewportOverlayWidget
			];
		}
	}
}

TSharedRef<ITableRow> FAlterMeshAssetEditor::GenerateAttributesRow(TSharedPtr<FAlterMeshAttributeRow> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FAlterMeshAttributeRow>>, OwnerTable)
	.ShowWires(true)
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(0.2)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Format(TEXT("{0}"), { Item->MeshId } )))
			]
			+SHorizontalBox::Slot()
			.FillWidth(0.2)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Format(TEXT("{0}"), { Item->SectionId } )))
			]
			+SHorizontalBox::Slot()
			.FillWidth(0.2)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Format(TEXT("{0}"), { Item->VertId } )))
			]
			+SHorizontalBox::Slot()
			.FillWidth(1)
			[
				SNew(STextBlock)
				.Text(FText::FromString( Item->Vert.ToString()  ))
			]
			+SHorizontalBox::Slot()
			.FillWidth(0.5)
			[
				SNew(STextBlock)
				.Text(FText::FromString( Item->UV0.ToString() ))
			]
			+SHorizontalBox::Slot()
			.FillWidth(0.5)
			[
				SNew(STextBlock)
				.Text(FText::FromString( Item->UV1.ToString() ))
			]
			+SHorizontalBox::Slot()
			.FillWidth(0.5)
			[
				SNew(STextBlock)
				.Text(FText::FromString( Item->UV2.ToString() ))
			]
			+SHorizontalBox::Slot()
			.FillWidth(0.5)
			[
				SNew(STextBlock)
				.Text(FText::FromString( Item->UV3.ToString() ))
			]
			+SHorizontalBox::Slot()
			.FillWidth(1)
			[
				SNew(STextBlock)
				.Text(FText::FromString( Item->Color.ToString() ))
			]
		]
	]
	
	;
}

TSharedRef<SWidget> FAlterMeshAssetEditor::CreateAttributesWidget()
{
	return
	SNew(SVerticalBox)
	+SVerticalBox::Slot()
	.AutoHeight()
	[		
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.FillWidth(0.2)
		[
			SNew(STextBlock)
			.Text(FText::FromString("MeshId"))
		]
		+SHorizontalBox::Slot()
		.FillWidth(0.2)
		[
			SNew(STextBlock)
			.Text(FText::FromString("SectionId"))
		]
		+SHorizontalBox::Slot()
		.FillWidth(0.2)
		[
			SNew(STextBlock)
			.Text(FText::FromString("VertId"))
		]
		+SHorizontalBox::Slot()
		.FillWidth(1)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Vertex"))
		]
		+SHorizontalBox::Slot()
		.FillWidth(0.5)
		[
			SNew(STextBlock)
			.Text(FText::FromString("UV0"))
		]
		+SHorizontalBox::Slot()
		.FillWidth(0.5)
		[
			SNew(STextBlock)
			.Text(FText::FromString("UV1"))
		]
		+SHorizontalBox::Slot()
		.FillWidth(0.5)
		[
			SNew(STextBlock)
			.Text(FText::FromString("UV2"))
		]
		+SHorizontalBox::Slot()
		.FillWidth(0.5)
		[
			SNew(STextBlock)
			.Text(FText::FromString("UV3"))
		]
		+SHorizontalBox::Slot()
		.FillWidth(1)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Color"))
		]
	]
	+SVerticalBox::Slot()
	[
		SAssignNew(AttributesListView, SListView<TSharedPtr<FAlterMeshAttributeRow>>)
		.Orientation(EOrientation::Orient_Vertical)
		.OnGenerateRow_Raw(this, &FAlterMeshAssetEditor::GenerateAttributesRow)
		.ScrollbarVisibility(EVisibility::Visible)
		.ListItemsSource(&Attributes)
	];
	
}

TSharedRef<SDockTab> FAlterMeshAssetEditor::SpawnAttributesTab(const FSpawnTabArgs& SpawnTabArgs)
{
	TSharedPtr<SDockTab> DetailsTab = SNew(SDockTab)
		.Label(LOCTEXT("AttributesTitle", "Attributes"))
		[
			CreateAttributesWidget()
		];

	return DetailsTab.ToSharedRef();
}

void FAlterMeshAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FBaseAssetToolkit::RegisterTabSpawners(InTabManager);
	
	const FAlterMeshEditorModule& AlterMeshEditorModule = FModuleManager::Get().LoadModuleChecked<FAlterMeshEditorModule>(TEXT("AlterMeshEditor"));
	
	InTabManager->RegisterTabSpawner("AlterMeshEditorAttributes", FOnSpawnTab::CreateSP(this, &FAlterMeshAssetEditor::SpawnAttributesTab))
		.SetDisplayName(LOCTEXT("Attributes", "Attributes"))
		.SetGroup(AssetEditorTabsCategory.ToSharedRef())
		.SetIcon(FSlateIcon("AlterMesh", "ClassIcon.AlterMeshAsset"));
}

void FAlterMeshAssetEditor::Tick(float DeltaTime)
{

}

TStatId FAlterMeshAssetEditor::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FAlterMeshAssetEditor, STATGROUP_Tickables);
}

void FAlterMeshAssetEditor::BindCommands()
{
	ToolkitCommands->MapAction(
		FAlterMeshAssetEditorCommands::Get().OpenParent,
		FExecuteAction::CreateSP(this, &FAlterMeshAssetEditor::OnOpenParent),
		FCanExecuteAction::CreateLambda([](){ return true; }));
	
	ToolkitCommands->MapAction(
		FAlterMeshAssetEditorCommands::Get().ReimportParams,
		FExecuteAction::CreateSP(this, &FAlterMeshAssetEditor::OnReimportParams),
		FCanExecuteAction::CreateLambda([](){ return true; }));

	ToolkitCommands->MapAction(
		FAlterMeshAssetEditorCommands::Get().Reimport,
		FExecuteAction::CreateSP(this, &FAlterMeshAssetEditor::OnReimport),
		FCanExecuteAction::CreateLambda([](){ return true; }));

	ToolkitCommands->MapAction(
		FAlterMeshAssetEditorCommands::Get().Open,
		FExecuteAction::CreateSP(this, &FAlterMeshAssetEditor::OnOpen),
		FCanExecuteAction::CreateLambda([](){ return true; }));
	
	ToolkitCommands->MapAction(
		FAlterMeshAssetEditorCommands::Get().Refresh,
		FExecuteAction::CreateSP(this, &FAlterMeshAssetEditor::RefreshActor),
		FCanExecuteAction::CreateLambda([](){ return true; }));

}

void FAlterMeshAssetEditor::OnOpenParent()
{
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(EditedAsset->Get());
}

void FAlterMeshAssetEditor::OnReimportParams()
{
	if (EditedAsset)
	{
		EditedAsset->Get()->bReimportOnlyParams = true;
		FReimportManager::Instance()->Reimport(EditedAsset, true);
		
		if (DetailsView.IsValid())
		{
			DetailsView.Get()->SetObject(EditedAsset, true);
		}
	}
}

void FAlterMeshAssetEditor::OnReimport()
{
	if (EditedAsset)
	{
		EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString("Reimporting the file will reset all your parameters to it's default values.\nContinue?"));

		if (Result == EAppReturnType::Yes)
		{
			EditedAsset->Get()->bReimportOnlyParams = false;
			FReimportManager::Instance()->Reimport(EditedAsset, true);

			if (DetailsView.IsValid())
			{
				DetailsView.Get()->SetObject(EditedAsset, true);
			}
		}
	}
}

void FAlterMeshAssetEditor::OnOpen()
{
	if (EditedAsset)
	{

		const UAlterMeshSettings* Settings = GetDefault<UAlterMeshSettings>();
		const FString URL = FPaths::ConvertRelativePathToFull(Settings->ExecutablePath.FilePath);
		const FString FilePath = UAlterMeshLibrary::ConvertFilenameToFull(EditedAsset->Get()->Filename.FilePath);
		const FString ProcParams = FString(" --factory-startup ") + TEXT("\"") + FilePath + TEXT("\"");

		const bool bFileExists = FPaths::FileExists(FilePath);
		const bool bIsBlendFile = FPaths::GetExtension(FilePath) == FString("Blend");

		FProcHandle Handle;

		if (bFileExists && bIsBlendFile)
		{
			Handle = FPlatformProcess::CreateProc(*URL, *ProcParams, true, false, false, nullptr, 0, nullptr, nullptr);
		}

		FNotificationInfo Info(FText::GetEmpty());
		Info.ExpireDuration = 2.0f;

		if (Handle.IsValid())
		{
			Info.Text = FText::Format(LOCTEXT("AlterMeshAssetEditorOpen", "Opening {0}"), FText::FromString(EditedAsset->Get()->Filename.FilePath));
		}
		else
		{
			Info.Text = FText::Format(LOCTEXT("AlterMeshAssetEditorOpenFailed", "Could not open file: {0}"), FText::FromString(EditedAsset->Get()->Filename.FilePath));
			Info.Image = FCoreStyle::Get().GetBrush(TEXT("MessageLog.Warning"));
		}

		FSlateNotificationManager::Get().AddNotification(Info);
	}
}

void FAlterMeshAssetEditor::FillPreviousMaterials()
{
	StaticCastSharedPtr<FAlterMeshEditorViewportClient>(ViewportClient)->PreviousMaterials.Empty();;
	
	TArray<UAlterMeshComponent*> Components;
	PreviewActor->GetComponents<UAlterMeshComponent>(Components);
	for (UAlterMeshComponent* Component : Components)
	{
		for (int32 i = 0; i < Component->GetNumMaterials(); i++)
		{
			StaticCastSharedPtr<FAlterMeshEditorViewportClient>(ViewportClient)->PreviousMaterials.Add(Component->GetMaterial(i));
		}
	}
}

void FAlterMeshAssetEditor::RefreshActor()
{
	PreviewActor->OverrideMaterials.Empty();
	PreviewActor->InitializeParamsFromAsset();
	PreviewActor->RefreshAsync();
}

void FAlterMeshAssetEditor::RefreshViewport()
{
	CreateThumbnail();
	FocusObject(false);
	UpdateMeshStats();
	FillPreviousMaterials();
	
	if (ViewportClient.IsValid())
	{
		if (auto CustomViewport = StaticCastSharedPtr<FAlterMeshEditorViewportClient>(ViewportClient))
		{
			if (CustomViewport->bShowColors || CustomViewport->bShowAlpha)
			{
				CustomViewport->UpdateShowColors();
			}
			else
			{
				CustomViewport->UpdateShowUVs();				
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
