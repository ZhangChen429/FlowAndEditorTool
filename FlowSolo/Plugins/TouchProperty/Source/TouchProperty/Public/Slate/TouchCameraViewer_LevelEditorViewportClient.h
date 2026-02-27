#pragma once
#include "LevelEditorViewport.h"
class UTouchCameraAsset;

class TOUCHPROPERTY_API FTouchCameraViewer_LevelEditorViewportClient: public FEditorViewportClient
{
public:
	FTouchCameraViewer_LevelEditorViewportClient(FEditorModeTools* InModeTools, FPreviewScene* InPreviewScene = nullptr,
										const TWeakPtr<SEditorViewport>& InEditorViewportWidget = nullptr,
										const TWeakObjectPtr<UTouchCameraAsset> InEditingObject = nullptr );

	virtual ~FTouchCameraViewer_LevelEditorViewportClient() override;
	void SetEditingObject(UStaticMesh* PreviewObject);
	void OnAssetChanged();
	UFUNCTION()
	void OnEditorObjectChange(UStaticMesh* NewMesh);
	virtual void Tick(float DeltaSeconds) override;
	void SetPreviewScene(FPreviewScene* NewPreviewScene);

	// CinematicControl
	bool AllowsCinematicControlExpose() const { return bAllowCinematicControl; }
	void SetAllowCinematicControlExpose( bool bInAllowCinematicControl) { bAllowCinematicControl = bInAllowCinematicControl; }
private:

	TWeakObjectPtr<UStaticMesh> EditingPreviewMesh;
	TWeakObjectPtr<AStaticMeshActor> MeshActor;
	
	TWeakObjectPtr<UTouchCameraAsset> EditingObject;  // 使用 TObjectPtr 安全引用

	FDelegateHandle ChangeAssetDelegateHandle; 
};
