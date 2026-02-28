# SmartObject API 正确用法说明

## 关键 API 修正

### ❌ 错误用法
```cpp
// 这是错误的 - 没有这个签名的 Claim 函数
FSmartObjectClaimHandle ClaimHandle = SmartObjectSubsystem->Claim(SmartObjectHandle, Filter);
```

### ✅ 正确用法
```cpp
// 步骤 1: 创建查询请求
FSmartObjectRequest Request;
Request.QueryBox = FBox(ActorLocation, ActorLocation).ExpandBy(SearchRadius);
Request.Filter.BehaviorDefinitionClasses = { UGameplayBehaviorSmartObjectBehaviorDefinition::StaticClass() };

// 步骤 2: 查找可用的 slots
TArray<FSmartObjectRequestResult> Results;
SmartObjectSubsystem->FindSmartObjects(Request, Results);

// 步骤 3: 检查是否可以 Claim
if (SmartObjectSubsystem->CanBeClaimed(Results[0].SlotHandle))
{
    // 步骤 4: Claim slot
    FSmartObjectClaimHandle ClaimHandle = SmartObjectSubsystem->MarkSlotAsClaimed(
        Results[0].SlotHandle,
        ESmartObjectClaimPriority::Normal,
        FConstStructView() // UserData
    );
}
```

---

## 核心数据结构

### 1. `FSmartObjectHandle`
- 代表一个 SmartObject 实例
- 从 `USmartObjectComponent::GetRegisteredHandle()` 获取

### 2. `FSmartObjectSlotHandle`
- 代表 SmartObject 上的一个具体 slot
- 从 `FSmartObjectRequestResult::SlotHandle` 获取

### 3. `FSmartObjectRequestResult`
- 查询结果，包含:
  - `SmartObjectHandle` - 指向 SmartObject
  - `SlotHandle` - 指向具体的 slot

### 4. `FSmartObjectClaimHandle`
- Claim 成功后返回的 handle
- 用于后续的 Use, Release 等操作

### 5. `FSmartObjectUserHandle` (可选)
- 代表使用者 (User)
- 可以通过 `UserData` 参数传递给 `MarkSlotAsClaimed`

---

## 完整流程示例

### Claim Slot
```cpp
// 1. 获取 SmartObject
USmartObjectComponent* SOComponent = SmartObjectActor->FindComponentByClass<USmartObjectComponent>();
FSmartObjectHandle SmartObjectHandle = SOComponent->GetRegisteredHandle();

// 2. 创建查询
FSmartObjectRequest Request;
Request.QueryBox = FBox(Pawn->GetActorLocation(), Pawn->GetActorLocation()).ExpandBy(1000.0f);
Request.Filter.BehaviorDefinitionClasses = { UGameplayBehaviorSmartObjectBehaviorDefinition::StaticClass() };

// 3. 查找可用 slots
TArray<FSmartObjectRequestResult> Results;
SmartObjectSubsystem->FindSmartObjects(Request, Results);

// 4. 过滤结果 (只要我们的 SmartObject)
FSmartObjectRequestResult* TargetResult = Results.FindByPredicate([SmartObjectHandle](const FSmartObjectRequestResult& Result)
{
    return Result.SmartObjectHandle == SmartObjectHandle && Result.IsValid();
});

// 5. 检查能否 Claim
if (SmartObjectSubsystem->CanBeClaimed(TargetResult->SlotHandle))
{
    // 6. Claim
    FSmartObjectClaimHandle ClaimHandle = SmartObjectSubsystem->MarkSlotAsClaimed(
        TargetResult->SlotHandle,
        ESmartObjectClaimPriority::Normal,
        FConstStructView() // 可选的 UserData
    );

    // 7. 验证 Claim 成功
    if (ClaimHandle.IsValid())
    {
        // 成功！
    }
}
```

