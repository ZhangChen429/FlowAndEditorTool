# ClaimAndUseWorkspotSlot - 完整工作流程

## 🎯 主函数

```cpp
UWorkspotInstance* UWorkspotHelpers::ClaimAndUseWorkspotSlot(
    AAIController* Controller,
    const FSmartObjectClaimHandle& ClaimHandle,
    bool bLockAILogic = true
)
```

### 功能

将 SmartObject 系统与 Workspot 系统完全集成的**一站式函数**。

### 参数

- **Controller**: AI 控制器，其 Pawn 将执行 Workspot
- **ClaimHandle**: 已经 Claimed 的 SmartObject Slot 句柄
- **bLockAILogic**: 是否在 Workspot 执行期间锁定 AI 逻辑（停止移动、暂停行为树）

### 返回值

- **成功**: 返回 `UWorkspotInstance*` - 正在执行的 Workspot 实例
- **失败**: 返回 `nullptr`

---

## 📋 完整工作流程（6步）

```
[1/6] 验证输入
  ├─ Controller 是否有效
  ├─ Pawn 是否存在
  └─ ClaimHandle 是否有效
       ↓
[2/6] Use Slot (标记为 Occupied)
  └─ 调用 SmartObjectSubsystem::MarkSlotAsOccupied<UGameplayBehaviorSmartObjectBehaviorDefinition>()
       ↓
[3/6] 从 BehaviorDefinition 获取 WorkspotBehaviorConfig
  └─ Cast<UWorkspotBehaviorConfig>(BehaviorDef->GameplayBehaviorConfig)
       ↓
[4/6] 从 Config 获取 WorkspotTree
  └─ WorkspotConfig->WorkspotTree
       ↓
[5/6] 锁定 AI 逻辑（如果 bLockAILogic = true）
  ├─ Controller->StopMovement()
  └─ BrainComponent->PauseLogic("Workspot")
       ↓
[6/6] 启动 Workspot
  └─ WorkspotSubsystem->StartWorkspot(Pawn, WorkspotTree, EntryPointTag)
       ↓
    ✅ 返回 UWorkspotInstance*
```

---

## 🔧 辅助函数

### 1. `UseSmartObjectSlot()`
```cpp
const UGameplayBehaviorSmartObjectBehaviorDefinition* UseSmartObjectSlot(
    UObject* WorldContextObject,
    const FSmartObjectClaimHandle& ClaimHandle
)
```

**功能**: 标记 Slot 为 Occupied，返回 BehaviorDefinition

**实现**:
```cpp
SmartObjectSubsystem->MarkSlotAsOccupied<UGameplayBehaviorSmartObjectBehaviorDefinition>(ClaimHandle)
```

---

### 2. `LockAILogic()`
```cpp
void LockAILogic(AAIController* Controller)
```

**功能**: 锁定 AI 逻辑，停止移动和行为树

**实现**:
```cpp
Controller->StopMovement();
BrainComponent->PauseLogic("Workspot");
```

---

### 3. `UnlockAILogic()`
```cpp
void UnlockAILogic(AAIController* Controller)
```

**功能**: 解锁 AI 逻辑，恢复行为树

**实现**:
```cpp
BrainComponent->ResumeLogic("Workspot");
```

---

## 📝 使用示例

### 场景 1: 在 BTTask 中使用

```cpp
// 在 BTTask 的 ExecuteTask 中
AAIController* AIController = OwnerComp.GetAIOwner();

// 假设已经 Claim 了 Slot
FSmartObjectClaimHandle ClaimHandle = /* ... */;

// 一键启动 Workspot
UWorkspotInstance* Instance = UWorkspotHelpers::ClaimAndUseWorkspotSlot(
    AIController,
    ClaimHandle,
    true  // 锁定 AI 逻辑
);

if (Instance)
{
    // Workspot 正在运行
    // 等待完成...
}
```

---

### 场景 2: 在蓝图中使用

```
[Event] AI 到达 SmartObject
   ↓
[Claim Slot] → ClaimHandle
   ↓
[Claim And Use Workspot Slot]
   - Controller: Self (AI Controller)
   - ClaimHandle: 上一步的输出
   - Lock AI Logic: true
   ↓
[Branch] Instance Valid?
   ├─ True → 等待 Workspot 完成
   └─ False → 处理错误
```

