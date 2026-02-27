// Fill out your copyright notice in the Description page of Project Settings.

#include "WaitContinue/ExecutionContext.h"
#include "WaitContinue/EventListener.h"

namespace WaitContinueSystem
{
	void FExecutionContext::RegisterListener(const FGuid& NodeId, TSharedPtr<FEventListener> Listener)
	{
		FScopeLock Lock(&CriticalSection);
		Listeners.Add(NodeId, Listener);
	}

	void FExecutionContext::UnregisterListener(const FGuid& NodeId)
	{
		FScopeLock Lock(&CriticalSection);
		Listeners.Remove(NodeId);
	}

	FEventListener* FExecutionContext::FindListener(const FGuid& NodeId) const
	{
		FScopeLock Lock(&CriticalSection);

		const TSharedPtr<FEventListener>* FoundListener = Listeners.Find(NodeId);
		if (FoundListener && FoundListener->IsValid())
		{
			return FoundListener->Get();
		}

		return nullptr;
	}

	void FExecutionContext::NotifyEvent(const FGuid& NodeId)
	{
		FScopeLock Lock(&CriticalSection);

		TSharedPtr<FEventListener>* FoundListener = Listeners.Find(NodeId);
		if (FoundListener && FoundListener->IsValid())
		{
			(*FoundListener)->OnEventTriggered();
		}
	}

	void FExecutionContext::ClearAllListeners()
	{
		FScopeLock Lock(&CriticalSection);
		Listeners.Empty();
	}

} // namespace WaitContinueSystem
