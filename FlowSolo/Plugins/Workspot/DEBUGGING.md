# Workspot 调试指南

## 使用方法

### 方案1: 屏幕显示 + 日志（推荐）

**步骤：**

1. **在蓝图中启用调试**
   ```
   Event BeginPlay
     ↓
   Enable Workspot Debug Display (Enable = True)
   ```

2. **播放 Workspot**
   ```
   Play Workspot On Actor
     - Actor: Your Character
     - Workspot Tree: Your Tree Asset
   ```

3. **观察效果**
   - **屏幕左上角**显示实时信息：
     ```
     🎭 Workspot [BP_Character_C_0]
        State: Playing | Idle: stand | Time: 2.3s
        🎬 Playing: AM_Sit_Down
     ```

   - **角色头顶**显示3D标记（青色球体）

   - **输出日志**显示详细遍历过程：
     ```
     LogWorkspot: 📦 SequenceIterator - Moving to entry [1/3]: WorkspotAnimClip
     LogWorkspot: 🎬 AnimClipIterator::Next - Playing clip: AM_Sit_Down (Idle: sit)
     LogWorkspot: 📦 SequenceIterator - Moving to entry [2/3]: WorkspotLoop
     LogWorkspot: 🔄 LoopIterator - Starting loop (0/5)
     ```

### 方案2: 打印树结构

**在蓝图中调用：**
```
Print Tree Structure
  - Workspot Tree: Your Tree Asset  - Detailed: True (显示详细信息)
```

**输出示例：**
```
╔═══════════════════════════════════════════╗
║   WORKSPOT TREE: WS_Chair_Idle
╚═══════════════════════════════════════════╝
📦 Sequence (3 children)
  [0]
  🎬 AnimClip [AM_Sit_Down] Idle:sit
  [1]
  🔄 Loop [3-5 times]
    🎭 EntryAnim [AM_Sit_Idle] Idle:sit
  [2]
  🎬 AnimClip [AM_Stand_Up] Idle:stand

📋 Transitions (2):
   stand → sit: AM_Trans_Stand_To_Sit
   sit → stand: AM_Trans_Sit_To_Stand
```

### 方案3: 调整日志详细程度

```
Set Debug Verbosity
  - Verbosity Level: 0=Off, 1=Basic, 2=Detailed, 3=VeryVerbose
```

## 图标说明

| 图标 | Entry 类型 | 说明 |
|------|-----------|------|
| 🎬 | AnimClip | 单个动画片段 |
| 🎭 | EntryAnim | 入口/出口动画 |
| 📦 | Sequence | 顺序播放多个Entry |
| 🎲 | RandomList | 随机选择一个Entry |
| 🔀 | Selector | 根据Idle状态选择Entry |
| 🔄 | Loop | 循环播放Entry |

## 调试技巧

### 1. 检查Entry遍历顺序
查看日志中的 `SequenceIterator - Moving to entry [X/Y]` 行，确认执行顺序

### 2. 检查Random选择
查看日志中的 `RandomListIterator - Selected option [X]` 行

### 3. 检查Selector逻辑
查看日志中的 `SelectorIterator - Checking option [X] for idle 'stand'` 行

### 4. 检查Loop次数
查看日志中的 `LoopIterator - Loop X/Y` 行

### 5. 屏幕显示太多时
调用 `Enable Workspot Debug Display (Enable = False)` 关闭屏幕显示，只看日志

## 常见问题

**Q: 看不到屏幕显示？**
- 确认调用了 `Enable Workspot Debug Display`
- 确认在 PIE/运行游戏中（不是编辑器视口）

**Q: 日志太少？**
- 调用 `Set Debug Verbosity (Level = 2)` 增加详细度

**Q: 想看树结构？**
- 在开始前调用 `Print Tree Structure`
- 可以用来验证资产配置是否正确
