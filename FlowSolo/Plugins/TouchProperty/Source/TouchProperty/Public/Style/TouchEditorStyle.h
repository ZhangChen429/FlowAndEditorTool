#pragma once

class FTouchEditorStyle
{
public:
	static void Initialize();

	static void Shutdown();

	static FName GetStyleSetName();

	static  FName StyleSetName;

	static const ISlateStyle& Get() { return *StyleSet; }

	static	TSharedRef<FSlateStyleSet> Create();
private:
	static TSharedPtr<FSlateStyleSet> StyleSet;
	
};
