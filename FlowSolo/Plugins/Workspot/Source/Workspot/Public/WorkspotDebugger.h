// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WorkspotDebugger.generated.h"

enum class EWorkspotState : uint8;
class UWorkspotInstance;

/**
 * Workspot debugging utilities
 *
 * Usage in Blueprint:
 * 1. Call "Enable Workspot Debug Display" on BeginPlay
 * 2. Watch on-screen debug info showing Entry traversal
 */
UCLASS()
class WORKSPOT_API UWorkspotDebugger : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Enable debug display for all workspot instances
	 * Shows Entry type, animation name, and traversal path on screen
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot|Debug", meta = (WorldContext = "WorldContextObject"))
	static void EnableDebugDisplay(UObject* WorldContextObject, bool bEnable = true);

	/**
	 * Check if debug display is enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Workspot|Debug", meta = (WorldContext = "WorldContextObject"))
	static bool IsDebugDisplayEnabled(UObject* WorldContextObject);

	/**
	 * Print workspot tree structure to log
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot|Debug")
	static void PrintTreeStructure(class UWorkspotTree* WorkspotTree, bool bDetailed = false);

	/**
	 * Get debug info string for an active workspot instance
	 */
	UFUNCTION(BlueprintPure, Category = "Workspot|Debug")
	static FString GetInstanceDebugInfo(UWorkspotInstance* Instance);

	/**
	 * Set debug verbosity level (0=Off, 1=Basic, 2=Detailed, 3=VeryVerbose)
	 */
	UFUNCTION(BlueprintCallable, Category = "Workspot|Debug", meta = (WorldContext = "WorldContextObject"))
	static void SetDebugVerbosity(UObject* WorldContextObject, int32 VerbosityLevel);

	/**
	 * Draw debug info for all active workspots in world
	 * Called automatically by WorkspotSubsystem if debug is enabled
	 */
	static void DrawDebugInfo(UWorld* World);

private:
	static bool bDebugDisplayEnabled;
	static int32 DebugVerbosity;

	// Helper functions
	static void PrintEntry(const class UWorkspotEntry* Entry, int32 Depth, bool bDetailed);
	static FString GetStateName(EWorkspotState State);
};
