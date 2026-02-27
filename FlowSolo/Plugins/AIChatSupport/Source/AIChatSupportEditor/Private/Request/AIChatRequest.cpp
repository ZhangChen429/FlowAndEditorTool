// Copyright Epic Games, Inc. All Rights Reserved.

#include "Request/AIChatRequest.h"
#include "AIChatSupportEditor.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"


void FAIChatRequestBase::SendRequest(const FAIChatSettings& Settings, const FOnAIChatProcessed& OnProcessed)
{
	// 1. 统一验证API Key
	if (Settings.APIKey.IsEmpty())
	{
		UE_LOG(LogAIChat, Error, TEXT("[AI Chat] API Key is empty"));
		OnProcessed.ExecuteIfBound(MakeErrorResponse(EAIChatErrorType::EmptyAPIKey, TEXT("API Key is empty")));
		return;
	}

	// 2. 构建Payload（调用子类实现）
	FString Payload = BuildPayload(Settings);
	if (Payload.IsEmpty())
	{
		UE_LOG(LogAIChat, Error, TEXT("[AI Chat] Failed to build request payload"));
		OnProcessed.ExecuteIfBound(MakeErrorResponse(EAIChatErrorType::InvalidPayload, TEXT("Failed to build request payload")));
		return;
	}

	// 3. 创建HTTP请求（通用逻辑）
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetTimeout(180.0f);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetURL(Settings.GetAPIUrl());
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// 4. 设置HTTP Header（调用子类实现）
	SetupHttpRequest(Settings, HttpRequest);

	// 5. 设置请求体
	HttpRequest->SetContentAsString(Payload);

	UE_LOG(LogAIChat, Log, TEXT("[AI Chat] Sending request to %s"), *Settings.GetAPIUrl());
	UE_LOG(LogAIChat, Verbose, TEXT("[AI Chat] Payload: %s"), *Payload);

	// 6. 绑定回调（捕获 TSharedRef 保证对象生命周期）
	// AsShared() 获取 TSharedRef<FAIChatRequestBase>，确保对象在异步回调期间保持存活
	TSharedRef<FAIChatRequestBase> SharedThis = AsShared();
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[SharedThis, OnProcessed](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			// SharedThis 保证了对象不会在回调前被销毁
			SharedThis->OnHttpRequestCompleted(Request, Response, bSuccess, OnProcessed);
		});

	// 7. 发送请求
	HttpRequest->ProcessRequest();
	
}

FString FAIChatRequestBase::BuildPayload(const FAIChatSettings& Settings) const
{

	TSharedPtr<FJsonObject> JsonPayload = MakeShareable(new FJsonObject());
	JsonPayload->SetStringField(TEXT("model"), Settings.GetModelName());
	JsonPayload->SetNumberField(TEXT("max_tokens"), Settings.MaxTokens);
	JsonPayload->SetNumberField(TEXT("temperature"), Settings.Temperature);
	JsonPayload->SetBoolField(TEXT("stream"), Settings.bStreamResponse);

	// 构建消息数组（空指针安全检查）
	TArray<TSharedPtr<FJsonValue>> MessagesArray;
	for (const FAIChatMessage& Msg : Settings.Messages)
	{
		if (Msg.Role.IsEmpty() || Msg.Content.IsEmpty()) continue; // 跳过无效消息
		
		TSharedPtr<FJsonObject> JsonMsg = MakeShareable(new FJsonObject());
		JsonMsg->SetStringField(TEXT("role"), Msg.Role);
		JsonMsg->SetStringField(TEXT("content"), Msg.Content);
		MessagesArray.Add(MakeShareable(new FJsonValueObject(JsonMsg)));
	}
	JsonPayload->SetArrayField(TEXT("messages"), MessagesArray);

	// 序列化（错误处理）
	FString PayloadStr;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadStr);
	if (!FJsonSerializer::Serialize(JsonPayload.ToSharedRef(), Writer))
	{
		return TEXT("");
	}
	return PayloadStr;
}

