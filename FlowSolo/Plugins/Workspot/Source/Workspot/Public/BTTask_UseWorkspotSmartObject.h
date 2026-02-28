// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SmartObjectSubsystem.h"
#include "BTTask_UseWorkspotSmartObject.generated.h"

/**
 * Diagnostic BTTask for Workspot SmartObject integration
 * Use this instead of MoveToAndUseSmartObjectWithGameplayBehavior to debug why Trigger() isn't called
 *
 * This task will:
 * 1. Verify ClaimHandle validity
 * 2. Check BehaviorDefinition configuration
 * 3. Manually trigger Use() if needed
 * 4. Log detailed failure reasons
 */
UCLASS()
class WORKSPOT_API UBTTask_UseWorkspotSmartObject : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_UseWorkspotSmartObject(const FObjectInitializer& ObjectInitializer);

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	/** Blackboard key storing the SmartObjectClaimHandle */
	UPROPERTY(EditAnywhere, Category = "SmartObject")
	FBlackboardKeySelector ClaimHandleKey;

	/** If true, will attempt to manually call Use() if automatic behavior fails */
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bManuallyTriggerUse = true;

	/** If true, will use WorkspotHelpers::StartWorkspotFromClaimHandle as fallback */
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bUseFallbackStart = true;

private:
	void LogDiagnostics(UBehaviorTreeComponent& OwnerComp, const FSmartObjectClaimHandle& ClaimHandle);
};
