// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIChatRequestFactory.h"
#include "Data/AIChatStructs.h"
#include "Interfaces/IHttpRequest.h"

class IHttpRequest;
// 响应回调委托
DECLARE_DELEGATE_ThreeParams(FOnAIChatResponse, const FString& /*Response*/, const FString& /*Error*/, bool /*bSuccess*/);

/**
 * AI Chat 请求类
 * 负责向远端 AI API 发送消息并接收响应
 */

UENUM()
enum class EAIChatErrorType : uint8
{
	None,
	EmptyAPIKey,
	InvalidPayload,
	HttpRequestFailed,
	JsonParseFailed,
	InvalidResponseFormat,
	APIError
};

// 统一的响应结构体（替代零散的参数）
struct FAIChatResponse
{
	FString Content;       // AI回复内容
	FString ErrorMessage;  // 错误信息
	EAIChatErrorType ErrorType = EAIChatErrorType::None;
	bool bSuccess = false; // 是否成功
};

// FOnAIChatProcessed is declared in AIChatRequestFactory.h

/**
 * AI Chat 请求基类
 * 使用 TSharedFromThis 支持安全的异步回调
 */
class AICHATSUPPORTEDITOR_API FAIChatRequestBase : public TSharedFromThis<FAIChatRequestBase>
{
public:
	/**
	 * 发送聊天请求
	 * @param Settings - 聊天设置
	 * @param OnResponse - 响应回调
	 */
	//static void SendChatRequest(const FAIChatSettings& Settings, const FOnAIChatResponse& OnResponse);
	
	void SendRequest(const FAIChatSettings& Settings, const FOnAIChatProcessed& OnProcessed);
	friend FAIChatRequestFactory;

	static FAIChatResponse MakeErrorResponse(EAIChatErrorType ErrorType, const FString& ErrorMsg);
	virtual ~FAIChatRequestBase() {}
protected:
	
	virtual FString BuildPayload(const FAIChatSettings& Settings) const;
	virtual FAIChatResponse ParseResponse(const FString& ResponseStr) const;
	virtual void SetupHttpRequest(const FAIChatSettings& Settings, TSharedRef<IHttpRequest> HttpRequest) const;


private:
	/**
	 * 构建 JSON 请求体
	 * @param Settings - 聊天设置
	 * @return JSON 字符串
	 */
	static FString BuildRequestPayload(const FAIChatSettings& Settings);
	
	void OnHttpRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, const FOnAIChatProcessed& OnProcessed);

	/**
	 * 处理 API 响应
	 * @param ResponseStr - 响应 JSON 字符串
	 * @param OnResponse - 响应回调
	 */
	static void ProcessResponse(const FString& ResponseStr, const FOnAIChatResponse& OnResponse);
};


class FAIChatRequest_OpenAI : public FAIChatRequestBase
{
protected:
	virtual FString BuildPayload(const FAIChatSettings& Settings) const override;
	virtual FAIChatResponse ParseResponse(const FString& ResponseStr) const override;
	virtual void SetupHttpRequest(const FAIChatSettings& Settings, TSharedRef<IHttpRequest> HttpRequest) const override;
};

// Claude 实现
class FAIChatRequest_Claude : public FAIChatRequestBase
{
protected:
	virtual FString BuildPayload(const FAIChatSettings& Settings) const override;
	virtual FAIChatResponse ParseResponse(const FString& ResponseStr) const override;
	virtual void SetupHttpRequest(const FAIChatSettings& Settings, TSharedRef<IHttpRequest> HttpRequest) const override;
};

class FAIChatRequest_Deepseek : public FAIChatRequestBase
{
protected:
	virtual FString BuildPayload(const FAIChatSettings& Settings) const override;
	virtual FAIChatResponse ParseResponse(const FString& ResponseStr) const override;
	virtual void SetupHttpRequest(const FAIChatSettings& Settings, TSharedRef<IHttpRequest> HttpRequest) const override;
};
