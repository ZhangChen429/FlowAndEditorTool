// Copyright 2023 Aechmea

#define LOCTEXT_NAMESPACE "FAlterMeshRenderingModule"

#include "AlterMeshRendering.h"
#include "ShaderCore.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FAlterMeshRenderingModule::StartupModule()
{	
	const FString ShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("AlterMesh"))->GetBaseDir(), TEXT("Source/Shaders"));
    AddShaderSourceDirectoryMapping(TEXT("/Plugin/AlterMesh"), ShaderDir);
}

void FAlterMeshRenderingModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAlterMeshRenderingModule, AlterMeshRendering)