// Copyright Epic Games, Inc. All Rights Reserved.

#include "TouchProperty.h"

#include "AssetTypeActions_Base.h"
#include "Command/TouchCommand.h"
#include "FactoryAndMode/TouchCameraTypeAction.h"

#define LOCTEXT_NAMESPACE "FTouchPropertyModule"

EAssetTypeCategories::Type FTouchPropertyModule::TouchCameraAssetCategory;

void FTouchPropertyModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Register custom asset category
	TouchCameraAssetCategory = AssetTools.RegisterAdvancedAssetCategory(
		FName(TEXT("TouchCustom")),
		FText::FromString("TouchCamera")
	);

	//定义资产的分类名
	TSharedPtr<FTouchCameraTypeAction> actionType = MakeShareable(new FTouchCameraTypeAction());
	AssetTools.RegisterAssetTypeActions(actionType.ToSharedRef());

	ShowCaseCommands = MakeShareable(new FUICommandList);
	FTouchCommand::Register();
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FTouchPropertyModule::RegisterMenus));
}

void FTouchPropertyModule::ShutdownModule()
{

}
void FTouchPropertyModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			{
				FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FTouchCommand::Get().ShowcaseCustomInfo));
				Entry.SetCommandList(ShowCaseCommands);
			}
		}
	}
	
	if (FAssetToolsModule::IsModuleLoaded())
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
        
		for (auto& AssetTypeAction : RegisteredAssetTypeActions)
		{
			if (AssetTypeAction.IsValid())
			{
				AssetTools.UnregisterAssetTypeActions(AssetTypeAction.ToSharedRef());
			}
		}
	}
    
	RegisteredAssetTypeActions.Empty();
	
}
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTouchPropertyModule, TouchProperty)
