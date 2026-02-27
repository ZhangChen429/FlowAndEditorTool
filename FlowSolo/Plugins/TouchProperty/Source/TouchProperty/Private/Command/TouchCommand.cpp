#include "Command/TouchCommand.h"
#define LOCTEXT_NAMESPACE "ShowcaseCommands"
void FTouchCommand::RegisterCommands()
{
	UI_COMMAND(ShowcaseCustomInfo,"EpisodeShowcase","This is Episode Showcase",EUserInterfaceActionType::Button,FInputGesture())
	
	const FBindingContext BindingContext =FTouchCommand::Get().GetContext();
}
# undef LOCTEXT_NAMESPACE