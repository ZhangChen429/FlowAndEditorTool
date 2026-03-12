// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/SAIChatWindow.h"
#include "AIChatSupportEditor.h"
#include "Request/AIChatRequestFactory.h"
#include "Request/AIChatRequest.h"
#include "Settings/AIChatSupportSettings.h"
#include "Utils/AIChatFileUtils.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SAIChatWindow"

void SAIChatWindow::Construct(const FArguments& InArgs)
{
	bIsLoading = false;

	// 初始化提供商选项
	ProviderOptions.Add(MakeShareable(new FString(TEXT("DeepSeek"))));
	ProviderOptions.Add(MakeShareable(new FString(TEXT("OpenAI"))));
	ProviderOptions.Add(MakeShareable(new FString(TEXT("Claude"))));
	CurrentProvider = ProviderOptions[0];

	// 初始化默认聊天设置
	ChatSettings.Provider = EAIProvider::DeepSeek;
	ChatSettings.DeepSeekModel = EDeepSeekModel::Chat;
	ChatSettings.ClaudeModel = EClaudeModel::Sonnet3_5;
	ChatSettings.MaxTokens = 4096;
	ChatSettings.Temperature = 1.0f;
	ChatSettings.bUseCustomURL = false;
	ChatSettings.CustomModelName = TEXT("");
	ChatSettings.APIKey = LoadAPIKeyFromEnvironment();

	// 从 Editor Preferences 加载中转站配置
	if (const UAIChatSupportSettings* S = UAIChatSupportSettings::Get())
	{
		ChatSettings.bUseCustomURL   = S->bUseCustomURL;
		ChatSettings.CustomAPIUrl    = S->CustomAPIUrl;
		ChatSettings.CustomModelName = S->CustomModelName;
		ChatSettings.CustomProtocol  = S->CustomProtocol;

		if (S->bUseCustomURL)
			ChatSettings.APIKey = S->CustomAPIKey;
	}

	// 添加欢迎消息
	AddMessageToList(TEXT("system"), TEXT("Welcome to AI Chat! Select your provider and start chatting.\n\n💡 Tips:\n- Click 'Browse Content' button to let AI view your project files\n- Type '/browse <folder_path>' to explore specific folders (e.g., /browse /Game/Blueprints)\n- Type '/browse' to view /Game/ directory"));

	// 构建 UI
	ChildSlot
	[
		SNew(SVerticalBox)

		// 设置面板
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f)
		[
			CreateSettingsPanel()
		]

		// 分隔线
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
		]

		// 消息列表
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(5.0f)
		[
			CreateMessageListPanel()
		]

		// 状态提示
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f)
		[
			CreateStatusPanel()
		]

		// 输入面板
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f)
		[
			CreateInputPanel()
		]
	];
}

SAIChatWindow::~SAIChatWindow()
{
	ClearNotification();
}

