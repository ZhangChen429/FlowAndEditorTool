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

bool UAIChatFileUtils::FindProjectFilesByGlob(const FString& GlobPattern, int32 MaxResults, TArray<FString>& OutFiles,
	FString& OutError)
{
	OutFiles.Empty();
	OutError.Empty();

	const FString ProjectRoot = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	FString Pattern = GlobPattern.IsEmpty() ? TEXT("*.cpp") : GlobPattern;
	int32 Limit = FMath::Clamp(MaxResults, 1, 500);

	TArray<FString> Found;
	IFileManager::Get().FindFilesRecursive(Found, *ProjectRoot, *Pattern, true, false, true);

	// 过滤中间目录
	for (const FString& File : Found)
	{
		FString Std = File;
		FPaths::MakeStandardFilename(Std);

		if (Std.Contains(TEXT("/Intermediate/")) || Std.Contains(TEXT("/Binaries/")) || Std.Contains(TEXT("/Saved/")))
		{
			continue;
		}

		OutFiles.Add(Std);
		if (OutFiles.Num() >= Limit) break;
	}

	if (OutFiles.Num() == 0)
	{
		OutError = FString::Printf(TEXT("No files found for pattern: %s"), *Pattern);
		return false;
	}
	return true;
}

bool UAIChatFileUtils::ReadProjectTextFile(const FString& InPath, int32 StartLine, int32 MaxLines, FString& OutText,
	FString& OutError)
{
	OutText.Empty();
	OutError.Empty();

	FString FullPath;
	if (!ResolveAndValidatePathInProject(InPath, FullPath, true))
	{
		OutError = TEXT("Path invalid / out of project / file not exists");
		return false;
	}

	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *FullPath))
	{
		OutError = TEXT("Failed to read file as text");
		return false;
	}

	const int32 Begin = FMath::Max(1, StartLine);
	const int32 Count = FMath::Clamp(MaxLines, 1, 800);
	const int32 End = FMath::Min(Begin + Count - 1, Lines.Num());

	OutText += FString::Printf(TEXT("FILE: %s\nRANGE: %d-%d / %d\n\n"), *FullPath, Begin, End, Lines.Num());
	for (int32 L = Begin; L <= End; ++L)
	{
		OutText += FString::Printf(TEXT("%5d | %s\n"), L, *Lines[L - 1]);
	}

	if (End < Lines.Num())
	{
		OutText += TEXT("\n...TRUNCATED...\n");
	}
	return true;
}

bool UAIChatFileUtils::GrepProjectFiles(const FString& Keyword, const FString& FileGlob, int32 MaxHits,
	FString& OutText, FString& OutError)
{
	OutText.Empty();
	OutError.Empty();

	if (Keyword.IsEmpty())
	{
		OutError = TEXT("Keyword is empty");
		return false;
	}

	TArray<FString> Files;
	FString FindErr;
	if (!FindProjectFilesByGlob(FileGlob.IsEmpty() ? TEXT("*.cpp") : FileGlob, 1000, Files, FindErr))
	{
		OutError = FindErr;
		return false;
	}

	const int32 HitLimit = FMath::Clamp(MaxHits, 1, 500);
	int32 Hits = 0;

	OutText += FString::Printf(TEXT("GREP keyword='%s' glob='%s'\n\n"), *Keyword, *(FileGlob.IsEmpty() ? TEXT("*.cpp") : FileGlob));

	for (const FString& File : Files)
	{
		if (Hits >= HitLimit) break;

		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *File)) continue;

		for (int32 i = 0; i < Lines.Num(); ++i)
		{
			if (Lines[i].Contains(Keyword, ESearchCase::IgnoreCase))
			{
				OutText += FString::Printf(TEXT("%s:%d | %s\n"), *File, i + 1, *Lines[i]);
				++Hits;
				if (Hits >= HitLimit) break;
			}
		}
	}

	if (Hits == 0)
	{
		OutError = TEXT("No matches");
		return false;
	}

	if (Hits >= HitLimit)
	{
		OutText += TEXT("\n...HIT LIMIT REACHED...\n");
	}
	return true;
}

bool UAIChatFileUtils::ResolveAndValidatePathInProject(const FString& InPath, FString& OutFullPath, bool bMustBeFile)
{
	OutFullPath = InPath;

	// 支持 /Game/ /Engine/ /Plugins/ 虚拟路径
	if (InPath.StartsWith(TEXT("/")))
	{
		OutFullPath = ConvertToPhysicalPath(InPath);
	}

	if (FPaths::IsRelative(OutFullPath))
	{
		OutFullPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), OutFullPath);
	}
	else
	{
		OutFullPath = FPaths::ConvertRelativePathToFull(OutFullPath);
	}

	FPaths::CollapseRelativeDirectories(OutFullPath);
	FPaths::MakeStandardFilename(OutFullPath);

	if (!IsPathUnderProject(OutFullPath))
	{
		return false;
	}

	if (bMustBeFile)
	{
		return IFileManager::Get().FileExists(*OutFullPath);
	}

	return IFileManager::Get().DirectoryExists(*OutFullPath) || IFileManager::Get().FileExists(*OutFullPath);
}

bool UAIChatFileUtils::IsPathUnderProject(const FString& FullPath)
{
	FString StdFull = FullPath;
	FPaths::MakeStandardFilename(StdFull);

	FString ProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	FPaths::MakeStandardFilename(ProjectDir);

	return StdFull.StartsWith(ProjectDir);
}