FAIChatResponse FAIChatRequestBase::ParseResponse(const FString& ResponseStr) const
{
	// 1. 解析JSON（空指针安全检查）
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);
	if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
	{
		UE_LOG(LogAIChat, Error, TEXT("[AI Chat] OpenAI: Failed to parse JSON"));
		return MakeErrorResponse(EAIChatErrorType::JsonParseFailed, TEXT("Failed to parse JSON response"));
	}

	// 2. 检查API错误
	if (JsonObj->HasField(TEXT("error")))
	{
		TSharedPtr<FJsonObject> ErrorObj = JsonObj->GetObjectField(TEXT("error"));
		FString ErrorMsg = ErrorObj.IsValid() ? ErrorObj->GetStringField(TEXT("message")) : TEXT("Unknown API error");
		UE_LOG(LogAIChat, Error, TEXT("[AI Chat] OpenAI API error: %s"), *ErrorMsg);
		return MakeErrorResponse(EAIChatErrorType::APIError, ErrorMsg);
	}

	// 3. 解析正常响应（数组安全访问）
	if (!JsonObj->HasField(TEXT("choices")))
	{
		return MakeErrorResponse(EAIChatErrorType::InvalidResponseFormat, TEXT("No 'choices' field in response"));
	}

	const TArray<TSharedPtr<FJsonValue>>& Choices = JsonObj->GetArrayField(TEXT("choices"));
	if (Choices.Num() == 0 || !Choices[0].IsValid())
	{
		return MakeErrorResponse(EAIChatErrorType::InvalidResponseFormat, TEXT("Empty 'choices' array"));
	}

	TSharedPtr<FJsonObject> ChoiceObj = Choices[0]->AsObject();
	if (!ChoiceObj.IsValid() || !ChoiceObj->HasField(TEXT("message")))
	{
		return MakeErrorResponse(EAIChatErrorType::InvalidResponseFormat, TEXT("No 'message' field in choice"));
	}

	TSharedPtr<FJsonObject> MsgObj = ChoiceObj->GetObjectField(TEXT("message"));
	if (!MsgObj.IsValid())
	{
		return MakeErrorResponse(EAIChatErrorType::InvalidResponseFormat, TEXT("Invalid 'message' object"));
	}

	// 4. 组装成功响应
	FAIChatResponse Res;
	Res.bSuccess = true;
	Res.Content = MsgObj->GetStringField(TEXT("content"));

	// 处理DeepSeek Reasoning内容
	if (MsgObj->HasField(TEXT("reasoning_content")))
	{
		Res.Content += TEXT("\n\n[Reasoning]\n") + MsgObj->GetStringField(TEXT("reasoning_content"));
	}

	return Res;
}

void FAIChatRequestBase::SetupHttpRequest(const FAIChatSettings& Settings, TSharedRef<IHttpRequest> HttpRequest) const
{
	HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Settings.APIKey));
}

FAIChatResponse FAIChatRequestBase::MakeErrorResponse(EAIChatErrorType ErrorType, const FString& ErrorMsg)
{
	FAIChatResponse Res;
	Res.ErrorType = ErrorType;
	Res.ErrorMessage = ErrorMsg;
	Res.bSuccess = false;
	return Res;
}