bool SAIChatWindow::HandleSlashCommand(const FString& MessageContent)
{
	  TArray<FString> Args;
        MessageContent.ParseIntoArrayWS(Args);

        if (Args.Num() == 0) return false;

        const FString Cmd = Args[0].ToLower();

        // 兼容你已有功能
        if (Cmd == TEXT("/browse"))
        {
                FString FolderPath = Args.Num() > 1 ? Args[1] : TEXT("/Game/");
                TArray<FAIChatFileInfo> Files;
                if (UAIChatFileUtils::ListFilesInFolder(FolderPath, Files))
                {
                        AddMessageToList(TEXT("system"), UAIChatFileUtils::FormatFileListForAI(Files, FolderPath));
                }
                else
                {
                        AddMessageToList(TEXT("system"), FString::Printf(TEXT("❌ Browse failed: %s"), *FolderPath));
                }
                return true;
        }

        // /files *.cpp [limit]
        if (Cmd == TEXT("/files"))
        {
                const FString Pattern = Args.Num() > 1 ? Args[1] : TEXT("*.cpp");
                const int32 Limit = Args.Num() > 2 ? FCString::Atoi(*Args[2]) : 100;

                TArray<FString> Files;
                FString Err;
                if (UAIChatFileUtils::FindProjectFilesByGlob(Pattern, Limit, Files, Err))
                {
                        FString Out = FString::Printf(TEXT("FILES (%d):\n"), Files.Num());
                        for (const FString& F : Files) Out += FString::Printf(TEXT("- %s\n"), *F);
                        AddMessageToList(TEXT("system"), Out);
                }
                else
                {
                        AddMessageToList(TEXT("system"), FString::Printf(TEXT("❌ /files failed: %s"), *Err));
                }
                return true;
        }

        // /read <path> [start] [count]
        if (Cmd == TEXT("/read"))
        {
                if (Args.Num() < 2)
                {
                        AddMessageToList(TEXT("system"), TEXT("用法: /read <path> [startLine] [lineCount]"));
                        return true;
                }

                const FString Path = Args[1];
                const int32 Start = Args.Num() > 2 ? FMath::Max(1, FCString::Atoi(*Args[2])) : 1;
                const int32 Count = Args.Num() > 3 ? FMath::Clamp(FCString::Atoi(*Args[3]), 1, 400) : 200;

                FString Text, Err;
                if (UAIChatFileUtils::ReadProjectTextFile(Path, Start, Count, Text, Err))
                {
                        AddMessageToList(TEXT("system"), Text);
                }
                else
                {
                        AddMessageToList(TEXT("system"), FString::Printf(TEXT("❌ /read failed: %s"), *Err));
                }
                return true;
        }

        // /grep <keyword> [glob] [maxHits]
        if (Cmd == TEXT("/grep"))
        {
                if (Args.Num() < 2)
                {
                        AddMessageToList(TEXT("system"), TEXT("用法: /grep <keyword> [glob] [maxHits]"));
                        return true;
                }

                const FString Keyword = Args[1];
                const FString Glob = Args.Num() > 2 ? Args[2] : TEXT("*.{h,cpp}");
                const int32 MaxHits = Args.Num() > 3 ? FMath::Clamp(FCString::Atoi(*Args[3]), 1, 200) : 80;

                FString Text, Err;
                if (UAIChatFileUtils::GrepProjectFiles(Keyword, Glob, MaxHits, Text, Err))
                {
                        AddMessageToList(TEXT("system"), Text);
                }
                else
                {
                        AddMessageToList(TEXT("system"), FString::Printf(TEXT("❌ /grep failed: %s"), *Err));
                }
                return true;
        }

        return false;
}

TSharedRef<SWidget> SAIChatWindow::CreateMessageListPanel()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(5.0f)
		[
			SAssignNew(MessageListView, SListView<TSharedPtr<FAIChatMessage>>)
			.ListItemsSource(&MessageList)
			.OnGenerateRow(this, &SAIChatWindow::OnGenerateMessageRow)
			.SelectionMode(ESelectionMode::Single)
		];
}

TSharedRef<SWidget> SAIChatWindow::CreateInputPanel()
{
	return SNew(SHorizontalBox)

		// 浏览Content按钮
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 5.0f, 0.0f)
		[
			SAssignNew(BrowseContentButton, SButton)
			.Text(LOCTEXT("BrowseContentButton", "Browse Content"))
			.ToolTipText(LOCTEXT("BrowseContentTooltip", "Let AI view project Content folder structure"))
			.OnClicked(this, &SAIChatWindow::OnBrowseContentClicked)
			.IsEnabled(this, &SAIChatWindow::CanSendMessage)
		]

		// 输入框
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 0.0f, 5.0f, 0.0f)
		[
			SAssignNew(InputTextBox, SEditableTextBox)
			.HintText(LOCTEXT("InputHint", "Type your message here..."))
			.OnTextCommitted(this, &SAIChatWindow::OnInputTextCommitted)
			.IsEnabled(this, &SAIChatWindow::CanSendMessage)
		]

		// 发送按钮
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SAssignNew(SendButton, SButton)
			.Text(LOCTEXT("SendButton", "Send"))
			.OnClicked(this, &SAIChatWindow::OnSendButtonClicked)
			.IsEnabled(this, &SAIChatWindow::CanSendMessage)
		];
}

