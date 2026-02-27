#pragma once
#include "CoreMinimal.h"
#include "Data/AIChatStructs.h"

// 前向声明
class FAIChatRequestBase;
struct FAIChatResponse;
enum class EAIChatErrorType : uint8;

// 委托前向声明
DECLARE_DELEGATE_OneParam(FOnAIChatProcessed, const FAIChatResponse&);

/**
 * AI Chat 请求工厂
 * 根据 Provider 创建对应的请求处理器
 */
class AICHATSUPPORTEDITOR_API FAIChatRequestFactory
{
public:
	/**
	 * 根据服务商类型创建对应的请求实例
	 * @param Settings - 聊天设置
	 * @return 请求处理器（共享智能指针，保证异步回调时对象仍然有效）
	 */
	static TSharedPtr<FAIChatRequestBase> CreateRequest(const FAIChatSettings& Settings);

	/**
	 * 对外暴露的简化接口（一键发送请求）
	 * @param Settings - 聊天设置
	 * @param OnProcessed - 响应回调
	 */
	static void SendChatRequest(const FAIChatSettings& Settings, const FOnAIChatProcessed& OnProcessed);
};
