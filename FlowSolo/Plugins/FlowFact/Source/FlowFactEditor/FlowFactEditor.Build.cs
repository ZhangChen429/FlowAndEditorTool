using UnrealBuildTool;

public class FlowFactEditor : ModuleRules
{
    public FlowFactEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Flow",
                "FlowFactCore",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "FlowEditor",
                "UnrealEd",
                "GraphEditor", "FlowFact"
            }
        );
    }
}