### Release Slot
```cpp
// ❌ 废弃的 API
SmartObjectSubsystem->Release(ClaimHandle);

// ✅ 正确的 API
bool bReleased = SmartObjectSubsystem->MarkSlotAsFree(ClaimHandle);
```

---

## UserData 用法 (高级)

`MarkSlotAsClaimed` 的第三个参数 `UserData` 可以用来关联用户特定的数据:

```cpp
// 创建 User 数据结构
USTRUCT()
struct FMyUserData
{
    GENERATED_BODY()

    UPROPERTY()
    AActor* User;

    UPROPERTY()
    float Priority;
};

// 使用 UserData Claim
FMyUserData UserData;
UserData.User = Pawn;
UserData.Priority = 1.0f;

FSmartObjectClaimHandle ClaimHandle = SmartObjectSubsystem->MarkSlotAsClaimed(
    SlotHandle,
    ESmartObjectClaimPriority::Normal,
    FConstStructView::Make(UserData) // 传递用户数据
);
```

---

## 常见错误

### 错误 1: 直接用 SmartObjectHandle Claim
```cpp
// ❌ 错误 - 没有这个签名
FSmartObjectClaimHandle ClaimHandle = SmartObjectSubsystem->Claim(SmartObjectHandle, Filter);
```
**原因**: SmartObject 可能有多个 slots，必须指定具体的 SlotHandle。

### 错误 2: 使用废弃的 Release
```cpp
// ❌ 废弃 - 会有编译警告
SmartObjectSubsystem->Release(ClaimHandle);
```
**正确**: 使用 `MarkSlotAsFree(ClaimHandle)`

### 错误 3: 不检查 CanBeClaimed
```cpp
// ❌ 可能失败 - 没有检查
FSmartObjectClaimHandle ClaimHandle = SmartObjectSubsystem->MarkSlotAsClaimed(SlotHandle, ...);
```
**正确**: 先调用 `CanBeClaimed(SlotHandle)` 检查

---

## BTTask_MoveToAndUseWorkspot 中的实现

在 `BTTask_MoveToAndUseWorkspot::FindAndClaimSmartObject()` 中:

```cpp
// 1. 查找 slots
FSmartObjectRequest Request;
Request.QueryBox = FBox(Pawn->GetActorLocation(), Pawn->GetActorLocation()).ExpandBy(SearchRadius);
Request.Filter.BehaviorDefinitionClasses = { UGameplayBehaviorSmartObjectBehaviorDefinition::StaticClass() };

TArray<FSmartObjectRequestResult> Results;
SmartObjectSubsystem->FindSmartObjects(Request, Results);

// 2. 过滤到目标 SmartObject
FSmartObjectRequestResult* TargetResult = Results.FindByPredicate([SmartObjectHandle](const FSmartObjectRequestResult& Result)
{
    return Result.SmartObjectHandle == SmartObjectHandle && Result.IsValid();
});

// 3. 检查并 Claim
if (SmartObjectSubsystem->CanBeClaimed(TargetResult->SlotHandle))
{
    FSmartObjectClaimHandle ClaimHandle = SmartObjectSubsystem->MarkSlotAsClaimed(
        TargetResult->SlotHandle,
        ESmartObjectClaimPriority::Normal,
        FConstStructView()
    );
}
```

---

## 参考文件

- `SmartObjectSubsystem.h` - 核心 API
- `SmartObjectRequestTypes.h` - Request 和 Result 结构
- `SmartObjectTypes.h` - Handle 类型和 Enum
- `BTTask_MoveToAndUseWorkspot.cpp` - 完整实现示例

---

## 总结

**关键点**:
1. 使用 `FindSmartObjects()` 查找可用 slots
2. 使用 `MarkSlotAsClaimed()` (不是 `Claim()`)
3. 使用 `MarkSlotAsFree()` (不是 `Release()`)
4. 总是检查 `CanBeClaimed()` 再 Claim
5. Claim 需要 `FSmartObjectSlotHandle`，不是 `FSmartObjectHandle`