TSharedRef<SWidget> SAIChatWindow::CreateSettingsPanel()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(5.0f)
		[
			SNew(SVerticalBox)

			// 第一行：Provider、Temperature、MaxTokens
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

			// AI 提供商选择
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 10.0f, 0.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 5.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ProviderLabel", "Provider:"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(ProviderComboBox, SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&ProviderOptions)
					.OnGenerateWidget(this, &SAIChatWindow::OnGenerateProviderWidget)
					.OnSelectionChanged(this, &SAIChatWindow::OnProviderSelectionChanged)
					.Content()
					[
						SNew(STextBlock)
						.Text(this, &SAIChatWindow::GetCurrentProviderText)
					]
				]
			]

			// Temperature 设置
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 10.0f, 0.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 5.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("TemperatureLabel", "Temperature:"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(100.0f)
					[
						SAssignNew(TemperatureSlider, SSlider)
						.MinValue(0.0f)
						.MaxValue(2.0f)
						.Value(1.0f)
						.OnValueChanged(this, &SAIChatWindow::OnTemperatureChanged)
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(5.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { return FText::AsNumber(ChatSettings.Temperature, &FNumberFormattingOptions::DefaultNoGrouping()); })
				]
			]

			// MaxTokens 设置
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 5.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("MaxTokensLabel", "Max Tokens:"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(MaxTokensSpinBox, SSpinBox<int32>)
					.MinValue(100)
					.MaxValue(32000)
					.Value(4096)
					.OnValueChanged(this, &SAIChatWindow::OnMaxTokensChanged)
				]
			]
			]

			// 第二行：自定义 URL 复选框
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 5.0f, 0.0f, 0.0f)
			[
				SAssignNew(UseCustomURLCheckBox, SCheckBox)
				.OnCheckStateChanged(this, &SAIChatWindow::OnUseCustomURLChanged)
				.Content()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("UseCustomURL", "☑ Use Custom URL (中转站模式)"))
				]
			]

			// 第三行：自定义 URL 输入框
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 3.0f, 0.0f, 0.0f)
			[
				SNew(SHorizontalBox)
				.Visibility_Lambda([this]() { return ChatSettings.bUseCustomURL ? EVisibility::Visible : EVisibility::Collapsed; })

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 5.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("URLLabel", "URL:"))
					.MinDesiredWidth(60.0f)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SAssignNew(CustomURLTextBox, SEditableTextBox)
					.HintText(LOCTEXT("CustomURLHint", "https://api.example.com/v1/chat/completions"))
					.OnTextChanged(this, &SAIChatWindow::OnCustomURLChanged)
				]
			]

			// 第四行：API Key 输入框
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 3.0f, 0.0f, 0.0f)
			[
				SNew(SHorizontalBox)
				.Visibility_Lambda([this]() { return ChatSettings.bUseCustomURL ? EVisibility::Visible : EVisibility::Collapsed; })

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 5.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("APIKeyLabel", "API Key:"))
					.MinDesiredWidth(60.0f)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SAssignNew(CustomAPIKeyTextBox, SEditableTextBox)
					.HintText(LOCTEXT("APIKeyHint", "sk-xxxxxxxx"))
					.IsPassword(true)
					.OnTextChanged(this, &SAIChatWindow::OnCustomAPIKeyChanged)
				]
			]

			// 第五行：Model 输入框
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 3.0f, 0.0f, 0.0f)
			[
				SNew(SHorizontalBox)
				.Visibility_Lambda([this]() { return ChatSettings.bUseCustomURL ? EVisibility::Visible : EVisibility::Collapsed; })

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 5.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ModelLabel", "Model:"))
					.MinDesiredWidth(60.0f)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SAssignNew(CustomModelTextBox, SEditableTextBox)
					.HintText(LOCTEXT("ModelHint", "claude-3-5-sonnet-20241022 / gpt-4o / deepseek-chat"))
					.OnTextChanged(this, &SAIChatWindow::OnCustomModelChanged)
				]
			]

			// 第六行：协议选择
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 3.0f, 0.0f, 0.0f)
			[
				SNew(SHorizontalBox)
				.Visibility_Lambda([this]() { return ChatSettings.bUseCustomURL ? EVisibility::Visible : EVisibility::Collapsed; })

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 5.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ProtocolLabel", "Protocol:"))
					.MinDesiredWidth(60.0f)
				]

				// OpenAI 兼容
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 10.0f, 0.0f)
				[
					SNew(SCheckBox)
					.Style(FAppStyle::Get(), "RadioButton")
					.IsChecked_Lambda([this]() { return ChatSettings.CustomProtocol == EAPIProtocol::OpenAI ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([this](ECheckBoxState) { ChatSettings.CustomProtocol = EAPIProtocol::OpenAI; })
					.Content()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("OpenAIProtocol", "OpenAI 兼容 (大多数中转站)"))
					]
				]

				// Claude 原生
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SCheckBox)
					.Style(FAppStyle::Get(), "RadioButton")
					.IsChecked_Lambda([this]() { return ChatSettings.CustomProtocol == EAPIProtocol::Claude ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([this](ECheckBoxState) { ChatSettings.CustomProtocol = EAPIProtocol::Claude; })
					.Content()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ClaudeProtocol", "Claude 原生"))
					]
				]
			]
		];
}

