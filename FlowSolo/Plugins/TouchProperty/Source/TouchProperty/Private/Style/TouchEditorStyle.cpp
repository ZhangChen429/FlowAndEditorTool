
#include "Style/TouchEditorStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FTouchEditorStyle::StyleSet = nullptr;
FName FTouchEditorStyle::StyleSetName=TEXT("EpisodeShowcase");

void FTouchEditorStyle::Initialize()
{
	if (!StyleSet.IsValid())
	{
		StyleSet = MakeShared<FSlateStyleSet>(StyleSetName);
		
		StyleSet=Create();
		
		FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
	}
}

FName FTouchEditorStyle::GetStyleSetName()
{
	return FName(StyleSetName);
}

TSharedRef<FSlateStyleSet> FTouchEditorStyle::Create()
{
	const FVector2D Icon20x20(20.0f, 20.0f);
	FString PluginResourceDir = IPluginManager::Get().FindPlugin("TouchProperty")->GetBaseDir() / TEXT("Resources");
	
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("TouchProperty")->GetBaseDir() / TEXT("Resources"));
	
	
	const FString IconPath = PluginResourceDir / TEXT("ShowCase10.svg");
	Style->Set("EpisodeShowcase.ShowcaseCustomInfo", new  FSlateVectorImageBrush  (Style->RootToContentDir( TEXT("ShowCase10.svg") ), Icon20x20 ));
	Style->Set("ShowCaseIcon", new  FSlateVectorImageBrush  (Style->RootToContentDir( TEXT("ShowCase10.svg") ), Icon20x20 ));
	
	return Style;
}

void FTouchEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
	ensure(StyleSet.IsUnique());
	StyleSet.Reset();
}

