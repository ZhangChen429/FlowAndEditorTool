// Copyright 2023 Aechmea

#include "AlterMeshBrowserCommands.h"

#define LOCTEXT_NAMESPACE "AlterMeshBrowserCommands"

void FAlterMeshBrowserCommands::RegisterCommands()
{
	UI_COMMAND(Browse, "Browse", "Opens the online browser", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE