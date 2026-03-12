# Narrative Compiler Skill (Three-Layer Architecture)

## Trigger
This skill is triggered when the user message starts with "我是导演策划：" or "我是导演策划:".

## Description
将导演的自然语言叙事描述转换为三层结构化格式：语义层 → 约束层 → 时序层。

## Architecture Overview

AI 叙事系统必须分为三层，避免混乱：

1. **语义层**：发生什么（goal, participants, atoms）
2. **约束层**：它们之间是什么关系（ordering_constraints, sync_constraints）
3. **时序层**：什么时候发生、持续多久（duration_sources, tempo_modifiers）

---

## Instructions

当用户输入以"我是导演策划："开头的消息时，你需要生成三层结构的 JSON。

### 第 1 层：语义层（Semantic Layer）

只回答"发生什么"，**不涉及精确时间**。

```json
{
  "semantic": {
    "goal": "叙事目标（简短英文短语）",
    "participants": ["角色列表"],
    "atoms": [
      {
        "id": "原子标识",
        "type": "动作类型",
        "actor": "角色名",
        "content": "内容（可选）",
        "target": "目标（可选）",
        "animation": "动画提示（可选）",
        "tone": "语气（可选）"
      }
    ]
  }
}
```

**可用的原子类型：**
- `Speak`: 说话
- `MoveTo`: 移动/坐下/站起
- `Gesture`: 手势/动作
- `LookAt`: 看向
- `Wait`: 等待
- `Signal`: 发送信号

---

### 第 2 层：约束层（Constraint Layer）

定义原子之间的**拓扑关系**，还不是最终时间轴。

```json
{
  "constraints": {
    "ordering_constraints": [
      {
        "type": "约束类型",
        "from": "源原子ID 或 [多个ID]",
        "to": "目标原子ID",
        "reason": "推理原因（可选）"
      }
    ],
    "sync_constraints": [
      {
        "type": "同步类型",
        "atoms": ["原子ID列表"],
        "condition": "同步条件"
      }
    ],
    "interrupt_constraints": [
      {
        "trigger": "触发条件",
        "affected": ["受影响的原子ID"],
        "action": "中断动作"
      }
    ]
  }
}
```

**约束类型（ordering_constraints）：**
- `finish_to_start`: A 完成后 B 开始（串行）
- `start_to_start`: A 开始后 B 可以开始（可重叠）
- `join`: A 和 B 都完成后 C 开始（汇合点）
- `join_after_stable`: A 和 B 都稳定后 C 开始
- `signal_wait`: 等待信号触发
- `parallel`: A 和 B 并行执行

**同步类型（sync_constraints）：**
- `wait_all`: 等待所有原子完成
- `wait_any`: 等待任一原子完成
- `wait_stable`: 等待进入稳定状态

---

### 第 3 层：时序层（Timing Layer）

提供时序求解的**参数和提示**，不是精确时间。

```json
{
  "timing": {
    "duration_sources": [
      {
        "atom_id": "原子ID",
        "mode": "时长来源模式",
        "hint": "时长提示（秒）"
      }
    ],
    "tempo_modifiers": {
      "style": "节奏风格",
      "pause_scale": 1.0,
      "overlap_preference": "重叠偏好"
    }
  }
}
```

**时长来源模式（mode）：**
- `resource`: 从资源获取（如语音文件时长）
- `animation`: 从动画获取
- `estimated`: 估算时长
- `fixed`: 固定时长

**节奏风格（style）：**
- `fast`: 快节奏
- `medium`: 中等节奏
- `slow`: 慢节奏
- `slow_deliberate`: 缓慢克制
- `urgent`: 紧急

**重叠偏好（overlap_preference）：**
- `high`: 高重叠（动作可以大量重叠）
- `medium`: 中等重叠
- `low`: 低重叠（动作尽量不重叠）
- `none`: 无重叠（严格串行）

---

## 完整输出格式

```json
{
  "semantic": {
    "goal": "叙事目标",
    "participants": ["角色列表"],
    "atoms": [...]
  },
  "constraints": {
    "ordering_constraints": [...],
    "sync_constraints": [...],
    "interrupt_constraints": [...]
  },
  "timing": {
    "duration_sources": [...],
    "tempo_modifiers": {...}
  }
}
```

---

## 参考示例

### 示例 1：简单顺序

**输入：** "Hanako 站起来后退"

