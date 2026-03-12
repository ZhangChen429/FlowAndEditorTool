// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AIChatFileUtils.generated.h"

/**
 * 文件信息结构
 */
USTRUCT(BlueprintType)
struct FAIChatFileInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString FileName;

	UPROPERTY(BlueprintReadWrite)
	FString FullPath;

	UPROPERTY(BlueprintReadWrite)
	bool bIsDirectory;

	UPROPERTY(BlueprintReadWrite)
	int64 FileSize;
};

/**
 * AI Chat 文件工具类
 */
UCLASS()
class AICHATSUPPORTEDITOR_API UAIChatFileUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "AI Chat | File Utils")
	static bool ListFilesInFolder(const FString& FolderPath, TArray<FAIChatFileInfo>& OutFiles);
	
	UFUNCTION(BlueprintCallable, Category = "AI Chat | File Utils")
	static FString FormatFileListForAI(const TArray<FAIChatFileInfo>& Files, const FString& FolderPath);
	
	UFUNCTION(BlueprintCallable, Category = "AI Chat | File Utils")
	static FString ConvertToPhysicalPath(const FString& VirtualPath);
	
	UFUNCTION(BlueprintCallable, Category = "AI Chat | File Utils")
	static TArray<FString> GetCommonUEPaths();
	
	static bool FindProjectFilesByGlob(const FString& GlobPattern, int32 MaxResults, TArray<FString>& OutFiles, FString& OutError);

	/** Read: 读取文本文件指定行区间（1-based） */
	static bool ReadProjectTextFile(const FString& InPath, int32 StartLine, int32 MaxLines, FString& OutText, FString& OutError);

	/** Grep: 在项目文件中按关键字搜索（大小写不敏感） */
	static bool GrepProjectFiles(const FString& Keyword, const FString& FileGlob, int32 MaxHits, FString& OutText, FString& OutError);
	
private:
	static bool ResolveAndValidatePathInProject(const FString& InPath, FString& OutFullPath, bool bMustBeFile);
	static bool IsPathUnderProject(const FString& FullPath);

	
};
