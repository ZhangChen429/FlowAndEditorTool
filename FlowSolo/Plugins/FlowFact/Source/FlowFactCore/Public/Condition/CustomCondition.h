// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Condition/ICondition.h"

namespace WaitContinueSystem
{
	/**
	 * Custom Event Listener
	 * Uses a custom function to check if the condition is met
	 * NOTE: Header-only class, no API export needed
	 */
	class FCustomEventListener : public FEventListener
	{
	public:
		explicit FCustomEventListener(TFunction<bool()> InCheckFunc)
			: CheckFunc(InCheckFunc)
		{
		}

		virtual ~FCustomEventListener() = default;

		/**
		 * Check the custom condition
		 * Should be called every frame/tick
		 */
		void CheckCondition()
		{
			if (!IsFulfilled() && CheckFunc && CheckFunc())
			{
				SetFulfilled(true);
			}
		}

	private:
		TFunction<bool()> CheckFunc;
	};

	/**
	 * Custom Condition
	 * Allows using a custom function/lambda to determine if condition is met
	 * NOTE: Header-only class, no API export needed
	 */
	class FCustomCondition : public FBaseCondition
	{
	public:
		FCustomCondition(const FGuid& InNodeId, TFunction<bool()> InCheckFunc)
			: FBaseCondition(InNodeId)
			, CheckFunc(InCheckFunc)
		{
		}

		virtual ~FCustomCondition() = default;

		// ICondition interface
		virtual TSharedPtr<FEventListener> CreateEventListener(FExecutionContext& Context) override
		{
			return MakeShared<FCustomEventListener>(CheckFunc);
		}

		virtual bool CheckImmediately(FExecutionContext& Context) const override
		{
			return CheckFunc ? CheckFunc() : false;
		}

		virtual FString GetFriendlyName() const override
		{
			return TEXT("Custom Condition");
		}
		// --

	private:
		TFunction<bool()> CheckFunc;
	};

} // namespace WaitContinueSystem
