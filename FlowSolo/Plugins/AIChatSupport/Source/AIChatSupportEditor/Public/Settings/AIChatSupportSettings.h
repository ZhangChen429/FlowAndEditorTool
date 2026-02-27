// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Data/AIChatStructs.h"
#include "AIChatSupportSettings.generated.h"

/**
 * AI Chat Support 中转站默认配置
 * Editor Preferences → Plugins → AI Chat Support
 */
UCLASS(Config = EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "AI Chat Support"))
class AICHATSUPPORTEDITOR_API UAIChatSupportSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UAIChatSupportSettings();
	static UAIChatSupportSettings* Get() { return GetMutableDefault<UAIChatSupportSettings>(); }

	UPROPERTY(Config, EditAnywhere, Category = "Custom URL", meta = (DisplayName = "Use Custom URL (中转站)"))
	bool bUseCustomURL = false;

	UPROPERTY(Config, EditAnywhere, Category = "Custom URL", meta = (DisplayName = "API URL", EditCondition = "bUseCustomURL"))
	FString CustomAPIUrl;

	UPROPERTY(Config, EditAnywhere, Category = "Custom URL", meta = (DisplayName = "API Key", EditCondition = "bUseCustomURL"))
	FString CustomAPIKey;

	UPROPERTY(Config, EditAnywhere, Category = "Custom URL", meta = (DisplayName = "Model Name", EditCondition = "bUseCustomURL"))
	FString CustomModelName;

	UPROPERTY(Config, EditAnywhere, Category = "Custom URL", meta = (DisplayName = "API Protocol", EditCondition = "bUseCustomURL"))
	EAPIProtocol CustomProtocol = EAPIProtocol::Claude;

	virtual FName GetCategoryName() const override { return FName("Plugins"); }
	virtual FText GetSectionText() const override;
};
