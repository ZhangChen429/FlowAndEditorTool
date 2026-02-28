// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WorkspotComponent.generated.h"

class UWorkspotTree;

/**
 * WorkspotComponent - Reference holder for workspot resources
 *
 * Architecture (based on Cyberpunk 2077 WorkspotResourceComponent):
 * - Placed on actors or in the world
 * - Holds reference to WorkspotTree assets
 * - Does NOT contain execution logic
 * - Execution is handled by WorkspotSubsystem + WorkspotInstance
 *
 * This is a lightweight component that only stores references.
 * To actually execute a workspot, use:
 *   UWorkspotSubsystem* Subsystem = GetWorld()->GetSubsystem<UWorkspotSubsystem>();
 *   Subsystem->StartWorkspot(Actor, WorkspotComponent->WorkspotTree);
 */
UCLASS(ClassGroup = Workspot, meta = (BlueprintSpawnableComponent))
class WORKSPOT_API UWorkspotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWorkspotComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Workspot")
	FName EntryPointTag;
	
	UFUNCTION(BlueprintCallable, Category = "Workspot")
	bool StartWorkspotWithTree(UWorkspotTree* Tree);
	
	UFUNCTION(BlueprintCallable, Category = "Workspot")
	void StopWorkspot(bool bForceStop = false);
	
	UFUNCTION(BlueprintPure, Category = "Workspot")
	bool IsInWorkspot() const;
	
	UFUNCTION(BlueprintPure, Category = "Workspot")
	class UWorkspotInstance* GetActiveInstance() const;
};
