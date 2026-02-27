// Copyright Epic Games, Inc. All Rights Reserved.

#include "Editor/AIChatCommand.h"
#include "Style/AIChatEditorStyle.h"

#define LOCTEXT_NAMESPACE "AIChatCommands"

void FAIChatCommand::RegisterCommands()
{
	// 注册 AI Chat 窗口打开命令，并关联图标
	UI_COMMAND(
		OpenAIChatWindow,                                  // 成员变量名
		"AI Chat",                                         // 按钮显示文本
		"Open AI Chat Support Window",                    // 工具提示
		EUserInterfaceActionType::Button,                 // 类型
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::A)  // 快捷键 Ctrl+Shift+A
	);
}

#undef LOCTEXT_NAMESPACE