TSharedRef<SWidget> SAIChatWindow::CreateStatusPanel()
{
	return SNew(SHorizontalBox)
		.Visibility_Lambda([this]() { return bIsLoading ? EVisibility::Visible : EVisibility::Collapsed; })

		// 加载进度条
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 0.0f, 5.0f, 0.0f)
		[
			SNew(SBox)
			.WidthOverride(100.0f)
			[
				SAssignNew(LoadingProgressBar, SProgressBar)
				.Percent_Lambda([]() { return TOptional<float>(); }) // 无限循环进度条
			]
		]

		// 状态文本
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SAssignNew(StatusTextBlock, STextBlock)
			.Text(LOCTEXT("LoadingText", "AI is thinking..."))
		];
}

TSharedRef<ITableRow> SAIChatWindow::OnGenerateMessageRow(TSharedPtr<FAIChatMessage> Message, const TSharedRef<STableViewBase>& OwnerTable)
{
	// 根据角色确定颜色和对齐方式
	FSlateColor MessageColor = FSlateColor::UseForeground();
	EHorizontalAlignment Alignment = HAlign_Left;

	if (Message->Role == TEXT("user"))
	{
		MessageColor = FLinearColor(0.2f, 0.6f, 1.0f); // 蓝色
		Alignment = HAlign_Right;
	}
	else if (Message->Role == TEXT("assistant"))
	{
		MessageColor = FLinearColor(0.2f, 1.0f, 0.6f); // 绿色
		Alignment = HAlign_Left;
	}
	else // system
	{
		MessageColor = FLinearColor(0.7f, 0.7f, 0.7f); // 灰色
		Alignment = HAlign_Center;
	}

	return SNew(STableRow<TSharedPtr<FAIChatMessage>>, OwnerTable)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(10.0f, 5.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(Alignment)
				[
					SNew(SVerticalBox)

					// 角色标签
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 3.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Message->Role.ToUpper()))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
						.ColorAndOpacity(MessageColor)
					]

					// 消息内容
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(Message->Content))
						.AutoWrapText(true)
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
				]
			]
		];
}

FReply SAIChatWindow::OnSendButtonClicked()
{
	FString MessageContent = InputTextBox->GetText().ToString();
	if (!MessageContent.IsEmpty())
	{
		SendMessage(MessageContent);
		InputTextBox->SetText(FText::GetEmpty());
	}
	return FReply::Handled();
}

FReply SAIChatWindow::OnBrowseContentClicked()
{
	UE_LOG(LogAIChat, Log, TEXT("[AI Chat] Browse Content clicked"));

	// 列举/Game/目录的文件
	TArray<FAIChatFileInfo> Files;
	if (UAIChatFileUtils::ListFilesInFolder(TEXT("/Game/"), Files))
	{
		// 格式化文件列表
		FString FormattedList = UAIChatFileUtils::FormatFileListForAI(Files, TEXT("/Game/"));

		// 添加系统消息显示文件列表
		AddMessageToList(TEXT("system"), FormattedList);

		// 添加提示消息
		FString HintMessage = TEXT("I've listed the project Content folder for you. You can now ask questions about these files or request to view specific folders.");
		AddMessageToList(TEXT("system"), HintMessage);

		// 自动发送一条消息给AI,让它知道文件结构
		FString AIPrompt = FString::Printf(
			TEXT("The user has shown you the project's Content folder structure. Here are the available files and folders:\n\n%s\n\nPlease acknowledge this and offer to help with any questions about the project structure."),
			*FormattedList
		);

		SendMessage(AIPrompt);
	}
	else
	{
		AddMessageToList(TEXT("system"), TEXT("❌ Failed to list files in /Game/ folder."));
		ShowNotification(LOCTEXT("BrowseContentFailed", "Failed to browse Content folder"), false);
	}

	return FReply::Handled();
}

void SAIChatWindow::OnInputTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter)
	{
		OnSendButtonClicked();
	}
}

