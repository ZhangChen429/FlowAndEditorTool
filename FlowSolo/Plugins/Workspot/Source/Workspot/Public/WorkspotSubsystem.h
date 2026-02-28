// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "WorkspotSubsystem.generated.h"

class UWorkspotTree;
class UWorkspotInstance;
class AActor;

/**
 * Workspot Subsystem - Global manager for all workspot instances
 *
 * Architecture (based on Cyberpunk 2077):
 * - Manages all active workspot instances
 * - Creates and destroys WorkspotInstance objects
 * - Provides API for starting/stopping workspots
 * - Ticks all active instances
 *
 * Usage:
 *   UWorkspotSubsystem* Subsystem = GetWorld()->GetSubsystem<UWorkspotSubsystem>();
 *   Subsystem->StartWorkspot(Actor, WorkspotTree);
 */
UCLASS()
class WORKSPOT_API UWorkspotSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	//////////////////////////////////////////////////////////////////////////
	// Subsystem Interface
	//////////////////////////////////////////////////////////////////////////

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	//////////////////////////////////////////////////////////////////////////
	// Workspot Management API
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Start a workspot on an actor
	 * @param Actor - The actor to use the workspot
	 * @param WorkspotTree - The workspot tree to execute
	 * @param EntryPointTag - Optional entry point tag (empty = auto-select)
	 * @return The created workspot instance (nullptr if failed)
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot")
	UWorkspotInstance* StartWorkspot(AActor* Actor, UWorkspotTree* WorkspotTree, FName EntryPointTag = NAME_None);

	/**
	 * Stop workspot on an actor
	 * @param Actor - The actor to stop workspot on
	 * @param bForceStop - If true, immediately stop without exit animation
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot")
	void StopWorkspot(AActor* Actor, bool bForceStop = false);

	/**
	 * Get active workspot instance for an actor
	 * @param Actor - The actor to query
	 * @return The active workspot instance (nullptr if none)
	 */
	UFUNCTION(BlueprintPure, Category = "Workspot")
	UWorkspotInstance* GetActiveWorkspot(AActor* Actor) const;

	/**
	 * Check if an actor is currently in a workspot
	 */
	UFUNCTION(BlueprintPure, Category = "Workspot")
	bool IsActorInWorkspot(AActor* Actor) const;

	/**
	 * Stop all active workspots
	 * @param bForceStop - If true, immediately stop all without exit animations
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot")
	void StopAllWorkspots(bool bForceStop = false);

	/**
	 * Get all active workspot instances
	 */
	const TMap<TWeakObjectPtr<AActor>, TObjectPtr<UWorkspotInstance>>& GetActiveInstances() const { return ActiveInstances; }

private:
	/** Remove completed instances */
	void CleanupCompletedInstances();

	/** Handle instance completion callback */
	void OnInstanceCompleted(UWorkspotInstance* Instance);

private:
	/** Map of Actor -> WorkspotInstance for all active workspots */
	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<AActor>, TObjectPtr<UWorkspotInstance>> ActiveInstances;

	/** Instances pending removal (to avoid iterator invalidation) */
	TArray<TWeakObjectPtr<AActor>> PendingRemoval;
};
