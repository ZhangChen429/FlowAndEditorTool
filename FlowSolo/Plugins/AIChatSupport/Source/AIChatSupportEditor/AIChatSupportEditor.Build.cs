using UnrealBuildTool;

public class AIChatSupportEditor : ModuleRules
{
    public AIChatSupportEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UnrealEd",        // 编辑器功能
                "LevelEditor",     // Level Editor 扩展
                "ToolMenus",       // 工具菜单系统
                "InputCore",       // 输入系统
                "EditorStyle",
                "Projects",        // 插件管理器
                "HTTP",            // HTTP 请求
                "Json",            // JSON 序列化
                "JsonUtilities",   // JSON 工具
                "WorkspaceMenuStructure", // 工作区菜单
                "DeveloperSettings",      // UDeveloperSettings (Editor Preferences)
                "AssetRegistry",          // 资产注册表
                "EditorScriptingUtilities", // UEditorAssetLibrary
            }
        );
    }
}