// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_UseWorkspotSmartObject.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SmartObjectSubsystem.h"
#include "GameplayBehaviorSmartObjectBehaviorDefinition.h"
#include "WorkspotBehaviorConfig.h"
#include "WorkspotHelpers.h"
#include "Workspot.h"

UBTTask_UseWorkspotSmartObject::UBTTask_UseWorkspotSmartObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Use Workspot SmartObject (Diagnostic)";
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_UseWorkspotSmartObject::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UE_LOG(LogWorkspot, Display, TEXT("═══════════════════════════════════════════"));
	UE_LOG(LogWorkspot, Display, TEXT("  BTTask_UseWorkspotSmartObject - START DIAGNOSTIC"));
	UE_LOG(LogWorkspot, Display, TEXT("═══════════════════════════════════════════"));

	// Get AI controller and pawn
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ FAILED: No AIController"));
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ FAILED: AIController has no Pawn"));
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogWorkspot, Display, TEXT("✅ AIController: %s"), *AIController->GetName());
	UE_LOG(LogWorkspot, Display, TEXT("✅ Pawn: %s"), *Pawn->GetName());

	// Get blackboard
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ FAILED: No BlackboardComponent"));
		return EBTNodeResult::Failed;
	}

	// Get ClaimHandle from blackboard
	UObject* ClaimHandleObject = Blackboard->GetValueAsObject(ClaimHandleKey.SelectedKeyName);
	if (!ClaimHandleObject)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ FAILED: ClaimHandle blackboard key '%s' is null"), *ClaimHandleKey.SelectedKeyName.ToString());
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogWorkspot, Display, TEXT("✅ ClaimHandle object found: %s"), *ClaimHandleObject->GetClass()->GetName());

	// Try to extract FSmartObjectClaimHandle
	// Note: This depends on how you store ClaimHandle in blackboard
	// Common approaches:
	// 1. Store as UObject wrapper (need to cast and extract)
	// 2. Store as struct wrapper
	// For now, let's assume it's stored correctly and we can access it

	// Get SmartObject subsystem
	UWorld* World = OwnerComp.GetWorld();
	USmartObjectSubsystem* SmartObjectSubsystem = World->GetSubsystem<USmartObjectSubsystem>();
	if (!SmartObjectSubsystem)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ FAILED: No SmartObjectSubsystem in world"));
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogWorkspot, Display, TEXT("✅ SmartObjectSubsystem found"));

	// ⚠️ CRITICAL: We need the actual FSmartObjectClaimHandle structure
	// The blackboard key type needs to be checked
	// For now, let's try to get it via WorkspotHelpers which already knows how to extract it

	// Alternative approach: Use WorkspotHelpers directly with the claim handle
	// This requires the ClaimHandle to be properly stored in a way we can access it

	UE_LOG(LogWorkspot, Warning, TEXT("⚠️  NOTE: This diagnostic task requires ClaimHandle to be accessible"));
	UE_LOG(LogWorkspot, Warning, TEXT("⚠️  Consider storing ClaimHandle in a custom blackboard value type"));

	// Log current state
	UE_LOG(LogWorkspot, Display, TEXT(""));
	UE_LOG(LogWorkspot, Display, TEXT("DIAGNOSTIC CHECKLIST:"));
	UE_LOG(LogWorkspot, Display, TEXT("  [1] AIController exists: ✅"));
	UE_LOG(LogWorkspot, Display, TEXT("  [2] Pawn exists: ✅"));
	UE_LOG(LogWorkspot, Display, TEXT("  [3] Blackboard exists: ✅"));
	UE_LOG(LogWorkspot, Display, TEXT("  [4] SmartObjectSubsystem exists: ✅"));
	UE_LOG(LogWorkspot, Display, TEXT("  [5] ClaimHandle object found: ✅"));
	UE_LOG(LogWorkspot, Display, TEXT("  [6] ClaimHandle structure access: ❓ (see warning above)"));

	UE_LOG(LogWorkspot, Display, TEXT(""));
	UE_LOG(LogWorkspot, Display, TEXT("NEXT STEPS:"));
	UE_LOG(LogWorkspot, Display, TEXT("  1. Verify SmartObject Definition has BehaviorDefinition set"));
	UE_LOG(LogWorkspot, Display, TEXT("  2. Verify BehaviorDefinition type is 'GameplayBehaviorSmartObjectBehaviorDefinition'"));
	UE_LOG(LogWorkspot, Display, TEXT("  3. Verify Config is instanced and assigned (not null)"));
	UE_LOG(LogWorkspot, Display, TEXT("  4. Verify Config type is 'UWorkspotBehaviorConfig'"));
	UE_LOG(LogWorkspot, Display, TEXT("  5. Verify WorkspotTree is assigned in Config"));

	UE_LOG(LogWorkspot, Display, TEXT("═══════════════════════════════════════════"));

	// For now, return Failed to force you to check configuration
	// Once configuration is verified, we can add actual Use() call here
	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_UseWorkspotSmartObject::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UE_LOG(LogWorkspot, Warning, TEXT("BTTask_UseWorkspotSmartObject - Task aborted"));
	return EBTNodeResult::Aborted;
}

