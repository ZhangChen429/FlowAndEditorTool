// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "RHIDefinitions.h"
#include "AdvancedPreviewScene.h"
#include "AlterMeshActor.h"
#include "SAssetEditorViewport.h"
#include "SCommonEditorViewportToolbarBase.h"
#include "Tools/BaseAssetToolkit.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Widgets/Views/SListView.h"

class UAlterMeshAsset;

DECLARE_LOG_CATEGORY_EXTERN(LogAlterMeshEditor, Log, All);

class SAlterMeshEditorViewportToolBar : public SCommonEditorViewportToolbarBase
{
public:
	SLATE_BEGIN_ARGS(SAlterMeshEditorViewportToolBar)
	{}

	SLATE_END_ARGS()

	virtual bool IsViewModeSupported(EViewModeIndex ViewModeIndex) const override;
	void Construct(const FArguments& InArgs, TSharedPtr<class ICommonEditorViewportToolbarInfoProvider> InInfoProvider);
	virtual TSharedRef<SWidget> GenerateShowMenu() const override;
};


class FAlterMeshPreviewScene : public FAdvancedPreviewScene
{
	
public:
	FAlterMeshPreviewScene(ConstructionValues CVS, float InFloorOffset = 0.0f)
		: FAdvancedPreviewScene(CVS, InFloorOffset)
	{
		
	};
	
};

class SAlterMeshEditorViewport : public SAssetEditorViewport, public ICommonEditorViewportToolbarInfoProvider
{
public:
	// SAssetEditorViewport
	void Construct(const FArguments& InArgs, const FAssetEditorViewportConstructionArgs& InViewportConstructionArgs);
	virtual void BindCommands() override;
	virtual TSharedPtr<SWidget> BuildViewportToolbar() override;
	virtual void OnFocusViewportToSelection() override;
	
	TSharedPtr<SOverlay> GetViewportOverlay();
	virtual TSharedRef<SEditorViewport> GetViewportWidget() override;
	virtual TSharedPtr<FExtender> GetExtenders() const override;
	virtual void OnFloatingButtonClicked() override;

};

class FAlterMeshEditorViewportClient : public FEditorViewportClient
{
public:
	FAlterMeshEditorViewportClient(FEditorModeTools* InModeTools, FPreviewScene* InPreviewScene = nullptr, const TWeakPtr<SEditorViewport>& InEditorViewportWidget = nullptr);
	void SetViewportWidget(const TWeakPtr<SEditorViewport> OwnerWidget);
	virtual bool ShouldOrbitCamera() const override { return true; };

	virtual void DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas) override;

	void UpdateShowUVs() const;
	
	void ToggleShowUV0();
	bool IsShowUV0Checked() const { return bShowUVs && UVIndex == 0; };
	
	void ToggleShowUV1();
	bool IsShowUV1Checked() const { return bShowUVs && UVIndex == 1; };
	
	void ToggleShowUV2();
	bool IsShowUV2Checked() const { return bShowUVs && UVIndex == 2; };
	
	void ToggleShowUV3();
	bool IsShowUV3Checked() const { return bShowUVs && UVIndex == 3; };
	
	void UpdateShowColors() const;
	
	void ToggleShowColorsRGB();
	bool IsShowColorsChecked() const { return bShowColors; };
	 
	void ToggleShowColorsAlpha();
	bool IsShowAlphaChecked() const { return bShowAlpha; };
	
	bool bShowUVs;
	int32 UVIndex;
	bool bShowColors;
	bool bShowAlpha;

	TArray<TWeakObjectPtr<UMaterialInterface>> PreviousMaterials;
	TWeakObjectPtr<AActor> PreviewActor;	
};

struct FAlterMeshAttributeRow
{
	FAlterMeshAttributeRow(int32 MeshId, int32 SectionId, int32 VertId, const FVector3f& Vert, const FVector2f& UV0,
		const FVector2f& UV1, const FVector2f& UV2, const FVector2f& UV3, const FLinearColor& Color)
		: MeshId(MeshId),
		  SectionId(SectionId),
		  VertId(VertId),
		  Vert(Vert),
		  UV0(UV0),
		  UV1(UV1),
		  UV2(UV2),
		  UV3(UV3),
		  Color(Color)
	{
	}

	
	int32 MeshId;
	int32 SectionId;
	int32 VertId;
	FVector Vert;
	FVector2f UV0;
	FVector2f UV1;
	FVector2f UV2;
	FVector2f UV3;
	FVector Color;
};

class FAlterMeshAssetEditor : public FBaseAssetToolkit, FTickableGameObject
{
public:
	
	FAlterMeshAssetEditor()
		: FBaseAssetToolkit(nullptr)
	{		
	}
	
	explicit FAlterMeshAssetEditor(UAssetEditor* InOwningAssetEditor)
		: FBaseAssetToolkit(InOwningAssetEditor)
	{
	}

	virtual ~FAlterMeshAssetEditor() override;
	
	virtual AssetEditorViewportFactoryFunction GetViewportDelegate() override;
	virtual TSharedPtr<FEditorViewportClient> CreateEditorViewportClient() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	void UpdateMeshStats();
	TSharedRef<SWidget> CreateOverlayWidget();
	virtual void InitAlterMeshAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UObject* ObjectToEdit);
	virtual void CreateWidgets() override;
	void CreateThumbnail();
	void OnPropChanged(const FPropertyChangedEvent& Event);

	void FocusObject(bool bInstant);

	virtual void AddViewportOverlayWidget(TSharedRef<SWidget> InViewportOverlayWidget, int32 ZOrder = INDEX_NONE);

	TSharedRef<ITableRow> GenerateAttributesRow(TSharedPtr<FAlterMeshAttributeRow> Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<SWidget> CreateAttributesWidget();
	TSharedRef<SDockTab> SpawnAttributesTab(const FSpawnTabArgs& SpawnTabArgs);
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

private:
	
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual bool IsTickableInEditor() const override { return true; }
	virtual TStatId GetStatId() const override;
	
	void BindCommands();

	void OnOpenParent();
	void OnReimportParams();
	void OnReimport();
	void OnOpen();
	void FillPreviousMaterials();

	void RefreshActor();
	void RefreshViewport();
	
	UAlterMeshAssetInterface* EditedAsset = nullptr;

	TStrongObjectPtr<AAlterMeshActor> PreviewActor = nullptr;

	// Mesh Stats
	int32 NumVerts = 0;
	int32 TotalVerts = 0;
	int32 NumInstances = 0;
	int32 NumTriangles = 0;
	int32 TotalTriangles = 0;
	int32 NumSections = 0;
	int32 NumMaterialSlots = 0;
	FVector Size = FVector::Zero();
	int32 NumUVMaps = 0;
	bool bHasVertColors = false;

	TArray<TSharedPtr<FAlterMeshAttributeRow>> Attributes;
	TSharedPtr<SListView<TSharedPtr<FAlterMeshAttributeRow>>> AttributesListView;	
};
