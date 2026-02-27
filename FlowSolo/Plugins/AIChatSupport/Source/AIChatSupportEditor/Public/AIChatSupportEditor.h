#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAIChat, Log, All);

class SDockTab;

class FAIChatSupportEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    // 注册编辑器菜单和工具栏
    void RegisterMenus();

    // 注册 DockTab
    void RegisterDockTab();

    // AI Chat 按钮点击事件
    void OnAIChatButtonClicked();

    // 生成 AI Chat 窗口 Tab
    TSharedRef<SDockTab> OnSpawnAIChatTab(const class FSpawnTabArgs& Args);

    // 命令列表
    TSharedPtr<FUICommandList> AIChatCommands;

    // Tab 名称
    static const FName AIChatTabName;
};
