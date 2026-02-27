#pragma once
#include "SCommonEditorViewportToolbarBase.h"

class UTouchCameraPreviewObject;

class TOUCHPROPERTY_API STouchCameraViewer_EditorViewportToolBar: public SCommonEditorViewportToolbarBase
{
public:
	SLATE_BEGIN_ARGS(STouchCameraViewer_EditorViewportToolBar) {}

	SLATE_END_ARGS()
	TObjectPtr<UTouchCameraPreviewObject> ManagedPreviewObject;
	void Construct(const FArguments& InArgs, const TSharedPtr<class ICommonEditorViewportToolbarInfoProvider>& InInfoProvider);

private:
	virtual void ExtendLeftAlignedToolbarSlots(TSharedPtr<SHorizontalBox> MainBoxPtr, TSharedPtr<SViewportToolBar> ParentToolBarPtr) const override;

private:

	
	
};


