#pragma once
#include "SEditorViewport.h"

class FTouchCameraViewer_LevelEditorViewportClient;
class FTouchCameraAssetToolkit;
class UTouchCameraAsset;

class TOUCHPROPERTY_API STouchCameraViewer_Viewport : public SEditorViewport
{
public:
	SLATE_BEGIN_ARGS(STouchCameraViewer_Viewport)	{ }
		SLATE_ARGUMENT(TWeakPtr<FTouchCameraAssetToolkit> , TouchCameraToolkit)
	SLATE_END_ARGS()
	void Construct(const FArguments& InArgs);
	virtual ~STouchCameraViewer_Viewport() override;


	// SEditorViewport
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	// ~SEditorViewport
	
private:
	TWeakPtr<FTouchCameraAssetToolkit> TouchCameraToolkit ;
	
	TSharedPtr<FPreviewScene> PreviewScene;

	TSharedPtr< FTouchCameraViewer_LevelEditorViewportClient> CustomAssetPreviewClient;	
	FDelegateHandle PreviewFeatureLevelChangedHandle;
};