FString FAIChatRequestBase::BuildRequestPayload(const FAIChatSettings& Settings)
{
	// 创建 JSON 对象
	TSharedPtr<FJsonObject> JsonPayload = MakeShareable(new FJsonObject());

	// 设置模型
	JsonPayload->SetStringField(TEXT("model"), Settings.GetModelName());

	if (Settings.Provider == EAIProvider::Claude)
	{
		// Claude API 格式
		JsonPayload->SetNumberField(TEXT("max_tokens"), Settings.MaxTokens);
		JsonPayload->SetNumberField(TEXT("temperature"), Settings.Temperature);

		// 提取 system 消息
		FString SystemMessage;
		TArray<TSharedPtr<FJsonValue>> MessagesArray;

		for (const FAIChatMessage& Message : Settings.Messages)
		{
			if (Message.Role == TEXT("system"))
			{
				// Claude 将 system 消息作为单独字段
				SystemMessage = Message.Content;
			}
			else if (Message.Role == TEXT("user") || Message.Role == TEXT("assistant"))
			{
				TSharedPtr<FJsonObject> JsonMessage = MakeShareable(new FJsonObject());
				JsonMessage->SetStringField(TEXT("role"), Message.Role);
				JsonMessage->SetStringField(TEXT("content"), Message.Content);
				MessagesArray.Add(MakeShareable(new FJsonValueObject(JsonMessage)));
			}
		}

		// 设置 system（如果有）
		if (!SystemMessage.IsEmpty())
		{
			JsonPayload->SetStringField(TEXT("system"), SystemMessage);
		}

		JsonPayload->SetArrayField(TEXT("messages"), MessagesArray);
	}
	else
	{
		// OpenAI / DeepSeek 格式
		JsonPayload->SetNumberField(TEXT("max_tokens"), Settings.MaxTokens);
		JsonPayload->SetNumberField(TEXT("temperature"), Settings.Temperature);
		JsonPayload->SetBoolField(TEXT("stream"), Settings.bStreamResponse);

		// 构建消息数组
		TArray<TSharedPtr<FJsonValue>> MessagesArray;
		for (const FAIChatMessage& Message : Settings.Messages)
		{
			TSharedPtr<FJsonObject> JsonMessage = MakeShareable(new FJsonObject());
			JsonMessage->SetStringField(TEXT("role"), Message.Role);
			JsonMessage->SetStringField(TEXT("content"), Message.Content);
			MessagesArray.Add(MakeShareable(new FJsonValueObject(JsonMessage)));
		}
		JsonPayload->SetArrayField(TEXT("messages"), MessagesArray);
	}

	// 序列化为 JSON 字符串
	FString PayloadString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadString);
	if (FJsonSerializer::Serialize(JsonPayload.ToSharedRef(), Writer))
	{
		return PayloadString;
	}

	return TEXT("");
}

void FAIChatRequestBase::OnHttpRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess,
	const FOnAIChatProcessed& OnProcessed)
{
	// 1. HTTP请求失败处理
	if (!bSuccess || !Response.IsValid())
	{
		FString ErrorMsg = Response.IsValid() ? Response->GetContentAsString() : TEXT("No response received");
		int32 ResponseCode = Response.IsValid() ? Response->GetResponseCode() : -1;
		
		if (ResponseCode == 0) ErrorMsg = TEXT("Request timed out");
		
		UE_LOG(LogAIChat, Error, TEXT("[AI Chat] HTTP failed: Code=%d, Error=%s"), ResponseCode, *ErrorMsg);
		OnProcessed.ExecuteIfBound(MakeErrorResponse(EAIChatErrorType::HttpRequestFailed, ErrorMsg));
		return;
	}

	// 2. 打印原始响应（方便调试）
	FString RawResponse = Response->GetContentAsString();
	int32 ResponseCode = Response->GetResponseCode();
	UE_LOG(LogAIChat, Log, TEXT("[AI Chat] HTTP Code: %d"), ResponseCode);
	UE_LOG(LogAIChat, Log, TEXT("[AI Chat] Raw Response: %s"), *RawResponse);

	// 3. 解析响应（调用子类实现）
	FAIChatResponse ResponseData = ParseResponse(RawResponse);
	OnProcessed.ExecuteIfBound(ResponseData);
}

