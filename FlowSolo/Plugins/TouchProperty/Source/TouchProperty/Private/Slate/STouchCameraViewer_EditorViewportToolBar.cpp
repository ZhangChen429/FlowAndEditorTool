
#include "..\..\Public\Slate\TouchCameraViewer_EditorViewportToolBar.h"

#include "PreviewProfileController.h"
#include "SCommonEditorViewportToolbarBase.h"

void STouchCameraViewer_EditorViewportToolBar::Construct(const FArguments& InArgs,
                                                         const TSharedPtr<class ICommonEditorViewportToolbarInfoProvider>& InInfoProvider)
{

	//ManagedPreviewObject = InArgs._ManagedPreviewObject;
	
	SCommonEditorViewportToolbarBase::Construct(SCommonEditorViewportToolbarBase::FArguments().
		PreviewProfileController(MakeShared<FPreviewProfileController>()), InInfoProvider);
}

 void STouchCameraViewer_EditorViewportToolBar::ExtendLeftAlignedToolbarSlots(
	TSharedPtr<SHorizontalBox> MainBoxPtr, TSharedPtr<SViewportToolBar> ParentToolBarPtr) const
{
	SCommonEditorViewportToolbarBase::ExtendLeftAlignedToolbarSlots(MainBoxPtr, ParentToolBarPtr);
}


