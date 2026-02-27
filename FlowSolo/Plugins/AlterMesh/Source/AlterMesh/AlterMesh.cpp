// Copyright 2023 Aechmea

#include "AlterMesh.h"

IMPLEMENT_MODULE(FAlterMeshModule, AlterMesh)void FAlterMeshModule::StartupModule()
{
	IModuleInterface::StartupModule();

#if ALTERMESH_FREE
#if !WITH_EDITOR
	UE_LOG(LogTemp, Warning, TEXT("AlterMesh Non-commercial version cannot be distributed. Please read EULA at http://altermesh.com/eula"))
	EMIT_CUSTOM_WARNING("AlterMesh Non-commercial version cannot be distributed. Please read EULA at http://altermesh.com/eula");
#endif
#endif
}

void FAlterMeshModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
}
