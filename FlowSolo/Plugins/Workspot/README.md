# Workspot Plugin for Unreal Engine 5

Cyberpunk 2077的Workspot系统移植到虚幻引擎5，提供完整的NPC行为编排工具。

## 架构概述

### 核心概念

1. **Entry组合模式** - 递归树结构，使用组合模式构建复杂行为
2. **Iterator迭代器** - 遍历Entry树，支持序列、随机、选择等逻辑
3. **Idle状态机** - 自动姿态转换管理（站立、坐下、蹲伏、趴下等）
4. **IdleGuard机制** - 自动检测并插入姿态转换动画
5. **自定义编辑器** - 完整的Toolkit编辑器，支持可视化编辑和扩展

### 模块结构

```
Workspot/
├── Source/
│   ├── Workspot/              # 运行时模块
│   │   ├── Public/
│   │   │   ├── Workspot.h
│   │   │   ├── WorkspotTypes.h           # 核心数据结构
│   │   │   ├── WorkspotEntry.h           # Entry基类和所有Entry类型
│   │   │   ├── WorkspotIterator.h        # Iterator接口和实现
│   │   │   ├── WorkspotTree.h            # 主资产类
│   │   │   ├── WorkspotComponent.h       # 运行时组件
│   │   │   └── WorkspotBehaviorDefinition.h  # SmartObjects集成
│   │   └── Private/
│   │       ├── Workspot.cpp
│   │       ├── WorkspotEntry.cpp
│   │       ├── WorkspotIterator.cpp
│   │       ├── WorkspotTree.cpp
│   │       ├── WorkspotComponent.cpp
│   │       └── WorkspotBehaviorDefinition.cpp
│   │
│   └── WorkspotEditor/        # 编辑器模块
│       ├── Public/
│       │   ├── WorkspotEditor.h          # 编辑器模块
│       │   ├── WorkspotEditorToolkit.h   # 自定义Asset Editor Toolkit
│       │   ├── SWorkspotTreeView.h       # Slate树形视图
│       │   ├── WorkspotTreeFactory.h     # 资产工厂
│       │   ├── WorkspotTreeDetails.h     # 细节面板定制
│       │   └── AssetTypeActions_WorkspotTree.h  # 资产类型动作
│       └── Private/
│           ├── WorkspotEditor.cpp
│           ├── WorkspotEditorToolkit.cpp
│           ├── SWorkspotTreeView.cpp
│           ├── WorkspotTreeFactory.cpp
│           ├── WorkspotTreeDetails.cpp
│           └── AssetTypeActions_WorkspotTree.cpp
│
├── Content/                   # 示例内容（待添加）
└── Resources/                 # 图标资源
```

## 快速开始

### 1. 创建WorkspotTree资产

1. 在Content Browser中右键
2. 选择 Animation → Workspot Tree
3. 双击打开Workspot编辑器

### 2. 编辑Entry树

Workspot编辑器包含三个主要面板：

- **Tree View（左侧）** - Entry层级树视图
- **Details（右上）** - 属性编辑面板
- **Preview（右下）** - 预览视口（待实现）

### 3. 使用WorkspotComponent

在蓝图或C++中添加WorkspotComponent到Actor：

```cpp
// C++ 示例
UWorkspotComponent* WorkspotComp = Actor->FindComponentByClass<UWorkspotComponent>();
WorkspotComp->StartWorkspot(YourWorkspotTree);
```

## Entry类型说明

### 叶子节点

- **AnimClip** - 播放单个动画片段
  - 支持BlendIn/BlendOut时间配置
  - 指定Idle状态

- **EntryAnim** - 进入动画
  - 用于Workspot开始时的过渡

- **ExitAnim** - 退出动画
  - 支持快速退出（FastExit）

### 容器节点

- **Sequence** - 顺序播放
  - 按顺序依次播放子Entry
  - 支持循环（LoopInfinitely / MaxLoops）

- **RandomList** - 随机列表
  - 随机选择N个子Entry播放
  - 支持权重配置
  - 支持暂停时间（PauseDuration）

- **Selector** - 选择器 ⭐核心创新
  - 连续随机选择子Entry
  - 支持权重配置
  - **IdleGuard机制** - 自动检测Idle状态变化并插入转换动画

## Idle状态机系统

### 常用Idle状态

- `stand` - 站立
- `sit` - 坐下
- `crouch` - 蹲伏
- `prone` - 趴下
- `kneel` - 跪姿

### 转换动画命名规范

转换动画应遵循以下命名：
```
<FromIdle>__2__<ToIdle>
```

