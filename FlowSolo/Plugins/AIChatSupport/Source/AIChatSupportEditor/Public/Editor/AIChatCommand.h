// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "Style/AIChatEditorStyle.h"

/**
 * AI Chat Support 编辑器命令
 * 定义编辑器工具栏和菜单中的按钮命令
 */
class FAIChatCommand : public TCommands<FAIChatCommand>
{
public:
	FAIChatCommand()
		: TCommands<FAIChatCommand>(
			TEXT("AIChatSupport"),                                      // 上下文名称
			NSLOCTEXT("AIChat", "AIChatCommands", "AI Chat Commands"),  // 友好名称
			NAME_None,                                                   // 父级上下文
			FAIChatEditorStyle::GetStyleSetName()                        // 样式集名称
		)
	{
	}

	// 注册所册命令
	virtual void RegisterCommands() override;

	// AI Chat 窗口打开命令
	TSharedPtr<FUICommandInfo> OpenAIChatWindow;
};
