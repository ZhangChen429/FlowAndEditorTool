
#include "Slate\TouchCameraViewer_LevelEditorViewportClient.h"

#include "Engine\StaticMeshActor.h"
#include "TouchAsset\TouchCameraAsset.h"


FTouchCameraViewer_LevelEditorViewportClient::FTouchCameraViewer_LevelEditorViewportClient(
	FEditorModeTools* InModeTools, FPreviewScene* InPreviewScene,
	const TWeakPtr<SEditorViewport>& InEditorViewportWidget, const TWeakObjectPtr<UTouchCameraAsset> InEditingObject): FEditorViewportClient(InModeTools, InPreviewScene, InEditorViewportWidget)
{
	if (!EditingObject.IsValid())
	{
		EditingObject=InEditingObject;
	}
	
	if (!ChangeAssetDelegateHandle.IsValid())
	{
		SetEditingObject(EditingObject.Pin()->TouchCameraStaticMesh);
		OnAssetChanged();
		ChangeAssetDelegateHandle=EditingObject.Pin()->OnChangeAssetInPreview.AddRaw(this,&FTouchCameraViewer_LevelEditorViewportClient::OnEditorObjectChange);
		
	}
	SetRealtime(true);
}

FTouchCameraViewer_LevelEditorViewportClient::~FTouchCameraViewer_LevelEditorViewportClient()
{
	if (EditingObject.IsValid() && ChangeAssetDelegateHandle.IsValid()) 
	{
		EditingObject.Pin()->OnChangeAssetInPreview.Remove(ChangeAssetDelegateHandle);
	}

	
	UE_LOG(LogTemp, Warning, TEXT("ViewportClient Destroyed!"));
	MeshActor.Reset();
	ChangeAssetDelegateHandle.Reset();
}
void FTouchCameraViewer_LevelEditorViewportClient::SetEditingObject(UStaticMesh* PreviewObject)
{
	EditingPreviewMesh = PreviewObject;
}


void FTouchCameraViewer_LevelEditorViewportClient::OnAssetChanged()
{
	if (EditingPreviewMesh.IsValid())
	{
		if (!MeshActor.IsValid())
		{
			MeshActor=MakeWeakObjectPtr(PreviewScene->GetWorld()->SpawnActor<AStaticMeshActor>(FVector::Zero(), FRotator::ZeroRotator))	;
			MeshActor.Pin()->GetStaticMeshComponent()->SetStaticMesh(EditingPreviewMesh.Pin().Get());
		}
		else
		{
			MeshActor.Pin()->GetStaticMeshComponent()->SetStaticMesh(EditingPreviewMesh.Pin().Get());
		}
		return;
	}
	
	MeshActor.Reset();
}

void FTouchCameraViewer_LevelEditorViewportClient::OnEditorObjectChange(UStaticMesh* NewMesh)
{
	UE_LOG(LogTemp, Log, TEXT("NewMesh  Name %s,  %p"),*NewMesh->GetName(),this);
	SetEditingObject(NewMesh);
	OnAssetChanged();
}

void FTouchCameraViewer_LevelEditorViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);
	PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
}

void FTouchCameraViewer_LevelEditorViewportClient::SetPreviewScene(FPreviewScene* NewPreviewScene)
{
	PreviewScene = NewPreviewScene;
}
