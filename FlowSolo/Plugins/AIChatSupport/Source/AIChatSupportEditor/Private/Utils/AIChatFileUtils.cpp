// Copyright Epic Games, Inc. All Rights Reserved.

#include "Utils/AIChatFileUtils.h"
#include "AIChatSupportEditor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UObject/ConstructorHelpers.h"

bool UAIChatFileUtils::ListFilesInFolder(const FString& FolderPath, TArray<FAIChatFileInfo>& OutFiles)
{
	OutFiles.Empty();

	// 检查是否是UE虚拟路径（以 /Game/ 或 /Engine/ 开头）
	if (FolderPath.StartsWith(TEXT("/Game/")) || FolderPath.StartsWith(TEXT("/Engine/")))
	{
		// 使用AssetRegistry列举UE资产
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		// 获取该路径下的所有资产
		TArray<FAssetData> AssetDataList;
		AssetRegistry.GetAssetsByPath(FName(*FolderPath), AssetDataList, false);

		// 添加资产文件
		for (const FAssetData& AssetData : AssetDataList)
		{
			FAIChatFileInfo FileInfo;
			FileInfo.FullPath = AssetData.GetObjectPathString();
			FileInfo.FileName = AssetData.AssetName.ToString();
			FileInfo.bIsDirectory = false;
			FileInfo.FileSize = 0; // UE资产无法直接获取大小

			OutFiles.Add(FileInfo);
		}

		// 获取子文件夹
		TArray<FString> SubPaths;
		AssetRegistry.GetSubPaths(FolderPath, SubPaths, false);

		for (const FString& SubPath : SubPaths)
		{
			FAIChatFileInfo FolderInfo;
			FolderInfo.FullPath = SubPath;

			// 提取文件夹名
			int32 LastSlashIndex;
			if (SubPath.FindLastChar('/', LastSlashIndex))
			{
				FolderInfo.FileName = SubPath.RightChop(LastSlashIndex + 1);
			}
			else
			{
				FolderInfo.FileName = SubPath;
			}

			FolderInfo.bIsDirectory = true;
			FolderInfo.FileSize = 0;

			OutFiles.Add(FolderInfo);
		}

		UE_LOG(LogAIChat, Log, TEXT("[File Utils] Listed %d items in folder: %s"), OutFiles.Num(), *FolderPath);
		return true;
	}
	else
	{
		// 物理路径处理
		FString PhysicalPath = FolderPath;

		// 如果是相对路径，转换为绝对路径
		if (FPaths::IsRelative(PhysicalPath))
		{
			PhysicalPath = FPaths::ConvertRelativePathToFull(PhysicalPath);
		}

		// 检查目录是否存在
		if (!IFileManager::Get().DirectoryExists(*PhysicalPath))
		{
			UE_LOG(LogAIChat, Error, TEXT("[File Utils] Directory does not exist: %s"), *PhysicalPath);
			return false;
		}

		// 列举文件和文件夹
		TArray<FString> FoundFiles;
		IFileManager::Get().FindFiles(FoundFiles, *FPaths::Combine(PhysicalPath, TEXT("*")), true, true);

		for (const FString& FileName : FoundFiles)
		{
			FAIChatFileInfo FileInfo;
			FileInfo.FileName = FileName;
			FileInfo.FullPath = FPaths::Combine(PhysicalPath, FileName);
			FileInfo.bIsDirectory = IFileManager::Get().DirectoryExists(*FileInfo.FullPath);

			if (!FileInfo.bIsDirectory)
			{
				FileInfo.FileSize = IFileManager::Get().FileSize(*FileInfo.FullPath);
			}
			else
			{
				FileInfo.FileSize = 0;
			}

			OutFiles.Add(FileInfo);
		}

		UE_LOG(LogAIChat, Log, TEXT("[File Utils] Listed %d items in folder: %s"), OutFiles.Num(), *PhysicalPath);
		return true;
	}
}

