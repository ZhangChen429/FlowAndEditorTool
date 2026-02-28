// Copyright Epic Games, Inc. All Rights Reserved.

#include "Workspot.h"

#define LOCTEXT_NAMESPACE "FWorkspotModule"

DEFINE_LOG_CATEGORY(LogWorkspot);

void FWorkspotModule::StartupModule()
{
	UE_LOG(LogWorkspot, Log, TEXT("Workspot module started"));
}

void FWorkspotModule::ShutdownModule()
{
	UE_LOG(LogWorkspot, Log, TEXT("Workspot module shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWorkspotModule, Workspot)
