// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Condition/ICondition.h"

namespace WaitContinueSystem
{
	/**
	 * Time Event Listener
	 * Monitors time elapsed and triggers when delay is reached
	 * NOTE: Header-only class, no API export needed
	 */
	class FTimeEventListener : public FEventListener
	{
	public:
		explicit FTimeEventListener(float InDelaySeconds)
			: DelaySeconds(InDelaySeconds)
			, StartTime(FPlatformTime::Seconds())
		{
		}

		virtual ~FTimeEventListener() = default;

		/**
		 * Update the listener - checks if enough time has elapsed
		 * Should be called every frame/tick
		 */
		void Update()
		{
			if (!IsFulfilled())
			{
				const double CurrentTime = FPlatformTime::Seconds();
				const double Elapsed = CurrentTime - StartTime;

				if (Elapsed >= DelaySeconds)
				{
					SetFulfilled(true);
				}
			}
		}

		/**
		 * Get the remaining time in seconds
		 * @return Remaining time, or 0 if already fulfilled
		 */
		float GetRemainingTime() const
		{
			if (IsFulfilled())
			{
				return 0.0f;
			}

			const double CurrentTime = FPlatformTime::Seconds();
			const double Elapsed = CurrentTime - StartTime;
			const float Remaining = DelaySeconds - static_cast<float>(Elapsed);
			return FMath::Max(0.0f, Remaining);
		}

	private:
		float DelaySeconds;
		double StartTime;
	};

	/**
	 * Time Condition
	 * Waits for a specified amount of time before being fulfilled
	 * NOTE: Header-only class, no API export needed
	 */
	class FTimeCondition : public FBaseCondition
	{
	public:
		FTimeCondition(const FGuid& InNodeId, float InDelaySeconds)
			: FBaseCondition(InNodeId)
			, DelaySeconds(InDelaySeconds)
		{
		}

		virtual ~FTimeCondition() = default;

		// ICondition interface
		virtual TSharedPtr<FEventListener> CreateEventListener(FExecutionContext& Context) override
		{
			return MakeShared<FTimeEventListener>(DelaySeconds);
		}

		virtual bool CheckImmediately(FExecutionContext& Context) const override
		{
			// Time conditions cannot be checked immediately
			return false;
		}

		virtual FString GetFriendlyName() const override
		{
			return FString::Printf(TEXT("Time Delay: %.2f seconds"), DelaySeconds);
		}
		// --

		/**
		 * Get the delay time in seconds
		 * @return The delay time
		 */
		float GetDelaySeconds() const
		{
			return DelaySeconds;
		}

	private:
		float DelaySeconds;
	};

} // namespace WaitContinueSystem
