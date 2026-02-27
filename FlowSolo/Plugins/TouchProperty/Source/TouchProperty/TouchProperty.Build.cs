// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TouchProperty : ModuleRules
{
	public TouchProperty(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", "Slate","UMG",
			}
			);
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine","AdvancedPreviewScene",
				"Slate","UnrealEd","Blutility",
				"SlateCore","EditorStyle","InputCore", "UnrealEd","ToolMenus", "Engine",  "ToolMenus","Projects","GraphEditor","Flow","FlowEditor","FlowDebugger"
				
			}
			);
		
	}
}
