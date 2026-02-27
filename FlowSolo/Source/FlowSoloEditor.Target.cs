// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

using UnrealBuildTool;

public class FlowSoloEditorTarget : TargetRules
{
	public FlowSoloEditorTarget(TargetInfo target) : base(target)
	{
		Type = TargetType.Editor;

		DefaultBuildSettings = BuildSettingsVersion.V6;
		//IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		//NativePointerMemberBehaviorOverride = PointerMemberBehavior.AllowSilently;
		
		ExtraModuleNames.AddRange(new[]
		{ 
			"FlowSolo"
		});
    }
}
