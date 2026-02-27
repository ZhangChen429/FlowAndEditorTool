// Copyright 2023 Aechmea

#include "AlterMeshBrowser.h"

#include "AlterMeshAsset.h"
#include "AlterMeshBrowserCommands.h"
#include "HttpModule.h"
#include "LevelEditor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Framework/Application/SWindowTitleBar.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformFileManager.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Styling/SlateIconFinder.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SSearchBox.h"

FAlterMeshBrowser::FAlterMeshBrowser()
{

}

TSharedPtr<FAlterMeshBrowser> FAlterMeshBrowser::MakeInstance()
{
	return MakeShareable(new FAlterMeshBrowser);
}

void FAlterMeshBrowser::AddEditorToolbarButton()
{
	if (GIsEditor)
	{
		// Setup Concert Toolbar
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

		// Register command list
		FAlterMeshBrowserCommands::Register();
		TSharedPtr<FUICommandList> CommandList = MakeShared<FUICommandList>();

		// Browser menu
		CommandList->MapAction(FAlterMeshBrowserCommands::Get().Browse,
			FExecuteAction::CreateRaw(this, &FAlterMeshBrowser::OpenBrowser)
		);

		ToolbarExtender = MakeShared<FExtender>();
		ToolbarExtender->AddToolBarExtension("Content", EExtensionHook::After, CommandList, FToolBarExtensionDelegate::CreateLambda([this, CommandList](FToolBarBuilder& ToolbarBuilder)
		{
			ToolbarBuilder.BeginSection("Browser");
			{
				ToolbarBuilder.AddToolBarButton
				(
					FAlterMeshBrowserCommands::Get().Browse
				);
			}
			ToolbarBuilder.EndSection();
		}));

		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FAlterMeshBrowser::RemoveEditorToolbarButton()
{
	if (GIsEditor)
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetToolBarExtensibilityManager()->RemoveExtender(ToolbarExtender);
		FAlterMeshBrowserCommands::Unregister();
		ToolbarExtender.Reset();
	}
}

void FAlterMeshBrowser::OpenBrowser()
{
	SAssignNew(BrowserWindow, SWindow)
		.IsPopupWindow(true)
		.CreateTitleBar(false)
		.Title(FText::FromString("AlterMesh Browser"))
		.MaxWidth(700)
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		.FocusWhenFirstShown(true)
		.SaneWindowPlacement(true);

	BrowserWindow->SetContent(CreateContent());

	FVector2D WindowSize = BrowserWindow->GetSizeInScreen();
	FVector2D DesiredPosition = FSlateApplication::Get().GetCursorPos();
	DesiredPosition.X -= FSlateApplication::Get().GetCursorSize().X;
	DesiredPosition.Y += FSlateApplication::Get().GetCursorSize().Y;
	DesiredPosition = FSlateApplication::Get().CalculateTooltipWindowPosition(FSlateRect(DesiredPosition.X, DesiredPosition.Y, DesiredPosition.X, DesiredPosition.Y), WindowSize, false);

	BrowserWindow->MoveWindowTo(DesiredPosition);

	TSharedPtr<SDockTab> LevelEditorTab = FModuleManager::Get().GetModuleChecked<FLevelEditorModule>("LevelEditor").GetLevelEditorTab();
	TSharedPtr<SWindow> ParentWindow = LevelEditorTab->GetParentWindow();

	FPopupTransitionEffect TransitionEffect(FPopupTransitionEffect::None);
	FSlateApplication::Get().PushMenu(ParentWindow.ToSharedRef(), FWidgetPath(), BrowserWindow.ToSharedRef(), DesiredPosition, TransitionEffect, true);

	RefreshDownloadState();
}

void FAlterMeshBrowser::RefreshDownloadState()
{
	const FSlateBrush* AlterMeshIcon = FSlateIconFinder::FindIconBrushForClass(UAlterMeshAsset::StaticClass());
	const FSlateBrush* DownloadIcon = FSlateIconFinder::FindIcon(FName("AlterMeshBrowser.Download")).GetIcon();

	for (TSharedPtr<FAlterMeshBrowserAsset> Asset : ListViewAssets)
	{
		// Check if asset already exists
		UAlterMeshAsset* AlterMeshAsset = Cast<UAlterMeshAsset>(FSoftObjectPath(Asset->Path).TryLoad());
		Asset->bDownloaded = !!AlterMeshAsset;

		if (Asset->DownloadImage.IsValid())
		{
			Asset->DownloadImage->SetImage(Asset->bDownloaded ? AlterMeshIcon : DownloadIcon);
			Asset->DownloadImage->Invalidate(EInvalidateWidgetReason::Paint);
		}
	}
}

void FAlterMeshBrowser::RequestFileList()
{
	static const FString ListFilesURL = TEXT("altermesh.com/listfiles");

	FHttpModule& HttpModule = FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");
	FHttpRequestRef HttpRequest = HttpModule.Get().CreateRequest();
	HttpRequest->SetURL(ListFilesURL);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FAlterMeshBrowser::OnReceiveFileList);
	HttpRequest->ProcessRequest();
}

