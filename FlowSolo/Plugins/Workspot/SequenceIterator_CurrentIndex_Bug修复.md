# FSequenceIterator CurrentIndex = -1 Bug 修复

## 🐛 问题描述

在 `FSequenceIterator::Next()` 中，当 Sequence 完成一轮并准备 Loop 重新开始时，会出现访问 `Entries[-1]` 的崩溃。

### 错误场景

```cpp
// 假设 Sequence 有 5 个 Entries (index 0-4)
// CurrentIndex 当前为 4

// Step 1: 完成最后一个 Entry，尝试移动到下一个
CurrentIndex++;  // 4 → 5

// Step 2: 检查是否超出范围
if (CurrentIndex >= Sequence->Entries.Num())  // 5 >= 5, true
{
    // Step 3: Loop 重新开始
    CurrentIndex = -1;  // 重置为 -1
    LoopCount++;

    // ❌ BUG: 没有 return，继续向下执行！
}

// Step 4: 尝试访问 Entry
const UWorkspotEntry* Entry = Sequence->Entries[CurrentIndex];
// ❌ CRASH: 访问 Entries[-1]！
```

---

## 🔍 根本原因

**设计意图**:
- `CurrentIndex` 初始化为 `-1`
- 每次调用 `Next()` 时，先 `CurrentIndex++`，使其变成 `0`
- 这样第一次访问的是 `Entries[0]`

**问题**:
当 Loop 重新开始时：
1. 重置 `CurrentIndex = -1`
2. **没有递归调用 `Next()`** 或 **return**
3. 继续执行到访问 `Entries[CurrentIndex]`
4. 此时 `CurrentIndex` 还是 `-1`
5. **崩溃！**

---

## ✅ 修复方案

在重置 `CurrentIndex` 后，**递归调用 `Next(Context)`** 以正确开始新的循环：

### Before (Bug)
```cpp
if (CurrentIndex >= Sequence->Entries.Num())
{
    if (Sequence->bLoopInfinitely || LoopCount < Sequence->MaxLoops - 1)
    {
        CurrentIndex = -1;
        LoopCount++;

        UE_LOG(...);

        // ❌ 没有 return，继续向下执行
    }
    else
    {
        return false;
    }
}

// ❌ 继续执行，访问 Entries[-1]
const UWorkspotEntry* Entry = Sequence->Entries[CurrentIndex];
```

### After (Fixed)
```cpp
if (CurrentIndex >= Sequence->Entries.Num())
{
    if (Sequence->bLoopInfinitely || LoopCount < Sequence->MaxLoops - 1)
    {
        CurrentIndex = -1;
        LoopCount++;

        UE_LOG(...);

        // ✅ 递归调用以开始新的循环
        return Next(Context);
    }
    else
    {
        return false;
    }
}

// ✅ 只有 CurrentIndex 有效时才会执行到这里
const UWorkspotEntry* Entry = Sequence->Entries[CurrentIndex];
```

---

## 📊 执行流程对比

### Bug 版本流程

```
初始状态: CurrentIndex = 4, Entries.Num() = 5

Next() 被调用
  ↓
CurrentIndex++ → 5
  ↓
检查: 5 >= 5? YES
  ↓
Loop 重新开始: CurrentIndex = -1
  ↓
❌ 继续向下执行
  ↓
访问 Entries[-1] → CRASH!
```

### 修复后流程

```
初始状态: CurrentIndex = 4, Entries.Num() = 5

Next() 被调用
  ↓
CurrentIndex++ → 5
  ↓
检查: 5 >= 5? YES
  ↓
Loop 重新开始: CurrentIndex = -1
  ↓
✅ 递归调用 Next(Context)
  ↓
CurrentIndex++ → 0
  ↓
访问 Entries[0] → SUCCESS!
```

---

## 🧪 测试场景

### 场景 1: 单次循环（不会触发 bug）
```cpp
Sequence:
  - bLoopInfinitely = false
  - MaxLoops = 1
  - Entries.Num() = 3

执行:
Entry[0] → Entry[1] → Entry[2] → Finished
```
**结果**: ✅ 正常，因为不会进入 Loop 逻辑

### 场景 2: 无限循环（会触发 bug）
```cpp
Sequence:
  - bLoopInfinitely = true
  - Entries.Num() = 3

执行:
Entry[0] → Entry[1] → Entry[2] → Loop → ❌ CRASH at Entries[-1]
```
**结果**:
- ❌ Bug 版本: 崩溃
- ✅ 修复版本: Entry[0] → Entry[1] → Entry[2] → Entry[0] → ...

### 场景 3: 多次循环（会触发 bug）
```cpp
Sequence:
  - bLoopInfinitely = false
  - MaxLoops = 2
  - Entries.Num() = 3

执行:
Entry[0] → Entry[1] → Entry[2] → Loop → ❌ CRASH at Entries[-1]
```
**结果**:
- ❌ Bug 版本: 崩溃
- ✅ 修复版本: Entry[0] → Entry[1] → Entry[2] → Entry[0] → Entry[1] → Entry[2] → Finished

---

## 🔍 为什么 RandomListIterator 没有这个问题？

看一下 `FRandomListIterator::Next()`:

```cpp
CurrentPlayIndex++;

if (CurrentPlayIndex >= SelectedIndices.Num())
{
    // Finished all selected clips
    return false;  // ✅ 直接 return，不继续执行
}

// 只有 CurrentPlayIndex 有效时才会执行到这里
int32 EntryIndex = SelectedIndices[CurrentPlayIndex];
```

**关键差异**: RandomListIterator **不支持循环**，所以当完成时直接 `return false`，不会继续向下执行。

---

## 📝 教训

### 设计模式: "Reset-Then-Continue" 陷阱

```cpp
// ❌ 危险模式
if (需要重新开始)
{
    重置状态
    // 没有 return，继续执行
}
使用重置后的状态  // 可能是无效状态！
```

```cpp
// ✅ 安全模式
if (需要重新开始)
{
    重置状态
    return 递归调用();  // 或 return/continue
}
使用状态
```

### 原则

当重置迭代器状态时：
1. **立即递归调用** 或
2. **立即 return** 或
3. **立即 continue**

**永远不要**在重置后继续执行，除非你确定状态是有效的！

---

## 🎯 总结

- **Bug**: Loop 重新开始后没有递归调用 `Next()`，导致访问 `Entries[-1]`
- **修复**: 在重置 `CurrentIndex = -1` 后，立即 `return Next(Context)`
- **影响**: 所有使用 Loop 的 Sequence 都会在第二轮时崩溃
- **相关**: RandomListIterator 没有此问题，因为它不支持循环

**修复后，Sequence 的 Loop 功能完全正常！**
