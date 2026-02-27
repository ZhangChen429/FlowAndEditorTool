// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WaitContinue/WaitContinueTypes.h"
#include "WaitContinue/EventListener.h"
#include "WaitContinue/ExecutionContext.h"

namespace WaitContinueSystem
{
	/**
	 * Condition Interface
	 * Defines the contract for all condition types
	 * Can be used throughout the project for various condition checking needs
	 * NOTE: Header-only interface, no API export needed
	 */
	class ICondition
	{
	public:
		virtual ~ICondition() = default;

		/**
		 * Create an event listener for this condition
		 * @param Context The execution context
		 * @return Shared pointer to the created listener
		 */
		virtual TSharedPtr<FEventListener> CreateEventListener(FExecutionContext& Context) = 0;

		/**
		 * Check if the condition is fulfilled
		 * This checks the listener first, then falls back to immediate check
		 * @param Context The execution context
		 * @return True if the condition is met
		 */
		virtual bool IsFulfilled(FExecutionContext& Context) const = 0;

		/**
		 * Immediately check if the condition is met (without using a listener)
		 * @param Context The execution context
		 * @return True if the condition is met
		 */
		virtual bool CheckImmediately(FExecutionContext& Context) const = 0;

		/**
		 * Get a friendly name for this condition (for debugging)
		 * @return Descriptive name of the condition
		 */
		virtual FString GetFriendlyName() const = 0;
	};

	/**
	 * Base Condition Implementation
	 * Provides common functionality and default implementations for all conditions
	 * Subclasses should override:
	 *   - CreateEventListener() - to provide specific listener types
	 *   - CheckImmediately() - to implement actual condition checking
	 *   - GetFriendlyName() - to provide descriptive names for debugging
	 * NOTE: Header-only class, no API export needed
	 */
	class FBaseCondition : public ICondition
	{
	public:
		explicit FBaseCondition(const FGuid& InNodeId)
			: NodeId(InNodeId)
		{
		}

		virtual ~FBaseCondition() = default;

		// ICondition interface implementation

		/**
		 * Default implementation: Returns a basic event listener
		 * Subclasses should override this to provide specific listener types
		 */
		virtual TSharedPtr<FEventListener> CreateEventListener(FExecutionContext& Context) override
		{
			return MakeShared<FEventListener>();
		}

		/**
		 * Default implementation that checks the listener first
		 * Falls back to immediate check if no listener exists
		 */
		virtual bool IsFulfilled(FExecutionContext& Context) const override
		{
			// 1. Try to get state from listener
			FEventListener* Listener = Context.FindListener(NodeId);
			if (Listener)
			{
				return Listener->IsFulfilled();
			}

			// 2. If no listener, check immediately
			return CheckImmediately(Context);
		}

		/**
		 * Default implementation: Always returns false
		 * Subclasses should override this to provide actual condition checking
		 */
		virtual bool CheckImmediately(FExecutionContext& Context) const override
		{
			return false;
		}

		/**
		 * Default implementation: Returns generic name
		 * Subclasses should override this to provide descriptive names
		 */
		virtual FString GetFriendlyName() const override
		{
			return TEXT("Base Condition");
		}

		// --

		/**
		 * Get the node ID associated with this condition
		 * @return The node GUID
		 */
		const FGuid& GetNodeId() const
		{
			return NodeId;
		}

	protected:
		// Unique identifier for the node using this condition
		FGuid NodeId;
	};

} // namespace WaitContinueSystem
