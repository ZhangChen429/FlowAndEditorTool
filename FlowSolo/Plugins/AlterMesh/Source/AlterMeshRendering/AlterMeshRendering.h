// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FAlterMeshRenderingModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

};
