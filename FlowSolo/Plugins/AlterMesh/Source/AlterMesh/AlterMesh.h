// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"
#include "Modules/ModuleManager.h"

static TAutoConsoleVariable<int32> CVarAlterMeshDebugProcessOutput(
	TEXT("AlterMesh.DebugProcessOutput"),
	0,
	TEXT("Enable printing blender's output to the log"));

class FAlterMeshModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
