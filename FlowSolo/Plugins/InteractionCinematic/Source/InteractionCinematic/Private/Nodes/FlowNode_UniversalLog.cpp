#include "Nodes/FlowNode_UniversalLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlowNode_UniversalLog)

UFlowNode_UniversalLog::UFlowNode_UniversalLog()
{
#if WITH_EDITOR
	Category = TEXT("Debug");
#endif

	// ✅ NOTE: We do NOT set AllowedAssetClasses here
	// This means this node can be used in ANY FlowAsset type
	// It will appear in both:
	//   - Regular FlowAsset palettes
	//   - InteractionCinematicFlowAsset palettes

	InputPins = {TEXT("In")};
	OutputPins = {TEXT("Out")};
}

void UFlowNode_UniversalLog::ExecuteInput(const FName& PinName)
{
	LogNote(Message);
	TriggerOutput(TEXT("Out"), true);
}
