// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WorkspotEditor : ModuleRules
{
	public WorkspotEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
			}
		);


		PrivateIncludePaths.AddRange(
			new string[] {
			}
		);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Workspot",
				"InputCore"
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"AssetTools",
				"PropertyEditor",
				"EditorStyle",
				"ContentBrowser",
				"ToolMenus",
				"SmartObjectsEditorModule",
				"AIModule"
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