void FAlterMeshBrowser::FilterAssetList()
{
}

void FAlterMeshBrowser::OnReceiveFileList(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	static const FString DownloadURL = TEXT("altermesh.com/download/");

	ListViewAssets.Empty();

	if (bConnectedSuccessfully && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()) && Response->GetContent().Num())
	{
		TArray<uint8> CharArray(Response->GetContent());
		CharArray.Add(0);
		FString FileListJson = FString(UTF8_TO_TCHAR(CharArray.GetData()));

		TSharedPtr<FJsonObject> Files = MakeShareable(new FJsonObject);
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileListJson);
		if (FJsonSerializer::Deserialize(JsonReader, Files))
		{
			for (TSharedPtr<FJsonValue> File : Files->GetArrayField(TEXT("Items")))
			{
				TSharedPtr<FAlterMeshBrowserAsset> NewAsset = MakeShareable(new FAlterMeshBrowserAsset);
				NewAsset->Name = FName(File->AsObject()->GetStringField(TEXT("Name")));
				NewAsset->DisplayName = FText::FromString(FPaths::GetBaseFilename(NewAsset->Name.ToString()));
				NewAsset->URL = DownloadURL + NewAsset->Name.ToString();
				NewAsset->Path = FPackageName::FilenameToLongPackageName(TEXT("/Game/AlterMesh/") + NewAsset->DisplayName.ToString());

				const TSharedPtr<FJsonObject>* Metadata;
				if (File->AsObject()->TryGetObjectField(TEXT("Metadata"), Metadata) && Metadata->IsValid())
				{
					FString Class = "";
					Metadata->Get()->TryGetStringField(TEXT("Class"), Class);
					NewAsset->Class = FName(Class);

					FString Dependencies = "";
					Metadata->Get()->TryGetStringField(TEXT("Dependencies"), Dependencies);
					TArray<FString> DependencyArray;
					Dependencies.ParseIntoArray(DependencyArray, TEXT(" "));

					for (const FString& Dependency : DependencyArray)
					{
						NewAsset->Dependencies.Add(FName(Dependency));
					}
				}

				if (NewAsset->Class == UAlterMeshAsset::StaticClass()->GetFName())
				{
					ListViewAssets.Add(NewAsset);
				}

				Assets.Add(NewAsset);
			}
		}
	}

	if (BrowserWindow.IsValid())
	{
		BrowserWindow->Invalidate(EInvalidateWidgetReason::Layout | EInvalidateWidgetReason::Paint);
	}

	RefreshDownloadState();
	FilterAssetList();
}

void FAlterMeshBrowser::OnAssetSelected(TSharedPtr<FAlterMeshBrowserAsset> Item, ESelectInfo::Type SelectType)
{
	if (SelectType == ESelectInfo::OnMouseClick)
	{
		if (Item.IsValid() && !Item->bDownloaded)
		{
			DownloadAsset(Item);

			for (const FName& Dependency : Item->Dependencies)
			{
				TSharedPtr<FAlterMeshBrowserAsset>* DependencyAsset = Assets.FindByPredicate([&Dependency](const TSharedPtr<FAlterMeshBrowserAsset> Item)
				{
					return Item->Name == Dependency;
				});

				if (DependencyAsset)
				{
					DownloadAsset(*DependencyAsset);
				}
			}
		}
	}
}

TSharedRef<SWidget> FAlterMeshBrowser::CreateContent()
{
	return
	SNew(SBorder)
	.BorderImage(FAppStyle::GetBrush("Menu.Background"))
	[
		SNew(SBox)
		.Padding(5.f)
		.WidthOverride(600.f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.Padding(4)
			.AutoHeight()
			[
				SNew(SSearchBox)
			]
			+SVerticalBox::Slot()
			.Padding(4)
			.AutoHeight()
			[
				CreateTags()
			]
			+SVerticalBox::Slot()
			.FillHeight(1)
			[
				SNew(SVerticalBox)

				+SVerticalBox::Slot()
				[
					CreateAssets(ListViewAssets)
				]

				/* todo categories
				+SVerticalBox::Slot()
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString("New"))
					]
					+SVerticalBox::Slot()
					[
						CreateAssets(NewAssets)
					]
				]

				+SVerticalBox::Slot()
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString("Hot"))
					]
					+SVerticalBox::Slot()
					[
						CreateAssets(HotAssets)
					]
				]
				*/
			]
		]
	];
}

