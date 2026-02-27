#pragma once

class STouchCameraViewer_Viewport;

class FTouchCameraView_ViewportModel
{
public:
	static TSharedPtr< FTouchCameraView_ViewportModel> Get();
	

	static TSharedPtr< FTouchCameraView_ViewportModel> Instance;
	
public:
	STouchCameraViewer_Viewport* GetViewport();


	TWeakPtr<STouchCameraViewer_Viewport> Viewport;


	
};


