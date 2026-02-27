// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Data/AIChatStructs.h"

class SNotificationItem;
struct FAIChatResponse;

/**
 * AI 聊天窗口
 * 提供完整的聊天界面，包括消息列表、输入框、设置面板等
 */
class AICHATSUPPORTEDITOR_API SAIChatWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAIChatWindow) {}
	SLATE_END_ARGS()

	/** 构造窗口 */
	void Construct(const FArguments& InArgs);

	/** 析构函数 */
	virtual ~SAIChatWindow();

private:
	// ========== UI 组件 ==========

	/** 消息列表视图 */
	TSharedPtr<SListView<TSharedPtr<FAIChatMessage>>> MessageListView;

	/** 输入框 */
	TSharedPtr<SEditableTextBox> InputTextBox;

	/** 发送按钮 */
	TSharedPtr<SButton> SendButton;

	/** 浏览Content按钮 */
	TSharedPtr<SButton> BrowseContentButton;

	/** AI 提供商下拉框 */
	TSharedPtr<SComboBox<TSharedPtr<FString>>> ProviderComboBox;

	/** Temperature 滑块 */
	TSharedPtr<SSlider> TemperatureSlider;

	/** MaxTokens 输入框 */
	TSharedPtr<SSpinBox<int32>> MaxTokensSpinBox;

	/** 自定义 URL 输入框 */
	TSharedPtr<SEditableTextBox> CustomURLTextBox;

	/** 使用自定义 URL 复选框 */
	TSharedPtr<SCheckBox> UseCustomURLCheckBox;

	/** 自定义 API Key 输入框 */
	TSharedPtr<SEditableTextBox> CustomAPIKeyTextBox;

	/** 自定义 Model 输入框 */
	TSharedPtr<SEditableTextBox> CustomModelTextBox;

	/** 加载进度条 */
	TSharedPtr<SProgressBar> LoadingProgressBar;

	/** 状态提示文本 */
	TSharedPtr<STextBlock> StatusTextBlock;

	// ========== 数据 ==========

	/** 消息列表数据源 */
	TArray<TSharedPtr<FAIChatMessage>> MessageList;

	/** AI 提供商选项列表 */
	TArray<TSharedPtr<FString>> ProviderOptions;

	/** 当前选中的提供商 */
	TSharedPtr<FString> CurrentProvider;

	/** 聊天设置 */
	FAIChatSettings ChatSettings;

	/** 是否正在加载 */
	bool bIsLoading;

	/** 通知项 */
	TSharedPtr<SNotificationItem> ActiveNotification;

	// ========== UI 构建方法 ==========

	/** 创建消息列表面板 */
	TSharedRef<SWidget> CreateMessageListPanel();

	/** 创建输入面板 */
	TSharedRef<SWidget> CreateInputPanel();

	/** 创建设置面板 */
	TSharedRef<SWidget> CreateSettingsPanel();

	/** 创建状态提示面板 */
	TSharedRef<SWidget> CreateStatusPanel();

	// ========== 列表视图相关 ==========

	/** 生成消息列表行 */
	TSharedRef<ITableRow> OnGenerateMessageRow(TSharedPtr<FAIChatMessage> Message, const TSharedRef<STableViewBase>& OwnerTable);

	// ========== 事件处理 ==========

	/** 发送按钮点击 */
	FReply OnSendButtonClicked();

	/** 浏览Content按钮点击 */
	FReply OnBrowseContentClicked();

	/** 输入框回车键 */
	void OnInputTextCommitted(const FText& Text, ETextCommit::Type CommitType);

	/** 提供商选择改变 */
	void OnProviderSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

	/** Temperature 改变 */
	void OnTemperatureChanged(float NewValue);

	/** MaxTokens 改变 */
	void OnMaxTokensChanged(int32 NewValue);

	/** 自定义 URL 改变 */
	void OnCustomURLChanged(const FText& Text);

	/** 使用自定义 URL 复选框状态改变 */
	void OnUseCustomURLChanged(ECheckBoxState NewState);

	/** 自定义 API Key 改变 */
	void OnCustomAPIKeyChanged(const FText& Text);

	/** 自定义 Model 改变 */
	void OnCustomModelChanged(const FText& Text);

	/** 提供商下拉框文本生成 */
	TSharedRef<SWidget> OnGenerateProviderWidget(TSharedPtr<FString> InOption);

	/** 获取当前提供商文本 */
	FText GetCurrentProviderText() const;

	// ========== 业务逻辑 ==========

	/** 发送消息到 AI */
	void SendMessage(const FString& MessageContent);

	/** 处理 AI 响应 */
	void OnAIResponseReceived(const FAIChatResponse& Response);

	/** 添加消息到列表 */
	void AddMessageToList(const FString& Role, const FString& Content);

	/** 滚动到列表底部 */
	void ScrollToBottom();

	/** 设置加载状态 */
	void SetLoadingState(bool bLoading, const FString& StatusMessage = TEXT(""));

	/** 显示通知 */
	void ShowNotification(const FText& Message, bool bSuccess);

	/** 清除通知 */
	void ClearNotification();

	/** 从环境变量加载 API Key */
	FString LoadAPIKeyFromEnvironment() const;

	/** 保存设置到 EditorPerProjectUserSettings */
	void SaveSettings() const;

	/** 从 EditorPerProjectUserSettings 加载设置 */
	void LoadSettings();

	/** 获取当前是否可以发送 */
	bool CanSendMessage() const;
};