void FAIChatRequestBase::ProcessResponse(const FString& ResponseStr, const FOnAIChatResponse& OnResponse)
{
	UE_LOG(LogAIChat, Verbose, TEXT("AI Chat: Response: %s"), *ResponseStr);

	// 解析 JSON 响应
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogAIChat, Error, TEXT("AI Chat: Failed to parse JSON response"));
		OnResponse.ExecuteIfBound(TEXT(""), TEXT("Failed to parse JSON response"), false);
		return;
	}

	// 检查是否有错误
	if (JsonObject->HasField(TEXT("error")))
	{
		TSharedPtr<FJsonObject> ErrorObject = JsonObject->GetObjectField(TEXT("error"));
		FString ErrorMessage = ErrorObject->GetStringField(TEXT("message"));
		UE_LOG(LogAIChat, Error, TEXT("AI Chat: API error: %s"), *ErrorMessage);
		OnResponse.ExecuteIfBound(TEXT(""), ErrorMessage, false);
		return;
	}

	// Claude API 响应格式
	if (JsonObject->HasField(TEXT("content")))
	{
		const TArray<TSharedPtr<FJsonValue>>& ContentArray = JsonObject->GetArrayField(TEXT("content"));
		if (ContentArray.Num() > 0 && ContentArray[0].IsValid())
		{
			TSharedPtr<FJsonObject> ContentObject = ContentArray[0]->AsObject();
			if (ContentObject.IsValid() && ContentObject->HasField(TEXT("text")))
			{
				FString Content = ContentObject->GetStringField(TEXT("text"));
				UE_LOG(LogAIChat, Log, TEXT("AI Chat: Claude response received successfully"));
				OnResponse.ExecuteIfBound(Content, TEXT(""), true);
				return;
			}
		}
	}

	// OpenAI / DeepSeek 响应格式
	if (JsonObject->HasField(TEXT("choices")))
	{
		const TArray<TSharedPtr<FJsonValue>>& Choices = JsonObject->GetArrayField(TEXT("choices"));
		if (Choices.Num() > 0 && Choices[0].IsValid())
		{
			TSharedPtr<FJsonObject> ChoiceObject = Choices[0]->AsObject();
			if (ChoiceObject.IsValid() && ChoiceObject->HasField(TEXT("message")))
			{
				TSharedPtr<FJsonObject> MessageObject = ChoiceObject->GetObjectField(TEXT("message"));
				FString Content = MessageObject->GetStringField(TEXT("content"));

				// DeepSeek Reasoner 可能包含推理内容
				if (MessageObject->HasField(TEXT("reasoning_content")))
				{
					FString ReasoningContent = MessageObject->GetStringField(TEXT("reasoning_content"));
					Content += TEXT("\n\n[Reasoning]\n") + ReasoningContent;
				}

				UE_LOG(LogAIChat, Log, TEXT("AI Chat: Response received successfully"));
				OnResponse.ExecuteIfBound(Content, TEXT(""), true);
				return;
			}
		}
	}

	UE_LOG(LogAIChat, Error, TEXT("AI Chat: Invalid response format"));
	OnResponse.ExecuteIfBound(TEXT(""), TEXT("Invalid response format"), false);
}

FString FAIChatRequest_OpenAI::BuildPayload(const FAIChatSettings& Settings) const
{
	TSharedPtr<FJsonObject> JsonPayload = MakeShareable(new FJsonObject());
	JsonPayload->SetStringField(TEXT("model"), Settings.GetModelName());
	JsonPayload->SetNumberField(TEXT("max_tokens"), Settings.MaxTokens);
	JsonPayload->SetNumberField(TEXT("temperature"), Settings.Temperature);
	JsonPayload->SetBoolField(TEXT("stream"), Settings.bStreamResponse);

	// 构建消息数组（空指针安全检查）
	TArray<TSharedPtr<FJsonValue>> MessagesArray;
	for (const FAIChatMessage& Msg : Settings.Messages)
	{
		if (Msg.Role.IsEmpty() || Msg.Content.IsEmpty()) continue; // 跳过无效消息
		
		TSharedPtr<FJsonObject> JsonMsg = MakeShareable(new FJsonObject());
		JsonMsg->SetStringField(TEXT("role"), Msg.Role);
		JsonMsg->SetStringField(TEXT("content"), Msg.Content);
		MessagesArray.Add(MakeShareable(new FJsonValueObject(JsonMsg)));
	}
	JsonPayload->SetArrayField(TEXT("messages"), MessagesArray);

	// 序列化（错误处理）
	FString PayloadStr;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadStr);
	if (!FJsonSerializer::Serialize(JsonPayload.ToSharedRef(), Writer))
	{
		return TEXT("");
	}
	return PayloadStr;
}