FString UAIChatFileUtils::FormatFileListForAI(const TArray<FAIChatFileInfo>& Files, const FString& FolderPath)
{
	FString Result = FString::Printf(TEXT("📁 Files in folder: %s\n"), *FolderPath);
	Result += TEXT("=====================================\n\n");

	if (Files.Num() == 0)
	{
		Result += TEXT("(Empty folder)\n");
		return Result;
	}

	// 分离文件夹和文件
	TArray<FAIChatFileInfo> Folders;
	TArray<FAIChatFileInfo> RegularFiles;

	for (const FAIChatFileInfo& File : Files)
	{
		if (File.bIsDirectory)
		{
			Folders.Add(File);
		}
		else
		{
			RegularFiles.Add(File);
		}
	}

	// 先显示文件夹
	if (Folders.Num() > 0)
	{
		Result += TEXT("📂 Folders:\n");
		for (const FAIChatFileInfo& Folder : Folders)
		{
			Result += FString::Printf(TEXT("  📁 %s\n"), *Folder.FileName);
		}
		Result += TEXT("\n");
	}

	// 再显示文件
	if (RegularFiles.Num() > 0)
	{
		Result += TEXT("📄 Files:\n");
		for (const FAIChatFileInfo& File : RegularFiles)
		{
			FString SizeStr;
			if (File.FileSize > 0)
			{
				// 格式化文件大小
				if (File.FileSize < 1024)
				{
					SizeStr = FString::Printf(TEXT("%lld B"), File.FileSize);
				}
				else if (File.FileSize < 1024 * 1024)
				{
					SizeStr = FString::Printf(TEXT("%.2f KB"), File.FileSize / 1024.0);
				}
				else
				{
					SizeStr = FString::Printf(TEXT("%.2f MB"), File.FileSize / (1024.0 * 1024.0));
				}
				Result += FString::Printf(TEXT("  📄 %s (%s)\n"), *File.FileName, *SizeStr);
			}
			else
			{
				Result += FString::Printf(TEXT("  📄 %s\n"), *File.FileName);
			}
		}
	}

	Result += FString::Printf(TEXT("\nTotal: %d folders, %d files\n"), Folders.Num(), RegularFiles.Num());
	return Result;
}

FString UAIChatFileUtils::ConvertToPhysicalPath(const FString& VirtualPath)
{
	// 处理 /Game/ 路径
	if (VirtualPath.StartsWith(TEXT("/Game/")))
	{
		FString RelativePath = VirtualPath.RightChop(6); // 移除 "/Game/"
		FString ContentDir = FPaths::ProjectContentDir();
		return FPaths::Combine(ContentDir, RelativePath);
	}

	// 处理 /Engine/ 路径
	if (VirtualPath.StartsWith(TEXT("/Engine/")))
	{
		FString RelativePath = VirtualPath.RightChop(8); // 移除 "/Engine/"
		FString EngineContentDir = FPaths::EngineContentDir();
		return FPaths::Combine(EngineContentDir, RelativePath);
	}

	// 处理 /Plugins/ 路径
	if (VirtualPath.StartsWith(TEXT("/Plugins/")))
	{
		FString RelativePath = VirtualPath.RightChop(9); // 移除 "/Plugins/"
		FString PluginsDir = FPaths::ProjectPluginsDir();
		return FPaths::Combine(PluginsDir, RelativePath);
	}

	// 如果已经是物理路径，直接返回
	return VirtualPath;
}

TArray<FString> UAIChatFileUtils::GetCommonUEPaths()
{
	TArray<FString> CommonPaths;

	// 项目常用路径
	CommonPaths.Add(TEXT("/Game/"));
	CommonPaths.Add(TEXT("/Game/Blueprints"));
	CommonPaths.Add(TEXT("/Game/Maps"));
	CommonPaths.Add(TEXT("/Game/Materials"));
	CommonPaths.Add(TEXT("/Game/Textures"));
	CommonPaths.Add(TEXT("/Game/Meshes"));
	CommonPaths.Add(TEXT("/Game/Audio"));
	CommonPaths.Add(TEXT("/Game/UI"));
	CommonPaths.Add(TEXT("/Game/Characters"));
	CommonPaths.Add(TEXT("/Game/Animations"));

	// 引擎路径
	CommonPaths.Add(TEXT("/Engine/"));
	CommonPaths.Add(TEXT("/Engine/BasicShapes"));
	CommonPaths.Add(TEXT("/Engine/EngineMaterials"));

	return CommonPaths;
}
