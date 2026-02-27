// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"

namespace WaitContinueSystem
{
	/**
	 * Node execution result
	 * Determines whether the flow should continue or wait
	 */
	enum class ENodeResult : uint8
	{
		LeaveNode,		// Leave the node and continue execution
		StayInNode		// Stay in the node and wait for condition
	};

	/**
	 * Condition state (three-valued logic)
	 */
	enum class EConditionState : int8
	{
		Unknown = -1,	// Unknown state
		False = 0,		// Condition not met
		True = 1		// Condition met
	};

	// Forward declarations
	class FExecutionContext;
	class FEventListener;
	class ICondition;

} // namespace WaitContinueSystem
