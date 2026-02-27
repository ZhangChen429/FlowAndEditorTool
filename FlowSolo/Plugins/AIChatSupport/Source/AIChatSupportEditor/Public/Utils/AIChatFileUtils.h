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
	/**
	 * 列出指定文件夹下的所有文件和子文件夹（不递归）
	 * @param FolderPath - 文件夹路径（支持 UE 虚拟路径如 /Game/ 或物理路径）
	 * @param OutFiles - 输出文件列表
	 * @return 是否成功
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Chat | File Utils")
	static bool ListFilesInFolder(const FString& FolderPath, TArray<FAIChatFileInfo>& OutFiles);

	/**
	 * 格式化文件列表为可读文本（供 AI 查看）
	 * @param Files - 文件列表
	 * @param FolderPath - 文件夹路径（显示在开头）
	 * @return 格式化的文本
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Chat | File Utils")
	static FString FormatFileListForAI(const TArray<FAIChatFileInfo>& Files, const FString& FolderPath);

	/**
	 *
	 * 转换 UE 虚拟路径到物理路径
	 * @param VirtualPath - 虚拟路径（如 /Game/MyFolder）
	 * @return 物理路径（如 F:/MyProject/Content/MyFolder）
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Chat | File Utils")
	static FString ConvertToPhysicalPath(const FString& VirtualPath);

	/**
	 * 获取常用的 UE 路径列表（快捷访问）
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Chat | File Utils")
	static TArray<FString> GetCommonUEPaths();
};
