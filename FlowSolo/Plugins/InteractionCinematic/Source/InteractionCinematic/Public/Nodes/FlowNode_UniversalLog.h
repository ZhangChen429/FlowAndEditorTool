#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "FlowNode_UniversalLog.generated.h"

/**
 * A universal log node that can be used in ANY FlowAsset
 *
 * This node does NOT set AllowedAssetClasses, so it will appear
 * in the palette of both regular FlowAssets and InteractionCinematicFlowAssets.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Log Message (Universal)"))
class INTERACTIONCINEMATIC_API UFlowNode_UniversalLog : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_UniversalLog();

#if WITH_EDITOR
	virtual FString GetNodeCategory() const override { return TEXT("Debug"); }
#endif

protected:
	/** Message to log */
	UPROPERTY(EditAnywhere, Category = "Log")
	FString Message = TEXT("Hello from Flow!");

	virtual void ExecuteInput(const FName& PinName) override;
};
