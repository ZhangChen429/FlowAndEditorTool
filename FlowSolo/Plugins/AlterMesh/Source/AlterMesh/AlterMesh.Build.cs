// Copyright 2023 Aechmea

using System.IO;
using UnrealBuildTool;

public class AlterMesh : ModuleRules
{
	public AlterMesh(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = false;
		PublicDefinitions.Add("ALTERMESH_FREE=0");
		bUsePrecompiled = false;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Json",
				"JsonUtilities",
				"InputCore",
				"AlterMeshRendering",
				"DeveloperSettings",
				"Projects",
				"RenderCore",
				"RHI",
				"Niagara",
				"Landscape",
				"MovieScene",
				"MovieSceneTracks",
				"PropertyPath",
				"SlateCore",
				// ... add other public dependencies that you statically link with here ...
			}
			);

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"UnrealEd",
				"Slate",
				"MeshUtilities",
				"EditorStyle",
				"DirectoryWatcher",
				"MovieSceneTools",
				"NiagaraEditor",
			});
		}

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		string dllDestPath = Path.Combine(PluginDirectory, "Binaries", Target.Platform.ToString(), "AlterMesh.dll");
		string dllSourcePath = Path.Combine(PluginDirectory, "Source", "Extern", "AlterMesh.dll");
		
		Directory.CreateDirectory(Path.Combine(PluginDirectory, "Binaries", Target.Platform.ToString()));

		if (File.Exists(dllDestPath))
		{
			File.SetAttributes(dllDestPath, FileAttributes.Normal);
			File.Delete(dllDestPath);
		}
		
		File.SetAttributes(dllSourcePath, FileAttributes.Normal);
		
		File.Copy(dllSourcePath, dllDestPath, true);
		
		RuntimeDependencies.Add("$(TargetOutputDir)/AlterMesh.dll", Path.Combine(PluginDirectory, "Source/Extern/Staging/AlterMesh.dll"));

		PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Source/Extern/AlterMesh.lib"));
	}
}
