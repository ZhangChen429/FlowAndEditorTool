// Copyright Epic Games, Inc. All Rights Reserved.

#include "Style/AIChatEditorStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"

TSharedPtr<FSlateStyleSet> FAIChatEditorStyle::StyleSet = nullptr;
FName FAIChatEditorStyle::StyleSetName = TEXT("AIChatSupport");

void FAIChatEditorStyle::Initialize()
{
	if (!StyleSet.IsValid())
	{
		StyleSet = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
	}
}

FName FAIChatEditorStyle::GetStyleSetName()
{
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FAIChatEditorStyle::Create()
{
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);

	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));

	// 设置资源根目录为插件的 Resources 文件夹
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("AIChatSupport")->GetBaseDir() / TEXT("Resources"));

	// 注册 AI Chat 按钮图标
	Style->Set(
		"AIChatSupport.OpenAIChatWindow",
		new FSlateVectorImageBrush(Style->RootToContentDir(TEXT("ai-chat.svg")), Icon20x20)
	);

	// 也注册一个 40x40 的版本用于可能的大图标
	Style->Set(
		"AIChatSupport.OpenAIChatWindow.Large",
		new FSlateVectorImageBrush(Style->RootToContentDir(TEXT("ai-chat.svg")), Icon40x40)
	);

	return Style;
}

void FAIChatEditorStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}
