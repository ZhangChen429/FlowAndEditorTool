using UnrealBuildTool;

public class InteractionCinematicEditor : ModuleRules
{
    public InteractionCinematicEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "InteractionCinematic",
                "Flow",
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
                "LevelSequence",
                "MovieScene",
                "Sequencer",
                "PropertyEditor",
                "EditorStyle",
                "LevelSequenceEditor",
                "FlowEditor",
                "GraphEditor",
                "InputCore"
            }
        );
    }
}