void SAIChatWindow::OnProviderSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection.IsValid())
	{
		CurrentProvider = NewSelection;

		if (*NewSelection == TEXT("DeepSeek"))
		{
			ChatSettings.Provider = EAIProvider::DeepSeek;
		}
		else if (*NewSelection == TEXT("OpenAI"))
		{
			ChatSettings.Provider = EAIProvider::OpenAI;
		}
		else if (*NewSelection == TEXT("Claude"))
		{
			ChatSettings.Provider = EAIProvider::Claude;
		}

		// 标准模式从环境变量加载
		ChatSettings.APIKey = LoadAPIKeyFromEnvironment();

		UE_LOG(LogAIChat, Log, TEXT("AI Chat: Provider changed to %s"), **NewSelection);
	}
}

void SAIChatWindow::OnTemperatureChanged(float NewValue)
{
	ChatSettings.Temperature = NewValue;
}

void SAIChatWindow::OnMaxTokensChanged(int32 NewValue)
{
	ChatSettings.MaxTokens = NewValue;
}

void SAIChatWindow::OnCustomURLChanged(const FText& Text)
{
	ChatSettings.CustomAPIUrl = Text.ToString();
	if (UAIChatSupportSettings* S = UAIChatSupportSettings::Get())
	{
		S->CustomAPIUrl = ChatSettings.CustomAPIUrl;
		S->SaveConfig();
	}
}

void SAIChatWindow::OnUseCustomURLChanged(ECheckBoxState NewState)
{
	ChatSettings.bUseCustomURL = (NewState == ECheckBoxState::Checked);

	if (ChatSettings.bUseCustomURL)
	{
		// 切换到中转站模式：恢复保存的 Custom Key
		if (const UAIChatSupportSettings* S = UAIChatSupportSettings::Get())
			ChatSettings.APIKey = S->CustomAPIKey;
		else
			ChatSettings.APIKey = TEXT("");
	}
	else
	{
		// 切换回标准模式：从环境变量重新加载
		ChatSettings.APIKey = LoadAPIKeyFromEnvironment();
	}

	if (UAIChatSupportSettings* S = UAIChatSupportSettings::Get())
	{
		S->bUseCustomURL = ChatSettings.bUseCustomURL;
		S->SaveConfig();
	}
}

void SAIChatWindow::OnCustomAPIKeyChanged(const FText& Text)
{
	ChatSettings.APIKey = Text.ToString();
	if (UAIChatSupportSettings* S = UAIChatSupportSettings::Get())
	{
		S->CustomAPIKey = ChatSettings.APIKey;
		S->SaveConfig();
	}
}

void SAIChatWindow::OnCustomModelChanged(const FText& Text)
{
	ChatSettings.CustomModelName = Text.ToString();
	if (UAIChatSupportSettings* S = UAIChatSupportSettings::Get())
	{
		S->CustomModelName = ChatSettings.CustomModelName;
		S->SaveConfig();
	}
}

TSharedRef<SWidget> SAIChatWindow::OnGenerateProviderWidget(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*InOption));
}

FText SAIChatWindow::GetCurrentProviderText() const
{
	return CurrentProvider.IsValid() ? FText::FromString(*CurrentProvider) : FText::GetEmpty();
}