void UBTTask_UseWorkspotSmartObject::LogDiagnostics(UBehaviorTreeComponent& OwnerComp, const FSmartObjectClaimHandle& ClaimHandle)
{
	UWorld* World = OwnerComp.GetWorld();
	USmartObjectSubsystem* SmartObjectSubsystem = World->GetSubsystem<USmartObjectSubsystem>();
	if (!SmartObjectSubsystem)
	{
		return;
	}

	// Get behavior definition
	const UGameplayBehaviorSmartObjectBehaviorDefinition* BehaviorDef =
		SmartObjectSubsystem->GetBehaviorDefinition<UGameplayBehaviorSmartObjectBehaviorDefinition>(ClaimHandle);

	if (!BehaviorDef)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ BehaviorDefinition is NULL"));
		UE_LOG(LogWorkspot, Error, TEXT("   → Check SmartObject Definition asset"));
		UE_LOG(LogWorkspot, Error, TEXT("   → Verify BehaviorDefinition is set on the slot"));
		return;
	}

	UE_LOG(LogWorkspot, Display, TEXT("✅ BehaviorDefinition found: %s"), *BehaviorDef->GetClass()->GetName());

	// Check config
	if (!BehaviorDef->GameplayBehaviorConfig)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ GameplayBehaviorConfig is NULL"));
		UE_LOG(LogWorkspot, Error, TEXT("   → Open BehaviorDefinition and assign Config"));
		UE_LOG(LogWorkspot, Error, TEXT("   → Config must be INSTANCED (not asset reference)"));
		return;
	}

	UE_LOG(LogWorkspot, Display, TEXT("✅ GameplayBehaviorConfig found: %s"), *BehaviorDef->GameplayBehaviorConfig->GetClass()->GetName());

	// Check if it's WorkspotBehaviorConfig
	const UWorkspotBehaviorConfig* WorkspotConfig = Cast<UWorkspotBehaviorConfig>(BehaviorDef->GameplayBehaviorConfig);
	if (!WorkspotConfig)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ Config is not UWorkspotBehaviorConfig"));
		UE_LOG(LogWorkspot, Error, TEXT("   → Config type: %s"), *BehaviorDef->GameplayBehaviorConfig->GetClass()->GetName());
		return;
	}

	UE_LOG(LogWorkspot, Display, TEXT("✅ Config is UWorkspotBehaviorConfig"));

	// Check WorkspotTree
	if (!WorkspotConfig->WorkspotTree)
	{
		UE_LOG(LogWorkspot, Error, TEXT("❌ WorkspotTree is NULL in config"));
		UE_LOG(LogWorkspot, Error, TEXT("   → Open Config and assign WorkspotTree"));
		return;
	}

	//UE_LOG(LogWorkspot, Display, TEXT("✅ WorkspotTree assigned: %s"), *WorkspotConfig->WorkspotTree->GetName());

	// Check BehaviorClass
	//if (!WorkspotConfig->BehaviorClass)
	//{
	//	UE_LOG(LogWorkspot, Error, TEXT("❌ BehaviorClass is NULL in config"));
	//	UE_LOG(LogWorkspot, Error, TEXT("   → This should be set automatically in constructor"));
	//	return;
	//}

	//UE_LOG(LogWorkspot, Display, TEXT("✅ BehaviorClass set: %s"), *WorkspotConfig->BehaviorClass->GetName());

	UE_LOG(LogWorkspot, Display, TEXT(""));
	UE_LOG(LogWorkspot, Display, TEXT("✅ ALL CHECKS PASSED - Configuration is correct"));
	UE_LOG(LogWorkspot, Display, TEXT("   → Problem may be in Use() call or behavior execution"));
}
