// Copyright 2023 Aechmea

#include "AlterMeshSettings.h"

#include "AlterMeshActor.h"
#include "EngineUtils.h"
#include "Widgets/Notifications/SNotificationList.h"

#if WITH_EDITOR
#include "Framework/Notifications/NotificationManager.h"
#endif

UAlterMeshSettings::UAlterMeshSettings()
{
#if WITH_EDITOR
	OnSettingChanged().AddUObject(this, &UAlterMeshSettings::OnChangedPath);
#endif
}

#if WITH_EDITOR
void UAlterMeshSettings::OnChangedPath(UObject* Object, FPropertyChangedEvent& Event)
{
	if (Event.Property->GetFName() == FName("FilePath"))
	{
		if (FPaths::GetCleanFilename(ExecutablePath.FilePath) != TEXT("blender.exe")
			|| !FPaths::FileExists(ExecutablePath.FilePath))
		{
			ExecutablePath.FilePath = TEXT("");
			FNotificationInfo Info(FText::FromString(TEXT("Invalid executable path")));
			Info.Image = FCoreStyle::Get().GetBrush(TEXT("MessageLog.Warning"));
			Info.ExpireDuration = 5.f;
			Info.bUseLargeFont = true;
			FSlateNotificationManager::Get().AddNotification(Info);
		}
		else
		{
			for (const TWeakObjectPtr<AActor> Actor : FActorRange(GWorld))
			{
				if (Cast<AAlterMeshActor>(Actor) && Cast<AAlterMeshActor>(Actor)->GetPreview())
				{
					Cast<AAlterMeshActor>(Actor)->RefreshAsync();
				}
			}
		}
	}
}
#endif
