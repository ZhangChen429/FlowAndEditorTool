// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WaitContinue/WaitContinueTypes.h"
#include "HAL/ThreadSafeBool.h"

namespace WaitContinueSystem
{
	// Forward declaration
	class FExecutionContext;

	/**
	 * Event Listener Base Class
	 * Monitors conditions and notifies when they are fulfilled
	 * Thread-safe using atomic operations
	 * NOTE: Header-only class, no API export needed
	 */
	class FEventListener
	{
	public:
		FEventListener()
			: bFulfilled(false)
			, bInitialized(false)
		{
		}

		virtual ~FEventListener() = default;

		/**
		 * Called when the listener is created and registered
		 * @param Context The execution context
		 */
		virtual void OnCreate(FExecutionContext& Context)
		{
			bInitialized = true;
		}

		/**
		 * Called when the listener is unregistered and destroyed
		 * @param Context The execution context
		 */
		virtual void OnDestroy(FExecutionContext& Context)
		{
			bInitialized = false;
		}

		/**
		 * Event trigger callback (called from external sources)
		 * Sets the fulfilled flag to true
		 */
		virtual void OnEventTriggered()
		{
			SetFulfilled(true);
		}

		/**
		 * Check if the condition is fulfilled
		 * @return True if the condition is met
		 */
		bool IsFulfilled() const
		{
			return bFulfilled;
		}

		/**
		 * Set the fulfilled state
		 * @param bInFulfilled The new fulfilled state
		 */
		void SetFulfilled(bool bInFulfilled)
		{
			bFulfilled = bInFulfilled;

			// Trigger callback if fulfilled
			if (bInFulfilled && OnFulfilledCallback)
			{
				OnFulfilledCallback();
			}
		}

		/**
		 * Set the callback to be called when the condition is fulfilled
		 * @param Callback The callback function
		 */
		void SetOnFulfilledCallback(TFunction<void()> Callback)
		{
			OnFulfilledCallback = Callback;
		}

		/**
		 * Check if the listener is initialized
		 * @return True if initialized
		 */
		bool IsInitialized() const
		{
			return bInitialized;
		}

	protected:
		// Thread-safe boolean indicating if the condition is fulfilled
		FThreadSafeBool bFulfilled;

		// Thread-safe boolean indicating if the listener is initialized
		FThreadSafeBool bInitialized;

		// Callback function to be called when fulfilled
		TFunction<void()> OnFulfilledCallback;
	};

} // namespace WaitContinueSystem