FAIChatResponse FAIChatRequest_OpenAI::ParseResponse(const FString& ResponseStr) const
{
	// 1. 解析JSON（空指针安全检查）
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);
	if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
	{
		UE_LOG(LogAIChat, Error, TEXT("[AI Chat] OpenAI: Failed to parse JSON"));
		return MakeErrorResponse(EAIChatErrorType::JsonParseFailed, TEXT("Failed to parse JSON response"));
	}

	// 2. 检查API错误
	if (JsonObj->HasField(TEXT("error")))
	{
		TSharedPtr<FJsonObject> ErrorObj = JsonObj->GetObjectField(TEXT("error"));
		FString ErrorMsg = ErrorObj.IsValid() ? ErrorObj->GetStringField(TEXT("message")) : TEXT("Unknown API error");
		UE_LOG(LogAIChat, Error, TEXT("[AI Chat] OpenAI API error: %s"), *ErrorMsg);
		return MakeErrorResponse(EAIChatErrorType::APIError, ErrorMsg);
	}

	// 3. 解析正常响应（数组安全访问）
	if (!JsonObj->HasField(TEXT("choices")))
	{
		return MakeErrorResponse(EAIChatErrorType::InvalidResponseFormat, TEXT("No 'choices' field in response"));
	}

	const TArray<TSharedPtr<FJsonValue>>& Choices = JsonObj->GetArrayField(TEXT("choices"));
	if (Choices.Num() == 0 || !Choices[0].IsValid())
	{
		return MakeErrorResponse(EAIChatErrorType::InvalidResponseFormat, TEXT("Empty 'choices' array"));
	}

	TSharedPtr<FJsonObject> ChoiceObj = Choices[0]->AsObject();
	if (!ChoiceObj.IsValid() || !ChoiceObj->HasField(TEXT("message")))
	{
		return MakeErrorResponse(EAIChatErrorType::InvalidResponseFormat, TEXT("No 'message' field in choice"));
	}

	TSharedPtr<FJsonObject> MsgObj = ChoiceObj->GetObjectField(TEXT("message"));
	if (!MsgObj.IsValid())
	{
		return MakeErrorResponse(EAIChatErrorType::InvalidResponseFormat, TEXT("Invalid 'message' object"));
	}

	// 4. 组装成功响应
	FAIChatResponse Res;
	Res.bSuccess = true;
	Res.Content = MsgObj->GetStringField(TEXT("content"));

	// 处理DeepSeek Reasoning内容
	if (MsgObj->HasField(TEXT("reasoning_content")))
	{
		Res.Content += TEXT("\n\n[Reasoning]\n") + MsgObj->GetStringField(TEXT("reasoning_content"));
	}

	return Res;
}

void FAIChatRequest_OpenAI::SetupHttpRequest(const FAIChatSettings& Settings,
	TSharedRef<IHttpRequest> HttpRequest) const
{
	HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Settings.APIKey));

}

