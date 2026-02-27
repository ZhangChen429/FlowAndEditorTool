#include "AIChatSupportEditor.h"

DEFINE_LOG_CATEGORY(LogAIChat);
#include "Editor/AIChatCommand.h"
#include "Style/AIChatEditorStyle.h"
#include "UI/SAIChatWindow.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"

#define LOCTEXT_NAMESPACE "FAIChatSupportEditorModule"

const FName FAIChatSupportEditorModule::AIChatTabName = FName(TEXT("AIChatWindow"));

void FAIChatSupportEditorModule::StartupModule()
{
	// 初始化样式（包含图标）
	FAIChatEditorStyle::Initialize();

	// 初始化命令列表
	AIChatCommands = MakeShareable(new FUICommandList);

	// 注册命令
	FAIChatCommand::Register();

	// 绑定命令到操作
	AIChatCommands->MapAction(
		FAIChatCommand::Get().OpenAIChatWindow,
		FExecuteAction::CreateRaw(this, &FAIChatSupportEditorModule::OnAIChatButtonClicked),
		FCanExecuteAction()
	);

	// 注册 DockTab
	RegisterDockTab();

	// 注册菜单扩展
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAIChatSupportEditorModule::RegisterMenus)
	);
}

void FAIChatSupportEditorModule::ShutdownModule()
{
	// 清理菜单
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	// 注销 DockTab
	if (FGlobalTabmanager::Get()->HasTabSpawner(AIChatTabName))
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AIChatTabName);
	}

	// 注销命令
	FAIChatCommand::Unregister();

	// 清理命令列表
	if (AIChatCommands.IsValid())
	{
		AIChatCommands.Reset();
	}

	// 清理样式
	FAIChatEditorStyle::Shutdown();
}

void FAIChatSupportEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	// 在工具栏中添加 AI Chat 按钮
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	if (ToolbarMenu)
	{
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
		FToolMenuEntry& Entry = Section.AddEntry(
			FToolMenuEntry::InitToolBarButton(FAIChatCommand::Get().OpenAIChatWindow)
		);
		Entry.SetCommandList(AIChatCommands);
	}
}

void FAIChatSupportEditorModule::RegisterDockTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		AIChatTabName,
		FOnSpawnTab::CreateRaw(this, &FAIChatSupportEditorModule::OnSpawnAIChatTab))
		.SetDisplayName(LOCTEXT("AIChatTabTitle", "AI Chat"))
		.SetTooltipText(LOCTEXT("AIChatTabTooltip", "Open the AI Chat window"))
		.SetIcon(FSlateIcon(FAIChatEditorStyle::GetStyleSetName(), "AIChatSupport.OpenAIChatWindow"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory());
}

void FAIChatSupportEditorModule::OnAIChatButtonClicked()
{
	UE_LOG(LogAIChat, Log, TEXT("AI Chat button clicked!"));

	// 打开或激活 AI Chat 窗口
	FGlobalTabmanager::Get()->TryInvokeTab(AIChatTabName);
}

TSharedRef<SDockTab> FAIChatSupportEditorModule::OnSpawnAIChatTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SAIChatWindow)
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAIChatSupportEditorModule, AIChatSupportEditor)