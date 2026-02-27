// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "AlterMeshAssetTypeActions.h"
#include "Interfaces/IHttpRequest.h"
#include "AlterMeshBrowser.generated.h"

struct FAlterMeshBrowserAsset
{
public:

	FName Name;
	FText DisplayName;
	FString URL;
	TArray<FName> Dependencies;
	TArray<FText> Tags;
	FName Class;
	FString Path;

	bool bDownloaded = false;
	TSharedPtr<SImage> DownloadImage = nullptr;
};

USTRUCT()
struct FAlterMeshBrowser
{
	GENERATED_BODY()
public:

	FAlterMeshBrowser();

	TSharedPtr<FExtender> ToolbarExtender;
	int32 DistanceDragged;
	static TSharedPtr<FAlterMeshBrowser> MakeInstance();

	void AddEditorToolbarButton();
	void RemoveEditorToolbarButton();
	void OpenBrowser();
	void RefreshDownloadState();
	void RequestFileList();
	void FilterAssetList();
	void OnReceiveFileList(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnAssetSelected(TSharedPtr<FAlterMeshBrowserAsset> Item, ESelectInfo::Type SelectType);

	TSharedRef<SWidget> CreateContent();
	TSharedRef<SWidget> CreateTags();
	TSharedRef<SWidget> CreateAssets(const TArray<TSharedPtr<FAlterMeshBrowserAsset>>& FromAssets);

	// Download files to disk
	void DownloadAsset(TSharedPtr<FAlterMeshBrowserAsset> Asset);
	void OnDownloadReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, TSharedPtr<FAlterMeshBrowserAsset> Asset);

private:

	TSharedPtr<SWindow> BrowserWindow;
	TSharedPtr<SWidget> ListView;

	TArray<TSharedPtr<FText>> Tags;
	TArray<TSharedPtr<FAlterMeshBrowserAsset>> ListViewAssets;
	TArray<TSharedPtr<FAlterMeshBrowserAsset>> Assets;

	/*
	TArray<TSharedPtr<FAlterMeshBrowserAsset>> TagAssets;
	TArray<TSharedPtr<FAlterMeshBrowserAsset>> HotAssets;
	TArray<TSharedPtr<FAlterMeshBrowserAsset>> NewAssets;
	*/

	TSharedRef<ITableRow> GenerateRowForTag(TSharedPtr<FText> Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> GenerateRowForAsset(TSharedPtr<FAlterMeshBrowserAsset> Item, const TSharedRef<STableViewBase>& OwnerTable);
};