**输出：**
```json
{
  "semantic": {
    "goal": "hanako_retreat",
    "participants": ["Hanako"],
    "atoms": [
      {"id": "hanako_stand", "type": "MoveTo", "actor": "Hanako", "target": "standing"},
      {"id": "hanako_back", "type": "MoveTo", "actor": "Hanako", "target": "safe_distance"}
    ]
  },
  "constraints": {
    "ordering_constraints": [
      {"type": "finish_to_start", "from": "hanako_stand", "to": "hanako_back"}
    ],
    "sync_constraints": [],
    "interrupt_constraints": []
  },
  "timing": {
    "duration_sources": [
      {"atom_id": "hanako_stand", "mode": "animation", "hint": 0.8},
      {"atom_id": "hanako_back", "mode": "animation", "hint": 1.2}
    ],
    "tempo_modifiers": {
      "style": "fast",
      "pause_scale": 0.8,
      "overlap_preference": "low"
    }
  }
}
```

---

### 示例 2：并行动作

**输入：** "V 拔枪的同时，Hanako 看向 V"

**输出：**
```json
{
  "semantic": {
    "goal": "confrontation_start",
    "participants": ["V", "Hanako"],
    "atoms": [
      {"id": "v_draw_weapon", "type": "Gesture", "actor": "V", "animation": "draw_weapon"},
      {"id": "hanako_look", "type": "LookAt", "actor": "Hanako", "target": "V"}
    ]
  },
  "constraints": {
    "ordering_constraints": [
      {"type": "parallel", "from": "v_draw_weapon", "to": "hanako_look"}
    ],
    "sync_constraints": [],
    "interrupt_constraints": []
  },
  "timing": {
    "duration_sources": [
      {"atom_id": "v_draw_weapon", "mode": "animation", "hint": 1.5},
      {"atom_id": "hanako_look", "mode": "animation", "hint": 0.8}
    ],
    "tempo_modifiers": {
      "style": "fast",
      "pause_scale": 0.5,
      "overlap_preference": "high"
    }
  }
}
```

---

### 示例 3：等待汇合

**输入：** "等 Hanako 和 V 都坐好后，Hanako 开始说话"

**输出：**
```json
{
  "semantic": {
    "goal": "formal_conversation_start",
    "participants": ["Hanako", "V"],
    "atoms": [
      {"id": "hanako_sit", "type": "MoveTo", "actor": "Hanako", "target": "chair_hanako"},
      {"id": "v_sit", "type": "MoveTo", "actor": "V", "target": "chair_v"},
      {"id": "wait_stable", "type": "Wait", "actor": "System"},
      {"id": "hanako_speak", "type": "Speak", "actor": "Hanako", "content": "我们开始吧", "tone": "formal"}
    ]
  },
  "constraints": {
    "ordering_constraints": [
      {"type": "join_after_stable", "from": ["hanako_sit", "v_sit"], "to": "wait_stable"},
      {"type": "finish_to_start", "from": "wait_stable", "to": "hanako_speak"}
    ],
    "sync_constraints": [
      {"type": "wait_stable", "atoms": ["hanako_sit", "v_sit"], "condition": "both_seated"}
    ],
    "interrupt_constraints": []
  },
  "timing": {
    "duration_sources": [
      {"atom_id": "hanako_sit", "mode": "animation", "hint": 2.8},
      {"atom_id": "v_sit", "mode": "animation", "hint": 2.4},
      {"atom_id": "wait_stable", "mode": "fixed", "hint": 1.0},
      {"atom_id": "hanako_speak", "mode": "resource", "hint": 3.0}
    ],
    "tempo_modifiers": {
      "style": "slow_deliberate",
      "pause_scale": 1.2,
      "overlap_preference": "low"
    }
  }
}
```

---

## 工作流程

1. **解析导演描述**
   - 识别叙事目标
   - 提取参与角色
   - 识别动作序列
   - 识别时序关键词（"先"、"然后"、"同时"、"等"）
   - 识别节奏提示

2. **生成语义层**
   - 定义 goal（叙事目标）
   - 列出 participants（参与者）
   - 拆解 atoms（原子动作）

3. **生成约束层**
   - 推断 ordering_constraints（顺序约束）
   - 识别 sync_constraints（同步约束）
   - 定义 interrupt_constraints（中断约束，如果有）

4. **生成时序层**
   - 为每个原子指定 duration_sources（时长来源）
   - 设置 tempo_modifiers（节奏修饰器）

5. **保存 JSON 文件**
   - 文件名格式：`narrative_{timestamp}.json`
   - 保存到：`F:\Data\UE_Project\FlowSolo\FlowAndEditorTool\Narratives\`

---

## 输出要求

- 严格按照三层结构输出
- 确保 JSON 格式正确
- 所有 id 使用小写字母和下划线
- 所有 type 必须从可用类型中选择
- 约束类型必须明确，不能模糊
- 时长提示单位为秒（浮点数）

---

## 验证清单

在输出 JSON 之前，请检查：
- [ ] 三层结构完整（semantic, constraints, timing）
- [ ] 所有 atom id 符合命名规则
- [ ] 所有约束类型明确
- [ ] 时长来源模式正确
- [ ] 节奏风格合理
- [ ] JSON 格式正确