例如：
- `stand__2__sit` - 从站立到坐下
- `sit__2__stand` - 从坐下到站立

### IdleGuard自动转换

Selector会自动：
1. 检测下一个Entry的Idle状态是否变化
2. 查找对应的转换动画（CustomTransitionAnims或命名规范）
3. 插入转换动画
4. 播放目标Entry

## 扩展Toolkit编辑器

### 添加新的Slate Widget标签页

1. 创建自定义Slate widget（继承自SCompoundWidget）

```cpp
// 示例：创建动画预览widget
class SWorkspotAnimPreview : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SWorkspotAnimPreview) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
};
```

2. 在WorkspotEditorToolkit.h中添加Tab ID

```cpp
static const FName AnimPreviewTabId;
```

3. 在WorkspotEditorToolkit.cpp中注册Tab Spawner

```cpp
void FWorkspotEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
    // ... 现有代码 ...

    InTabManager->RegisterTabSpawner(AnimPreviewTabId, FOnSpawnTab::CreateSP(this, &FWorkspotEditorToolkit::SpawnTab_AnimPreview))
        .SetDisplayName(LOCTEXT("AnimPreviewTab", "Animation Preview"))
        .SetGroup(WorkspaceMenuCategoryRef)
        .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));
}
```

4. 实现Tab Spawn函数

```cpp
TSharedRef<SDockTab> FWorkspotEditorToolkit::SpawnTab_AnimPreview(const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
        .Label(LOCTEXT("AnimPreviewTitle", "Animation Preview"))
        .TabColorScale(GetTabColorScale())
        [
            SNew(SWorkspotAnimPreview)
        ];
}
```

5. 更新布局（在InitWorkspotEditor中）

```cpp
const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_WorkspotEditor_Layout_v1")
    ->AddArea
    (
        FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
        ->Split
        (
            FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
            ->Split(/* TreeView */)
            ->Split(/* Details */)
            ->Split(/* 你的新Tab */)
        )
    );
```

### 添加自定义工具栏按钮

在`ExtendToolbar()`方法中添加：

```cpp
void FWorkspotEditorToolkit::ExtendToolbar()
{
    TSharedPtr<FExtender> ToolbarExtender = MakeShared<FExtender>();

    ToolbarExtender->AddToolBarExtension(
        "Asset",
        EExtensionHook::After,
        GetToolkitCommands(),
        FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& Builder)
        {
            Builder.AddToolBarButton(
                FUIAction(FExecuteAction::CreateSP(this, &FWorkspotEditorToolkit::OnCustomButtonClicked)),
                NAME_None,
                LOCTEXT("CustomButton", "Custom"),
                LOCTEXT("CustomButtonTooltip", "执行自定义操作"),
                FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Save")
            );
        })
    );

    AddToolbarExtender(ToolbarExtender);
}
```

## 待实现功能

### 短期（Phase 1）
- [x] 基础Entry系统
- [x] Iterator实现
- [x] WorkspotComponent运行时
- [x] 基础Toolkit编辑器
- [x] TreeView可视化
- [ ] 完整的预览系统

### 中期（Phase 2-3）
- [ ] 图形化编辑器（类似Behavior Tree）
- [ ] 拖拽支持
- [ ] 右键菜单（添加/删除Entry）
- [ ] 动画资产选择器
- [ ] 实时预览viewport
- [ ] Transition动画可视化

### 长期（Phase 4）
- [ ] 性能分析工具
- [ ] 批量操作
- [ ] 模板系统
- [ ] 与Sequencer集成
- [ ] 网络同步支持

## 与Cyberpunk 2077原系统的差异

### 保留的核心特性
✅ Entry组合模式
✅ Iterator迭代器
✅ Idle状态机
✅ IdleGuard自动转换
✅ Props系统
✅ Reaction系统框架

### 简化或调整
- **7层架构** → 简化为核心层，利用UE现有系统
- **Resource系统** → 使用UE的DataAsset和引用系统
- **LookAt系统** → 可通过UE的Control Rig实现
- **QuestPhase集成** → 使用SmartObjects桥接

### UE特有扩展
- EditInlineNew支持嵌套编辑
- Blueprint友好的API
- 与SmartObjects集成
- 自定义Toolkit编辑器
- Slate UI扩展性

## 参考资料

原始设计文档位于项目根目录：
- 01-Workspot系统概述.md
- 02-Entry节点系统.md
- 03-迭代器与执行流程.md
- 04-技术设计细节.md
- 05-实现案例分析.md

## 支持与反馈

如有问题或建议，请通过GitHub Issues反馈。
