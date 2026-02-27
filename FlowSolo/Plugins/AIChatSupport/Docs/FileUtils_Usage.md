# AI Chat Support - File Utils 使用指南

## 功能概述

AIChatSupport 插件现在支持让AI读取项目Content目录的功能,帮助AI更好地理解项目结构。

## 主要功能

### 1. 浏览Content按钮
在AI Chat窗口中,点击"Browse Content"按钮可以:
- 自动列举 `/Game/` 目录下的所有文件和文件夹
- 将文件列表发送给AI
- AI会自动理解项目结构并提供帮助

### 2. 命令行浏览
在聊天输入框中输入特殊命令来浏览特定文件夹:

```
/browse /Game/
/browse /Game/Blueprints
/browse /Game/Maps
/browse /Game/Materials
```

### 3. 支持的路径类型

#### UE虚拟路径
- `/Game/` - 项目Content目录
- `/Engine/` - 引擎Content目录
- `/Plugins/` - 插件目录

#### 物理路径
- 也可以使用绝对物理路径浏览文件系统

## 常用路径快捷列表

插件预定义了以下常用路径:
- `/Game/` - 项目根目录
- `/Game/Blueprints` - 蓝图文件
- `/Game/Maps` - 关卡地图
- `/Game/Materials` - 材质文件
- `/Game/Textures` - 纹理资源
- `/Game/Meshes` - 网格模型
- `/Game/Audio` - 音频文件
- `/Game/UI` - UI资源
- `/Game/Characters` - 角色资源
- `/Game/Animations` - 动画资源

## 文件信息显示

文件列表会显示:
- 📁 文件夹图标
- 📄 文件图标
- 文件大小(对于物理文件)
- 文件数量统计

## 使用示例

### 示例1: 让AI了解项目结构
```
用户: 点击"Browse Content"按钮
AI: 我看到你的项目包含以下文件夹: Blueprints, Maps, Materials...
```

### 示例2: 浏览特定文件夹
```
用户: /browse /Game/Blueprints
AI: 在Blueprints文件夹中,我看到以下蓝图文件: BP_Character, BP_GameMode...
```

### 示例3: 询问项目结构
```
用户: 我的项目中有哪些地图?
AI: 让我帮你查看... (可以建议用户使用 /browse /Game/Maps 命令)
```

## API参考

### UAIChatFileUtils类

#### ListFilesInFolder
```cpp
static bool ListFilesInFolder(const FString& FolderPath, TArray<FAIChatFileInfo>& OutFiles);
```
列举指定文件夹下的所有文件和子文件夹。

#### FormatFileListForAI
```cpp
static FString FormatFileListForAI(const TArray<FAIChatFileInfo>& Files, const FString& FolderPath);
```
将文件列表格式化为AI易读的文本格式。

#### ConvertToPhysicalPath
```cpp
static FString ConvertToPhysicalPath(const FString& VirtualPath);
```
将UE虚拟路径转换为物理路径。

#### GetCommonUEPaths
```cpp
static TArray<FString> GetCommonUEPaths();
```
获取常用的UE路径列表。

## 技术实现

- 使用 `IAssetRegistry` 来列举UE资产
- 使用 `IFileManager` 来列举物理文件
- 支持递归浏览(可选)
- 自动区分文件和文件夹

## 注意事项

1. 浏览Content目录是只读操作,不会修改任何文件
2. 大型项目可能需要一些时间来列举所有文件
3. 建议分批浏览特定文件夹而不是一次性浏览整个项目
4. AI能看到的是资产列表,无法直接读取文件内容

## 未来功能规划

- [ ] 支持读取特定资产的详细信息
- [ ] 支持搜索文件功能
- [ ] 支持文件过滤(按类型、大小等)
- [ ] 集成更多MCP功能(如蓝图编辑等)