void SAIChatWindow::SendMessage(const FString& MessageContent)
{
	if (MessageContent.IsEmpty())
	{
		return;
	}

	// 处理特殊命令：/browse <folder_path>
	if (MessageContent.StartsWith(TEXT("/browse ")))
	{
		FString FolderPath = MessageContent.RightChop(8).TrimStartAndEnd(); // 移除 "/browse " 并去除空格

		if (FolderPath.IsEmpty())
		{
			FolderPath = TEXT("/Game/");
		}

		// 列举文件夹
		TArray<FAIChatFileInfo> Files;
		if (UAIChatFileUtils::ListFilesInFolder(FolderPath, Files))
		{
			FString FormattedList = UAIChatFileUtils::FormatFileListForAI(Files, FolderPath);
			AddMessageToList(TEXT("system"), FormattedList);

			// 显示快捷提示
			FString CommonPaths = TEXT("Common paths you can browse:\n");
			for (const FString& Path : UAIChatFileUtils::GetCommonUEPaths())
			{
				CommonPaths += FString::Printf(TEXT("  /browse %s\n"), *Path);
			}
			AddMessageToList(TEXT("system"), CommonPaths);
		}
		else
		{
			AddMessageToList(TEXT("system"), FString::Printf(TEXT("❌ Failed to browse folder: %s"), *FolderPath));
		}
		return;
	}

	// 验证并获取 API Key
	if (ChatSettings.bUseCustomURL)
	{
		// 中转站模式：API Key 由 OnCustomAPIKeyChanged 写入 ChatSettings.APIKey
		if (ChatSettings.APIKey.IsEmpty())
		{
			ShowNotification(LOCTEXT("NoCustomAPIKey", "Please enter API Key for custom URL"), false);
			return;
		}
		if (ChatSettings.CustomAPIUrl.IsEmpty())
		{
			ShowNotification(LOCTEXT("NoCustomURL", "Please enter a custom API URL"), false);
			return;
		}
	}
	else
	{
		// 标准模式：从环境变量加载
		ChatSettings.APIKey = LoadAPIKeyFromEnvironment();
		if (ChatSettings.APIKey.IsEmpty())
		{
			ShowNotification(LOCTEXT("NoAPIKey", "API Key not found! Please set environment variable."), false);
			return;
		}
	}

	// 添加用户消息到列表
	AddMessageToList(TEXT("user"), MessageContent);

	// 准备发送请求
	SetLoadingState(true, TEXT("AI is thinking..."));

	// 构建消息历史（包括新消息）
	ChatSettings.Messages.Empty();
	for (const TSharedPtr<FAIChatMessage>& Msg : MessageList)
	{
		if (Msg->Role != TEXT("system")) // 排除系统消息
		{
			ChatSettings.Messages.Add(*Msg);
		}
	}

	// 打印实际发送参数（方便定位404等问题）
	FString ActualURL = ChatSettings.GetAPIUrl();
	FString ActualModel = ChatSettings.GetModelName();
	FString MaskedKey = ChatSettings.APIKey.Len() > 8
		? ChatSettings.APIKey.Left(4) + TEXT("****") + ChatSettings.APIKey.Right(4)
		: TEXT("****");

	if (ChatSettings.bUseCustomURL)
	{
		UE_LOG(LogAIChat, Warning, TEXT("===== AI Chat Request (中转站模式) ====="));
		UE_LOG(LogAIChat, Warning, TEXT("  URL      : %s"), *ActualURL);
		UE_LOG(LogAIChat, Warning, TEXT("  Model    : %s"), *ActualModel);
		UE_LOG(LogAIChat, Warning, TEXT("  API Key  : %s"), *MaskedKey);
		UE_LOG(LogAIChat, Warning, TEXT("  Protocol : %s"), ChatSettings.CustomProtocol == EAPIProtocol::Claude ? TEXT("Claude 原生") : TEXT("OpenAI 兼容"));
		UE_LOG(LogAIChat, Warning, TEXT("========================================"));
	}
	else
	{
		UE_LOG(LogAIChat, Log, TEXT("AI Chat: URL=%s  Model=%s  Key=%s"), *ActualURL, *ActualModel, *MaskedKey);
	}

	// 发送请求
	FAIChatRequestFactory::SendChatRequest(ChatSettings,
		FOnAIChatProcessed::CreateSP(this, &SAIChatWindow::OnAIResponseReceived));
}

void SAIChatWindow::OnAIResponseReceived(const FAIChatResponse& Response)
{
	SetLoadingState(false);

	if (Response.bSuccess)
	{
		// 添加 AI 响应到列表
		AddMessageToList(TEXT("assistant"), Response.Content);
		ScrollToBottom();

		UE_LOG(LogAIChat, Log, TEXT("AI Chat: Response received successfully"));
	}
	else
	{
		// 显示错误通知
		FString ErrorTypeStr;
		switch (Response.ErrorType)
		{
		case EAIChatErrorType::EmptyAPIKey:
			ErrorTypeStr = TEXT("Empty API Key");
			break;
		case EAIChatErrorType::InvalidPayload:
			ErrorTypeStr = TEXT("Invalid Payload");
			break;
		case EAIChatErrorType::HttpRequestFailed:
			ErrorTypeStr = TEXT("HTTP Request Failed");
			break;
		case EAIChatErrorType::JsonParseFailed:
			ErrorTypeStr = TEXT("JSON Parse Failed");
			break;
		case EAIChatErrorType::InvalidResponseFormat:
			ErrorTypeStr = TEXT("Invalid Response Format");
			break;
		case EAIChatErrorType::APIError:
			ErrorTypeStr = TEXT("API Error");
			break;
		default:
			ErrorTypeStr = TEXT("Unknown Error");
			break;
		}

		FString FullErrorMessage = FString::Printf(TEXT("[%s] %s"), *ErrorTypeStr, *Response.ErrorMessage);
		ShowNotification(FText::FromString(FullErrorMessage), false);

		UE_LOG(LogAIChat, Error, TEXT("AI Chat: Request failed - %s"), *FullErrorMessage);
	}
}