FString FAIChatRequest_Claude::BuildPayload(const FAIChatSettings& Settings) const
{
	TSharedPtr<FJsonObject> JsonPayload = MakeShareable(new FJsonObject());
	JsonPayload->SetStringField(TEXT("model"), Settings.GetModelName());
	JsonPayload->SetNumberField(TEXT("max_tokens"), Settings.MaxTokens);
	JsonPayload->SetNumberField(TEXT("temperature"), Settings.Temperature);

	// 拆分System消息和普通消息
	FString SystemMsg;
	TArray<TSharedPtr<FJsonValue>> MessagesArray;
	for (const FAIChatMessage& Msg : Settings.Messages)
	{
		if (Msg.Role.IsEmpty() || Msg.Content.IsEmpty()) continue;

		if (Msg.Role == TEXT("system"))
		{
			SystemMsg = Msg.Content;
		}
		else if (Msg.Role == TEXT("user") || Msg.Role == TEXT("assistant"))
		{
			TSharedPtr<FJsonObject> JsonMsg = MakeShareable(new FJsonObject());
			JsonMsg->SetStringField(TEXT("role"), Msg.Role);
			JsonMsg->SetStringField(TEXT("content"), Msg.Content);
			MessagesArray.Add(MakeShareable(new FJsonValueObject(JsonMsg)));
		}
	}

	if (!SystemMsg.IsEmpty())
	{
		JsonPayload->SetStringField(TEXT("system"), SystemMsg);
	}
	JsonPayload->SetArrayField(TEXT("messages"), MessagesArray);

	// 序列化
	FString PayloadStr;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadStr);
	if (!FJsonSerializer::Serialize(JsonPayload.ToSharedRef(), Writer))
	{
		return TEXT("");
	}
	return PayloadStr;
}

FAIChatResponse FAIChatRequest_Claude::ParseResponse(const FString& ResponseStr) const
{
	
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);
	if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
	{
		UE_LOG(LogAIChat, Error, TEXT("[AI Chat] Claude: Failed to parse JSON"));
		return MakeErrorResponse(EAIChatErrorType::JsonParseFailed, TEXT("Failed to parse JSON response"));
	}

	// 检查API错误
	if (JsonObj->HasField(TEXT("error")))
	{
		TSharedPtr<FJsonObject> ErrorObj = JsonObj->GetObjectField(TEXT("error"));
		FString ErrorMsg = ErrorObj.IsValid() ? ErrorObj->GetStringField(TEXT("message")) : TEXT("Unknown API error");
		return MakeErrorResponse(EAIChatErrorType::APIError, ErrorMsg);
	}

	// 解析Content
	if (!JsonObj->HasField(TEXT("content")))
	{
		return MakeErrorResponse(EAIChatErrorType::InvalidResponseFormat, TEXT("No 'content' field in response"));
	}

	const TArray<TSharedPtr<FJsonValue>>& ContentArray = JsonObj->GetArrayField(TEXT("content"));
	if (ContentArray.Num() == 0 || !ContentArray[0].IsValid())
	{
		return MakeErrorResponse(EAIChatErrorType::InvalidResponseFormat, TEXT("Empty 'content' array"));
	}

	TSharedPtr<FJsonObject> ContentObj = ContentArray[0]->AsObject();
	if (!ContentObj.IsValid() || !ContentObj->HasField(TEXT("text")))
	{
		return MakeErrorResponse(EAIChatErrorType::InvalidResponseFormat, TEXT("No 'text' field in content"));
	}

	FAIChatResponse Res;
	Res.bSuccess = true;
	Res.Content = ContentObj->GetStringField(TEXT("text"));
	return Res;
}

void FAIChatRequest_Claude::SetupHttpRequest(const FAIChatSettings& Settings,
	TSharedRef<IHttpRequest> HttpRequest) const
{
	HttpRequest->SetHeader(TEXT("x-api-key"), Settings.APIKey);
	HttpRequest->SetHeader(TEXT("anthropic-version"), TEXT("2023-06-01"));
}

// ========== DeepSeek 实现 ==========

