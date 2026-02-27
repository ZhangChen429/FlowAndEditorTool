// Copyright 2023 Aechmea

#include "AlterMeshAssetEditorCommands.h"

#define LOCTEXT_NAMESPACE "AlterMeshEditorCommands"

void FAlterMeshAssetEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenParent, "Open Parent", "Open the parent asset this instance was created from", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ReimportParams, "Reimport Params", "Bring in new params, does not revert changed params", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(Reimport, "Reimport", "Reimport the whole asset, will revert everything to defaults", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(Refresh, "Refresh", "Refreshes the viewport, does not refresh placed actors", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(Open, "Open", "Open the file in Blender", EUserInterfaceActionType::Button, FInputChord());
	
	UI_COMMAND(ShowUV0, "Show UV0", "Show Mesh UV0", EUserInterfaceActionType::Check, FInputChord());
	UI_COMMAND(ShowUV1, "Show UV1", "Show Mesh UV1", EUserInterfaceActionType::Check, FInputChord());
	UI_COMMAND(ShowUV2, "Show UV2", "Show Mesh UV2", EUserInterfaceActionType::Check, FInputChord());
	UI_COMMAND(ShowUV3, "Show UV3", "Show Mesh UV3", EUserInterfaceActionType::Check, FInputChord());
	UI_COMMAND(ShowColors, "Show Colors", "Show Vertex Colors RGB", EUserInterfaceActionType::Check, FInputChord());
	UI_COMMAND(ShowAlpha, "Show Alpha", "Show Vertex Colors Alpha Channel", EUserInterfaceActionType::Check, FInputChord());
}

#undef LOCTEXT_NAMESPACE