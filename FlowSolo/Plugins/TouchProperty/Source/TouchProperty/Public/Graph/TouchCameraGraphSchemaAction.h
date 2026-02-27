#pragma once
#include "Node/TouchCameraBaseNobe.h"

class UTouchCameraBaseNobe;


struct   FTouchCameraGraphSchemaAction_NewNode : public FEdGraphSchemaAction
{

public:
	UPROPERTY()
	class UClass* NodeClass;

	static FName StaticGetTypeId()
	{
		static FName Type("FTouchCameraGraphSchemaAction_NewNode");
		return Type;
	}
	virtual FName GetTypeId() const override { return StaticGetTypeId(); }

	FTouchCameraGraphSchemaAction_NewNode()
		: FEdGraphSchemaAction()
		, NodeClass(nullptr) {}

	explicit FTouchCameraGraphSchemaAction_NewNode(UClass* Node)
		: FEdGraphSchemaAction()
		, NodeClass(Node) {}

	explicit FTouchCameraGraphSchemaAction_NewNode(const UTouchCameraBaseNobe* InNodeTemplate)
		: FEdGraphSchemaAction(FText::FromString(InNodeTemplate->GetNodeCategory()), InNodeTemplate->GetNodeTitle(), InNodeTemplate->GetNodeToolTip(), 0, FText::GetEmpty())
		, NodeClass(InNodeTemplate->GetClass()) {}

	static UTouchCameraBaseNobe* CreateNode(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UClass* NodeClass, const FVector2D Location, const bool bSelectNewNode = true);

};

struct  FTouchCameraGraphSchemaAction_NewComment : public FEdGraphSchemaAction
{

	static FName StaticGetTypeId()
	{
		static FName Type("FTouchCameraGraphSchemaAction_NewComment");
		return Type;
	}

	virtual FName GetTypeId() const override { return StaticGetTypeId(); }

	FTouchCameraGraphSchemaAction_NewComment()
		: FEdGraphSchemaAction() {}

	FTouchCameraGraphSchemaAction_NewComment(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping) {}

	// FEdGraphSchemaAction
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override ;
	// --
};