TSharedRef<SWidget> FAlterMeshBrowser::CreateTags()
{
	return SNew(SListView<TSharedPtr<FText>>)
	.Orientation(EOrientation::Orient_Horizontal)
	.OnGenerateRow_Raw(this, &FAlterMeshBrowser::GenerateRowForTag)
	.ScrollbarVisibility(EVisibility::Hidden)
	.ListItemsSource(&Tags);
}

TSharedRef<SWidget> FAlterMeshBrowser::CreateAssets(const TArray<TSharedPtr<FAlterMeshBrowserAsset>>& FromAssets)
{
	return SAssignNew(ListView, SListView<TSharedPtr<FAlterMeshBrowserAsset>>)
	.Orientation(EOrientation::Orient_Horizontal)
	.SelectionMode(ESelectionMode::Single)
	.OnGenerateRow_Raw(this, &FAlterMeshBrowser::GenerateRowForAsset)
	.ScrollbarVisibility(EVisibility::Visible)
	.ListItemsSource(&FromAssets)
	.OnSelectionChanged_Raw(this, &FAlterMeshBrowser::OnAssetSelected)
	.ForceVolatile(true);
}

void FAlterMeshBrowser::DownloadAsset(TSharedPtr<FAlterMeshBrowserAsset> Asset)
{
	FHttpModule& HttpModule = FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");
	FHttpRequestRef HttpRequest = HttpModule.Get().CreateRequest();
	HttpRequest->SetURL(Asset->URL);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FAlterMeshBrowser::OnDownloadReady, Asset);
	HttpRequest->ProcessRequest();
}

void FAlterMeshBrowser::OnDownloadReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, TSharedPtr<FAlterMeshBrowserAsset> Asset)
{
	if (bConnectedSuccessfully && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()) && Asset)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		const FString AlterMeshDirectory = FPaths::ProjectContentDir() + TEXT("AlterMesh/");
		const FString PathOnDisk = FPaths::ConvertRelativePathToFull(AlterMeshDirectory + Asset->Name.ToString());
		PlatformFile.CreateDirectoryTree(*AlterMeshDirectory);
		IFileHandle* FileHandle = PlatformFile.OpenWrite(*PathOnDisk);
		if (FileHandle)
		{
			const TArray<uint8>& Content = Response->GetContent();
			FileHandle->Write(&Content[0], Content.Num());
			delete FileHandle;
		}

		RefreshDownloadState();
	}
}

TSharedRef<ITableRow> FAlterMeshBrowser::GenerateRowForTag(TSharedPtr<FText> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedRef<FText>>, OwnerTable)
	[
		SNew(SBorder)
		.Padding(4)
		[
			SNew(STextBlock).Text(*Item.Get())
		]
	];
}

TSharedRef<ITableRow> FAlterMeshBrowser::GenerateRowForAsset(TSharedPtr<FAlterMeshBrowserAsset> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const FSlateBrush* AlterMeshIcon = FSlateIconFinder::FindIconBrushForClass(UAlterMeshAsset::StaticClass());
	const FSlateBrush* DownloadIcon = FSlateIconFinder::FindIcon(FName("AlterMeshBrowser.Download")).GetIcon();

	return SNew(STableRow<TSharedRef<FText>>, OwnerTable)
	.OnDragDetected(FOnDragDetected::CreateLambda([this, Item](const FGeometry& Geometry, const FPointerEvent& PointerEvent)
	{
		if (Item->bDownloaded)
		{
			UAlterMeshAsset* AlterMeshAsset = Cast<UAlterMeshAsset>(FSoftObjectPath(Item->Path).TryLoad());
			if (AlterMeshAsset)
			{
				FAssetData Asset;
				Asset.AssetClass = Item->Class;
				Asset.ObjectPath = FName(AlterMeshAsset->GetPathName());
				Asset.AssetName = FName(Item->DisplayName.ToString());
				Asset.PackageName = FName(FPackageName::FilenameToLongPackageName(Asset.ObjectPath.ToString()));

				return FReply::Handled().BeginDragDrop(FAssetDragDropOp::New(Asset));
			}
		}

		return FReply::Unhandled();
	}))
	.Visibility(EVisibility::Visible)
	[
		SNew(SBox)
		.WidthOverride(150.f)
		.HeightOverride(150.f)
		.Padding(6.f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
			[
				SNew(STextBlock).Text(Item.Get()->DisplayName)
			]
			+SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SNew(SOverlay)
				+SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(32.f)
					.HeightOverride(32.f)
					[
						SAssignNew(Item->DownloadImage, SImage)
						.Image(Item->bDownloaded ? AlterMeshIcon : DownloadIcon)
						.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 1.0f))
					]
				]
			]
		]
	];
}