---

## 🔍 详细日志输出

函数会输出详细的执行日志：

```
═══════════════════════════════════════════
  ⭐ ClaimAndUseWorkspotSlot - START
═══════════════════════════════════════════
✅ [1/6] Validation passed (Controller: BP_AIController_0, Pawn: BP_Character_0)
✅ [2/6] Slot marked as occupied
✅ [3/6] WorkspotBehaviorConfig found
✅ [4/6] WorkspotTree valid: WS_SitOnChair
✅ [5/6] AI logic locked
✅ [6/6] Workspot started successfully!

═══════════════════════════════════════════
  ✅ SUCCESS - Workspot is now playing
  Tree: WS_SitOnChair
  Pawn: BP_Character_0
═══════════════════════════════════════════
```

如果失败，会在对应步骤输出错误：

```
❌ [2/6] Failed to use SmartObject slot
  UseSmartObjectSlot: MarkSlotAsOccupied returned NULL
    Possible reasons:
    1. Slot is already occupied
    2. BehaviorDefinition type mismatch
    3. ClaimHandle is stale/invalid
```

---

## ⚠️ 错误处理

函数在每一步都有错误检查，如果失败会：

1. **记录详细错误日志**（包含失败原因）
2. **清理已分配的资源**（例如，如果启动 Workspot 失败，会解锁 AI）
3. **返回 nullptr**

---

## 🔗 与其他系统的关系

```
SmartObject System
  ├─ MarkSlotAsClaimed() ← 外部调用（BTTask 或蓝图）
  ├─ MarkSlotAsOccupied() ← ClaimAndUseWorkspotSlot [Step 2]
  └─ MarkSlotAsFree() ← 外部调用（Workspot 完成后）
       ↓
WorkspotBehaviorConfig (Data Asset)
  ├─ WorkspotTree ← ClaimAndUseWorkspotSlot [Step 4]
  └─ PreferredEntryPoint
       ↓
Workspot System
  ├─ StartWorkspot() ← ClaimAndUseWorkspotSlot [Step 6]
  ├─ WorkspotInstance (运行时)
  └─ StopWorkspot() ← 外部调用（完成或中断）
       ↓
AI System
  ├─ StopMovement() ← LockAILogic
  ├─ PauseLogic() ← LockAILogic
  └─ ResumeLogic() ← UnlockAILogic
```

---

## 📌 最佳实践

### ✅ 推荐做法

```cpp
// 1. Claim slot
FSmartObjectClaimHandle ClaimHandle = SmartObjectSubsystem->MarkSlotAsClaimed(...);

// 2. 一键启动 Workspot
UWorkspotInstance* Instance = UWorkspotHelpers::ClaimAndUseWorkspotSlot(
    AIController,
    ClaimHandle,
    true  // 锁定 AI
);

// 3. 监听完成事件
if (Instance)
{
    Instance->OnCompleted.AddLambda([ClaimHandle](UWorkspotInstance* CompletedInstance)
    {
        // 4. 释放 slot
        SmartObjectSubsystem->MarkSlotAsFree(ClaimHandle);
    });
}
```

### ❌ 避免做法

```cpp
// ❌ 不要手动调用这些，ClaimAndUseWorkspotSlot 会处理
SmartObjectSubsystem->MarkSlotAsOccupied(...);  // 已包含在函数内
Controller->StopMovement();                      // 已包含在函数内
WorkspotSubsystem->StartWorkspot(...);          // 已包含在函数内
```

---

## 🎓 总结

**ClaimAndUseWorkspotSlot** 是 SmartObject 和 Workspot 集成的**核心函数**：

- ✅ **一站式调用** - 6 个步骤全自动化
- ✅ **详细日志** - 每一步都有清晰的反馈
- ✅ **错误处理** - 失败时自动清理
- ✅ **函数拆分** - 辅助函数可单独使用
- ✅ **蓝图友好** - UFUNCTION 标记

**使用这个函数，你只需要提供 AIController 和 ClaimHandle，剩下的交给它！**
