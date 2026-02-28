// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SmartObjectSubsystem.h"
#include "BTTask_MoveToAndUseWorkspot.generated.h"

class UWorkspotInstance;

/**
 * BTTask: Move to SmartObject and use Workspot
 *
 * This is the CORRECT task to use for Workspot + SmartObject integration.
 * DO NOT use MoveToAndUseSmartObjectWithGameplayBehavior - that's for generic GameplayBehavior.
 *
 * Workflow:
 * 1. Find SmartObject by tag/query
 * 2. Claim the slot
 * 3. Move to the slot location
 * 4. Start Workspot from the slot's WorkspotBehaviorConfig
 * 5. Wait for Workspot to complete
 * 6. Release the slot
 */
UCLASS()
class WORKSPOT_API UBTTask_MoveToAndUseWorkspot : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MoveToAndUseWorkspot(const FObjectInitializer& ObjectInitializer);

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	/** SmartObject to use (can be set via blackboard or by tag) */
	UPROPERTY(EditAnywhere, Category = "SmartObject")
	FBlackboardKeySelector SmartObjectActorKey;

	/** If set, will search for SmartObject with this tag */
	UPROPERTY(EditAnywhere, Category = "SmartObject")
	FGameplayTag SmartObjectTag;

	/** Search radius for SmartObject (if using tag) */
	UPROPERTY(EditAnywhere, Category = "SmartObject", meta = (EditCondition = "!SmartObjectTag.IsValid()"))
	float SearchRadius = 1000.0f;

	/** Acceptable radius to slot location */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float AcceptableRadius = 50.0f;

	/** If true, will stop AI movement while using workspot */
	UPROPERTY(EditAnywhere, Category = "Movement")
	bool bStopMovementOnUse = true;

	/** If true, will release slot when workspot completes */
	UPROPERTY(EditAnywhere, Category = "SmartObject")
	bool bReleaseSlotOnComplete = true;

private:
	// Internal state tracking
	struct FTaskMemory
	{
		FSmartObjectClaimHandle ClaimHandle;
		TWeakObjectPtr<UWorkspotInstance> WorkspotInstance;
		bool bMovementComplete = false;
		bool bWorkspotStarted = false;
	};

	bool FindAndClaimSmartObject(UBehaviorTreeComponent& OwnerComp, FTaskMemory* Memory);
	bool MoveToSlotLocation(UBehaviorTreeComponent& OwnerComp, FTaskMemory* Memory);
	bool StartWorkspotFromSlot(UBehaviorTreeComponent& OwnerComp, FTaskMemory* Memory);
	void ReleaseSlot(UBehaviorTreeComponent& OwnerComp, FTaskMemory* Memory);

	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FTaskMemory); }
};
