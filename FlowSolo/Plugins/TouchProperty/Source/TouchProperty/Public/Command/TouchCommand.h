#pragma once
#include "Style/TouchEditorStyle.h"

class FTouchCommand:public TCommands<FTouchCommand>
{
public:
	FTouchCommand():
	TCommands<FTouchCommand>(
				TEXT("EpisodeShowcase"),
				NSLOCTEXT("FShowcase","K1","FShowcaseCommand"),
				NAME_None,
				FTouchEditorStyle::GetStyleSetName()
				)
	{
		
	}
	virtual void RegisterCommands() override;
	TSharedPtr<FUICommandInfo>ShowcaseCustomInfo;
};
