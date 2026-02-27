// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIChatStructs.generated.h"

/**
 * AI 提供商枚举
 */
UENUM(BlueprintType)
enum class EAIProvider : uint8
{
	DeepSeek    UMETA(DisplayName = "DeepSeek"),
	OpenAI      UMETA(DisplayName = "OpenAI"),
	Claude      UMETA(DisplayName = "Claude (Anthropic)"),
	Custom      UMETA(DisplayName = "Custom")
};

/**
 * API 协议类型（中转站模式使用）
 * 决定请求/响应的 JSON 格式
 */
UENUM(BlueprintType)
enum class EAPIProtocol : uint8
{
	OpenAI     UMETA(DisplayName = "OpenAI 兼容"),   // 大多数中转站
	Claude     UMETA(DisplayName = "Claude 原生")    // Anthropic 官方格式
};

/**
 * DeepSeek 模型枚举
 */
UENUM(BlueprintType)
enum class EDeepSeekModel : uint8
{
	Chat        UMETA(DisplayName = "deepseek-chat"),
	Reasoner    UMETA(DisplayName = "deepseek-reasoner")
};

/**
 * Claude 模型枚举
 */
UENUM(BlueprintType)
enum class EClaudeModel : uint8
{
	Opus4_6         UMETA(DisplayName = "claude-opus-4-6"),
	Sonnet3_5       UMETA(DisplayName = "claude-3-5-sonnet-20241022"),
	Haiku3_5        UMETA(DisplayName = "claude-3-5-haiku-20241022"),
	Opus3           UMETA(DisplayName = "claude-3-opus-20240229"),
	Sonnet3         UMETA(DisplayName = "claude-3-sonnet-20240229")
};

/**
 * 聊天消息结构
 */
USTRUCT(BlueprintType)
struct FAIChatMessage
{
	GENERATED_BODY()

	// 角色: "user", "assistant", "system"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Chat")
	FString Role = TEXT("user");

	// 消息内容
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Chat")
	FString Content;

	FAIChatMessage() {}
	FAIChatMessage(const FString& InRole, const FString& InContent)
		: Role(InRole), Content(InContent) {}
};

/**
 * AI Chat 设置
 */
USTRUCT(BlueprintType)
struct FAIChatSettings
{
	GENERATED_BODY()

	// AI 提供商
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Chat")
	EAIProvider Provider = EAIProvider::DeepSeek;

	// DeepSeek 模型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Chat")
	EDeepSeekModel DeepSeekModel = EDeepSeekModel::Chat;

	// Claude 模型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Chat")
	EClaudeModel ClaudeModel = EClaudeModel::Sonnet3_5;

	// 自定义 API URL（当 Provider 为 Custom 时使用，或覆盖默认 URL）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Chat")
	FString CustomAPIUrl;

	// 是否使用自定义 URL（勾选后将覆盖默认 API 地址）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Chat")
	bool bUseCustomURL = false;

	// 中转站 API 协议（决定请求/响应格式）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Chat")
	EAPIProtocol CustomProtocol = EAPIProtocol::OpenAI;

	// 自定义模型名称（用于中转站）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Chat")
	FString CustomModelName;

	// API Key
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Chat")
	FString APIKey;

	// 最大 Token 数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Chat")
	int32 MaxTokens = 4096;

	// 温度参数 (0.0 - 2.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Chat")
	float Temperature = 1.0f;

	// 消息列表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Chat")
	TArray<FAIChatMessage> Messages;

	// 是否流式响应
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Chat")
	bool bStreamResponse = false;

	// 获取 API URL（支持自定义中转站）
	FString GetAPIUrl() const
	{
		// 优先使用自定义 URL（适用于 AI 中转站）
		if (bUseCustomURL && !CustomAPIUrl.IsEmpty())
		{
			return CustomAPIUrl;
		}

		// 使用默认 API 地址
		if (Provider == EAIProvider::Custom)
		{
			return CustomAPIUrl;
		}
		else if (Provider == EAIProvider::DeepSeek)
		{
			return TEXT("https://api.deepseek.com/chat/completions");
		}
		else if (Provider == EAIProvider::OpenAI)
		{
			return TEXT("https://api.openai.com/v1/chat/completions");
		}
		else if (Provider == EAIProvider::Claude)
		{
			return TEXT("https://api.anthropic.com/v1/messages");
		}
		return TEXT("");
	}

	// 获取模型名称
	FString GetModelName() const
	{
		// 优先使用自定义模型名称（中转站模式）
		if (bUseCustomURL && !CustomModelName.IsEmpty())
		{
			return CustomModelName;
		}

		// 使用 Provider 对应的默认模型
		if (Provider == EAIProvider::DeepSeek)
		{
			return DeepSeekModel == EDeepSeekModel::Chat ? TEXT("deepseek-chat") : TEXT("deepseek-reasoner");
		}
		else if (Provider == EAIProvider::Claude)
		{
			switch (ClaudeModel)
			{
			case EClaudeModel::Opus4_6: return TEXT("claude-opus-4-6");
			case EClaudeModel::Sonnet3_5: return TEXT("claude-3-5-sonnet-20241022");
			case EClaudeModel::Haiku3_5: return TEXT("claude-3-5-haiku-20241022");
			case EClaudeModel::Opus3: return TEXT("claude-3-opus-20240229");
			case EClaudeModel::Sonnet3: return TEXT("claude-3-sonnet-20240229");
			default: return TEXT("claude-3-5-sonnet-20241022");
			}
		}
		return TEXT("gpt-4o-mini"); // OpenAI 默认模型
	}
};
