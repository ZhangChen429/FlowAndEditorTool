
#include "Slate/TouchCameraViewer_Viewport.h"
#include "AdvancedPreviewScene.h"
#include "SEditorViewport.h"
#include "Slate/TouchCameraViewer_LevelEditorViewportClient.h"
#include "FactoryAndMode/FTouchCameraAssetToolkit.h"



void STouchCameraViewer_Viewport::Construct(const FArguments& InArgs )
{
	TouchCameraToolkit = InArgs._TouchCameraToolkit;

	
	SEditorViewport::Construct(SEditorViewport::FArguments());
}

STouchCameraViewer_Viewport::~STouchCameraViewer_Viewport()
{
	UEditorEngine* Editor = CastChecked<UEditorEngine>(GEngine);
	if (Editor) {
		Editor->OnPreviewFeatureLevelChanged().Remove(PreviewFeatureLevelChangedHandle);
	}

	PreviewScene.Reset();

}

TSharedRef<FEditorViewportClient> STouchCameraViewer_Viewport::MakeEditorViewportClient()
{
	FAdvancedPreviewScene::ConstructionValues ConstructionValues;
	ConstructionValues.LightBrightness = 4.0f;
	if (!PreviewScene.IsValid())
	{
		PreviewScene = MakeShareable(new FAdvancedPreviewScene(ConstructionValues));
		
	}
	
	if (!CustomAssetPreviewClient.IsValid())		
		CustomAssetPreviewClient = MakeShareable(new FTouchCameraViewer_LevelEditorViewportClient(nullptr,
			PreviewScene.Get(),nullptr,	TouchCameraToolkit.Pin()->GetTouchCameraObject()));
	
	return CustomAssetPreviewClient.ToSharedRef();
}