void SAIChatWindow::AddMessageToList(const FString& Role, const FString& Content)
{
	TSharedPtr<FAIChatMessage> NewMessage = MakeShareable(new FAIChatMessage(Role, Content));
	MessageList.Add(NewMessage);

	if (MessageListView.IsValid())
	{
		MessageListView->RequestListRefresh();
		ScrollToBottom();
	}
}

void SAIChatWindow::ScrollToBottom()
{
	if (MessageListView.IsValid() && MessageList.Num() > 0)
	{
		MessageListView->RequestScrollIntoView(MessageList.Last());
	}
}

void SAIChatWindow::SetLoadingState(bool bLoading, const FString& StatusMessage)
{
	bIsLoading = bLoading;

	if (StatusTextBlock.IsValid() && !StatusMessage.IsEmpty())
	{
		StatusTextBlock->SetText(FText::FromString(StatusMessage));
	}
}

void SAIChatWindow::ShowNotification(const FText& Message, bool bSuccess)
{
	ClearNotification();

	FNotificationInfo Info(Message);
	Info.ExpireDuration = 5.0f;
	Info.bUseLargeFont = false;
	Info.bUseSuccessFailIcons = true;

	if (bSuccess)
	{
		Info.Image = FAppStyle::GetBrush(TEXT("NotificationList.SuccessImage"));
	}
	else
	{
		Info.Image = FAppStyle::GetBrush(TEXT("NotificationList.FailImage"));
	}

	ActiveNotification = FSlateNotificationManager::Get().AddNotification(Info);

	if (ActiveNotification.IsValid())
	{
		ActiveNotification->SetCompletionState(bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
	}
}

void SAIChatWindow::ClearNotification()
{
	if (ActiveNotification.IsValid())
	{
		ActiveNotification->ExpireAndFadeout();
		ActiveNotification.Reset();
	}
}

FString SAIChatWindow::LoadAPIKeyFromEnvironment() const
{
	// 如果使用自定义 URL 且已手动输入 API Key，直接返回
	if (ChatSettings.bUseCustomURL && !ChatSettings.APIKey.IsEmpty())
	{
		UE_LOG(LogAIChat, Log, TEXT("AI Chat: Using custom API Key from UI input"));
		return ChatSettings.APIKey;
	}

	// 从环境变量读取
	FString EnvVarName;

	if (ChatSettings.Provider == EAIProvider::DeepSeek)
	{
		EnvVarName = TEXT("PS_DEEPSEEKAPIKEY");
	}
	else if (ChatSettings.Provider == EAIProvider::OpenAI)
	{
		EnvVarName = TEXT("PS_OPENAIAPIKEY");
	}
	else if (ChatSettings.Provider == EAIProvider::Claude)
	{
		EnvVarName = TEXT("ANTHROPIC_API_KEY");
	}

	FString APIKey = FPlatformMisc::GetEnvironmentVariable(*EnvVarName);

	if (APIKey.IsEmpty())
	{
		UE_LOG(LogAIChat, Warning, TEXT("AI Chat: API Key not found in environment variable '%s'"), *EnvVarName);
	}
	else
	{
		UE_LOG(LogAIChat, Log, TEXT("AI Chat: Using API Key from environment variable '%s'"), *EnvVarName);
	}

	return APIKey;
}

bool SAIChatWindow::CanSendMessage() const
{
	if (bIsLoading) return false;

	if (ChatSettings.bUseCustomURL)
	{
		// 中转站模式：URL 和 API Key 都必须填写（均由 callback 写入 ChatSettings）
		return !ChatSettings.APIKey.IsEmpty() && !ChatSettings.CustomAPIUrl.IsEmpty();
	}

	return !ChatSettings.APIKey.IsEmpty();
}

#undef LOCTEXT_NAMESPACE
