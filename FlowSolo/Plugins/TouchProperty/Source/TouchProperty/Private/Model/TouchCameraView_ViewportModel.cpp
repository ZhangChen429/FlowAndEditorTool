

#include "Model/TouchCameraView_ViewportModel.h"

#include "Slate/TouchCameraViewer_Viewport.h"
TSharedPtr<FTouchCameraView_ViewportModel> FTouchCameraView_ViewportModel::Instance = nullptr;
TSharedPtr< FTouchCameraView_ViewportModel> FTouchCameraView_ViewportModel::Get()
{
	if (Instance == nullptr)
	{
		Instance = MakeShareable(new FTouchCameraView_ViewportModel());
	}
	return Instance;
}

 STouchCameraViewer_Viewport* FTouchCameraView_ViewportModel::GetViewport()
{
	return Viewport.Pin().Get();
}
