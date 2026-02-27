// Copyright Epic Games, Inc. All Rights Reserved.

#include "Settings/AIChatSupportSettings.h"

#define LOCTEXT_NAMESPACE "AIChatSupportSettings"

UAIChatSupportSettings::UAIChatSupportSettings()
{
}

FText UAIChatSupportSettings::GetSectionText() const
{
	return LOCTEXT("SectionText", "AI Chat Support");
}

#undef LOCTEXT_NAMESPACE