FString FAIChatRequest_Deepseek::BuildPayload(const FAIChatSettings& Settings) const
{
	TSharedPtr<FJsonObject> JsonPayload = MakeShareable(new FJsonObject());
	JsonPayload->SetStringField(TEXT("model"), Settings.GetModelName());
	JsonPayload->SetNumberField(TEXT("max_tokens"), Settings.MaxTokens);
	JsonPayload->SetNumberField(TEXT("temperature"), Settings.Temperature);
	JsonPayload->SetBoolField(TEXT("stream"), Settings.bStreamResponse);

	// 构建消息数组
	TArray<TSharedPtr<FJsonValue>> MessagesArray;
	for (const FAIChatMessage& Msg : Settings.Messages)
	{
		if (Msg.Role.IsEmpty() || Msg.Content.IsEmpty()) continue;

		TSharedPtr<FJsonObject> JsonMsg = MakeShareable(new FJsonObject());
		JsonMsg->SetStringField(TEXT("role"), Msg.Role);
		JsonMsg->SetStringField(TEXT("content"), Msg.Content);
		MessagesArray.Add(MakeShareable(new FJsonValueObject(JsonMsg)));
	}
	JsonPayload->SetArrayField(TEXT("messages"), MessagesArray);

	// 序列化
	FString PayloadStr;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadStr);
	if (!FJsonSerializer::Serialize(JsonPayload.ToSharedRef(), Writer))
	{
		return TEXT("");
	}
	return PayloadStr;
}

FAIChatResponse FAIChatRequest_Deepseek::ParseResponse(const FString& ResponseStr) const
{
	// 解析 JSON
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);
	if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
	{
		UE_LOG(LogAIChat, Error, TEXT("[AI Chat] DeepSeek: Failed to parse JSON"));
		return MakeErrorResponse(EAIChatErrorType::JsonParseFailed, TEXT("Failed to parse JSON response"));
	}

	// 检查 API 错误
	if (JsonObj->HasField(TEXT("error")))
	{
		TSharedPtr<FJsonObject> ErrorObj = JsonObj->GetObjectField(TEXT("error"));
		FString ErrorMsg = ErrorObj.IsValid() ? ErrorObj->GetStringField(TEXT("message")) : TEXT("Unknown API error");
		UE_LOG(LogAIChat, Error, TEXT("[AI Chat] DeepSeek API error: %s"), *ErrorMsg);
		return MakeErrorResponse(EAIChatErrorType::APIError, ErrorMsg);
	}

	// 解析响应
	if (!JsonObj->HasField(TEXT("choices")))
	{
		return MakeErrorResponse(EAIChatErrorType::InvalidResponseFormat, TEXT("No 'choices' field in response"));
	}

	const TArray<TSharedPtr<FJsonValue>>& Choices = JsonObj->GetArrayField(TEXT("choices"));
	if (Choices.Num() == 0 || !Choices[0].IsValid())
	{
		return MakeErrorResponse(EAIChatErrorType::InvalidResponseFormat, TEXT("Empty 'choices' array"));
	}

	TSharedPtr<FJsonObject> ChoiceObj = Choices[0]->AsObject();
	if (!ChoiceObj.IsValid() || !ChoiceObj->HasField(TEXT("message")))
	{
		return MakeErrorResponse(EAIChatErrorType::InvalidResponseFormat, TEXT("No 'message' field in choice"));
	}

	TSharedPtr<FJsonObject> MsgObj = ChoiceObj->GetObjectField(TEXT("message"));
	if (!MsgObj.IsValid())
	{
		return MakeErrorResponse(EAIChatErrorType::InvalidResponseFormat, TEXT("Invalid 'message' object"));
	}

	// 组装成功响应
	FAIChatResponse Res;
	Res.bSuccess = true;
	Res.Content = MsgObj->GetStringField(TEXT("content"));

	// DeepSeek Reasoner 特有：处理推理内容
	if (MsgObj->HasField(TEXT("reasoning_content")))
	{
		FString ReasoningContent = MsgObj->GetStringField(TEXT("reasoning_content"));
		if (!ReasoningContent.IsEmpty())
		{
			Res.Content += TEXT("\n\n") + FString::Printf(TEXT("─────────────────────────────\n🧠 Reasoning Process:\n─────────────────────────────\n%s"), *ReasoningContent);
		}
	}

	UE_LOG(LogAIChat, Log, TEXT("[AI Chat] DeepSeek: Response received successfully"));
	return Res;
}

void FAIChatRequest_Deepseek::SetupHttpRequest(const FAIChatSettings& Settings,
	TSharedRef<IHttpRequest> HttpRequest) const
{
	HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Settings.APIKey));
}
