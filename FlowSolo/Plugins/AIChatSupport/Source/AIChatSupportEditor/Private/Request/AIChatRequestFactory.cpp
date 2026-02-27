// Copyright Epic Games, Inc. All Rights Reserved.

#include "Request/AIChatRequestFactory.h"
#include "AIChatSupportEditor.h"
#include "Request/AIChatRequest.h"

TSharedPtr<FAIChatRequestBase> FAIChatRequestFactory::CreateRequest(const FAIChatSettings& Settings)
{
	// 中转站模式：根据协议类型选择，而不是 Provider
	if (Settings.bUseCustomURL)
	{
		switch (Settings.CustomProtocol)
		{
		case EAPIProtocol::Claude:
			UE_LOG(LogAIChat, Log, TEXT("[AI Chat Factory] Custom URL with Claude protocol"));
			return MakeShareable(new FAIChatRequest_Claude());

		case EAPIProtocol::OpenAI:
		default:
			UE_LOG(LogAIChat, Log, TEXT("[AI Chat Factory] Custom URL with OpenAI-compatible protocol"));
			return MakeShareable(new FAIChatRequest_OpenAI());
		}
	}

	// 标准模式：根据 Provider 选择
	switch (Settings.Provider)
	{
	case EAIProvider::Claude:
		return MakeShareable(new FAIChatRequest_Claude());

	case EAIProvider::DeepSeek:
		return MakeShareable(new FAIChatRequest_Deepseek());

	case EAIProvider::OpenAI:
		return MakeShareable(new FAIChatRequest_OpenAI());

	default:
		UE_LOG(LogAIChat, Error, TEXT("[AI Chat Factory] Unsupported provider: %d"), (int32)Settings.Provider);
		return nullptr;
	}
}

void FAIChatRequestFactory::SendChatRequest(const FAIChatSettings& Settings, const FOnAIChatProcessed& OnProcessed)
{
	TSharedPtr<FAIChatRequestBase> Request = CreateRequest(Settings);

	if (!Request.IsValid())
	{
		FAIChatResponse ErrorResponse;
		ErrorResponse.bSuccess = false;
		ErrorResponse.ErrorType = EAIChatErrorType::InvalidPayload;
		ErrorResponse.ErrorMessage = TEXT("Failed to create request handler for the selected provider");
		OnProcessed.ExecuteIfBound(ErrorResponse);
		return;
	}

	Request->SendRequest(Settings, OnProcessed);
}
