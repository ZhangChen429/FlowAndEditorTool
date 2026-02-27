// Copyright 2023 Aechmea

#pragma once
#include "Framework/Commands/Commands.h"

class FAlterMeshBrowserCommands : public TCommands<FAlterMeshBrowserCommands>
{
public:
	FAlterMeshBrowserCommands()
	: TCommands<FAlterMeshBrowserCommands>(
	TEXT("AlterMeshBrowser"),
	NSLOCTEXT("Contexts", "AlterMeshBrowser", "AlterMesh Browser"),
	NAME_None,
	FName("AlterMesh"))
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> Browse;
};
