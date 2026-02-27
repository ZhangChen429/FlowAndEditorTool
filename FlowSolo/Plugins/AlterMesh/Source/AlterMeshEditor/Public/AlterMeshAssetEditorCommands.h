// Copyright 2023 Aechmea

#pragma once

#include "Framework/Commands/Commands.h"
#include "Templates/SharedPointer.h"

class FAlterMeshAssetEditorCommands : public TCommands<FAlterMeshAssetEditorCommands>
{
public:
	FAlterMeshAssetEditorCommands()
	: TCommands<FAlterMeshAssetEditorCommands>(
	TEXT("AlterMeshEditor"),
	NSLOCTEXT("Contexts", "AlterMeshEditor", "AlterMesh Editor"),
	NAME_None,
	FName("AlterMesh"))
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenParent;
	TSharedPtr<FUICommandInfo> ReimportParams;
	TSharedPtr<FUICommandInfo> Reimport;
	TSharedPtr<FUICommandInfo> Refresh;
	TSharedPtr<FUICommandInfo> Open;
	TSharedPtr<FUICommandInfo> ShowUV0;
	TSharedPtr<FUICommandInfo> ShowUV1;
	TSharedPtr<FUICommandInfo> ShowUV2;
	TSharedPtr<FUICommandInfo> ShowUV3;
	TSharedPtr<FUICommandInfo> ShowColors;
	TSharedPtr<FUICommandInfo> ShowAlpha;
};
