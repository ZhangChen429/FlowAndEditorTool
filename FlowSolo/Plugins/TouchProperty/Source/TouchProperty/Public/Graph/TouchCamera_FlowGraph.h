#pragma once
#include "Graph/FlowGraphEditor.h"
#include "TouchAsset/TouchCameraAsset.h"

class FTouchCameraAssetToolkit;

class TOUCHPROPERTY_API STouchCamera_FlowGraph: public SFlowGraphEditor
{
public:
	void Construct(const FArguments& InArgs, const TSharedPtr<FTouchCameraAssetToolkit> InAssetEditor);
private:
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 6
	static FReply TCOnSpawnGraphNodeByShortcut(FInputChord InChord, const FVector2D& InPosition, UEdGraph* InGraph);
#else
	static FReply TCOnSpawnGraphNodeByShortcut(FInputChord InChord, const FVector2f& InPosition, UEdGraph* InGraph);
#endif
protected:
	TWeakObjectPtr<UTouchCameraAsset> TouchCameraObject;
};


