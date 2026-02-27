// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WaitContinue/WaitContinueTypes.h"
#include "HAL/CriticalSection.h"

namespace WaitContinueSystem
{
	/**
	 * Execution Context - Manages event listeners
	 * Thread-safe container for all active listeners
	 */
	class FLOWFACTCORE_API FExecutionContext
	{
	public:
		FExecutionContext() = default;
		~FExecutionContext() = default;

		/**
		 * Register an event listener for a specific node
		 * @param NodeId Unique identifier for the node
		 * @param Listener The event listener to register
		 */
		void RegisterListener(const FGuid& NodeId, TSharedPtr<FEventListener> Listener);

		/**
		 * Unregister an event listener for a specific node
		 * @param NodeId Unique identifier for the node
		 */
		void UnregisterListener(const FGuid& NodeId);

		/**
		 * Find an event listener by node ID
		 * @param NodeId Unique identifier for the node
		 * @return Pointer to the listener, or nullptr if not found
		 */
		FEventListener* FindListener(const FGuid& NodeId) const;

		/**
		 * Notify an event (trigger from external sources)
		 * @param NodeId Unique identifier for the node
		 */
		void NotifyEvent(const FGuid& NodeId);

		/**
		 * Clear all registered listeners
		 */
		void ClearAllListeners();

	private:
		// Thread-safe protection for the listener map
		mutable FCriticalSection CriticalSection;

		// Map of NodeId to EventListener
		TMap<FGuid, TSharedPtr<FEventListener>> Listeners;
	};

} // namespace WaitContinueSystem
