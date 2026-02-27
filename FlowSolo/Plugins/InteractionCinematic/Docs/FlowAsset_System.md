# InteractionCinematic FlowAsset System

## Overview

This implementation creates a custom FlowAsset type specifically for InteractionCinematic, allowing nodes to be restricted to only appear in cinematic contexts.

## Architecture

```
UFlowAsset (Base Class)
    └─ UInteractionCinematicFlowAsset (Custom Subclass)
         └─ Used by InteractionCinematicAsset
```

## How It Works

### 1. Custom FlowAsset Type
- **UInteractionCinematicFlowAsset** is a subclass of UFlowAsset
- InteractionCinematicAsset creates this type instead of regular UFlowAsset
- This allows nodes to detect the context they're being used in

### 2. Node Restriction Mechanism

Nodes can restrict themselves using `AllowedAssetClasses`:

```cpp
// Cinematic-only node
UFlowNode_CinematicSpawnActor()
{
    AllowedAssetClasses.Empty();
    AllowedAssetClasses.Add(UInteractionCinematicFlowAsset::StaticClass());
}
```

### 3. Node Categories

**Cinematic-Only Nodes** (Only appear in InteractionCinematic):
- UFlowNode_CinematicSpawnActor
- UFlowNode_CinematicPlaySequence

**Universal Nodes** (Appear everywhere):
- UFlowNode_UniversalLog
- All standard FlowGraph nodes

## Usage

### Creating Cinematic-Only Nodes

```cpp
UCLASS()
class UFlowNode_YourCinematicNode : public UFlowNode
{
    GENERATED_BODY()

public:
    UFlowNode_YourCinematicNode()
    {
        // ✅ Restrict to InteractionCinematic only
        AllowedAssetClasses.Empty();
        AllowedAssetClasses.Add(UInteractionCinematicFlowAsset::StaticClass());

        Category = TEXT("Cinematic");
        InputPins = {TEXT("In")};
        OutputPins = {TEXT("Out")};
    }
};
```

### Creating Universal Nodes

```cpp
UCLASS()
class UFlowNode_YourUniversalNode : public UFlowNode
{
    GENERATED_BODY()

public:
    UFlowNode_YourUniversalNode()
    {
        // ✅ Don't set AllowedAssetClasses - works everywhere
        Category = TEXT("Utility");
        InputPins = {TEXT("In")};
        OutputPins = {TEXT("Out")};
    }
};
```

## File Structure

```
InteractionCinematic/
├── Public/
│   ├── InteractionCinematicFlowAsset.h
│   └── Nodes/
│       ├── FlowNode_CinematicSpawnActor.h
│       ├── FlowNode_CinematicPlaySequence.h
│       └── FlowNode_UniversalLog.h
└── Private/
    ├── InteractionCinematicFlowAsset.cpp
    └── Nodes/
        ├── FlowNode_CinematicSpawnActor.cpp
        ├── FlowNode_CinematicPlaySequence.cpp
        └── FlowNode_UniversalLog.cpp

InteractionCinematicEditor/
└── Private/
    └── InteractionCinematicAssetEditorHelpers.cpp (Modified to use custom FlowAsset)
```

## Benefits

✅ **Clear Separation**: Cinematic-specific nodes only appear in cinematic contexts
✅ **Maintainable**: Each node is its own class with single responsibility
✅ **Extensible**: Easy to add new cinematic-only or universal nodes
✅ **Type-Safe**: Uses UE's class system for compile-time checking
✅ **Non-Invasive**: Doesn't modify FlowGraph plugin code

## Testing

1. Open an **InteractionCinematicAsset**
   - Node palette should show: Cinematic nodes + Universal nodes

2. Open a **regular FlowAsset**
   - Node palette should show: Universal nodes (NO cinematic nodes)

## Start Node: FlowAsset Entry Point

### Overview
The **Start node** (`UFlowNode_Start`) is the entry point for all FlowAsset execution. Every FlowAsset has exactly one Start node that cannot be deleted or duplicated.

### Key Characteristics
```cpp
// From FlowNode_Start.h
UFlowNode_Start()
{
    bCanDelete = false;        // Cannot be deleted
    bCanDuplicate = false;     // Cannot be duplicated
    OutputPins = {TEXT("Out")}; // Single output, no inputs
}
```

### Execution Flow
```
FlowAsset::StartFlow()
    ↓
GetDefaultEntryNode() → Returns Start node
    ↓
Start->TriggerFirstOutput()
    ↓
Connected nodes begin executing
```

### Code Implementation
```cpp
// FlowAsset.cpp - How Start node is triggered
void UFlowAsset::StartFlow()
{
    PreStartFlow();

    // Get the Start node and trigger it
    UFlowNode* EntryNode = GetDefaultEntryNode();
    if (EntryNode)
    {
        EntryNode->TriggerFirstOutput(true);
    }
}
```

### Data Pin Suppliers
The Start node can receive external data when used inside SubGraph nodes:
```cpp
void UFlowNode_Start::ExecuteInput(const FName& PinName)
{
    // Process external data pin suppliers from parent graph
    // Then trigger output to begin the flow
    TriggerFirstOutput(true);
}
```

### Use Cases
- **Standalone FlowAssets**: Start node initiates the graph execution
- **SubGraph FlowAssets**: Start node receives data from parent graph
- **InteractionCinematic**: Start node begins the cinematic sequence

### Summary
The Start node is the "main()" function of a FlowAsset - every execution begins there.

---

## Advanced: Other Restriction Methods

### Method 1: Deny Specific Assets
```cpp
// Node that works everywhere EXCEPT InteractionCinematic
DeniedAssetClasses.Add(UInteractionCinematicFlowAsset::StaticClass());
```

### Method 2: FlowAsset-Side Restrictions
```cpp
// In UInteractionCinematicFlowAsset constructor
AllowedNodeClasses.Empty();
AllowedNodeClasses.Add(UFlowNode_CinematicSpawnActor::StaticClass());
// Only these nodes will be available
```

### Method 3: Hybrid Approach
```cpp
// Node declares what it needs
AllowedAssetClasses.Add(UInteractionCinematicFlowAsset::StaticClass());

// FlowAsset declares what it accepts
AllowedNodeClasses.Add(UFlowNode_CinematicSpawnActor::StaticClass());

// Both must agree for node to appear
```
