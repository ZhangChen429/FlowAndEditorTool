# Narrative Compiler Skill - 测试用例（三层架构）

## 测试用例 1：简单顺序动作

**输入：**
```
我是导演策划：Hanako 坐下
```

**期望输出：**
```json
{
  "semantic": {
    "goal": "hanako_sit_down",
    "participants": ["Hanako"],
    "atoms": [
      {"id": "hanako_sit", "type": "MoveTo", "actor": "Hanako", "target": "chair_hanako"}
    ]
  },
  "constraints": {
    "ordering_constraints": [],
    "sync_constraints": [],
    "interrupt_constraints": []
  },
  "timing": {
    "duration_sources": [
      {"atom_id": "hanako_sit", "mode": "animation", "hint": 2.5}
    ],
    "tempo_modifiers": {
      "style": "medium",
      "pause_scale": 1.0,
      "overlap_preference": "medium"
    }
  }
}
```

---

## 测试用例 2：串行动作

**输入：**
```
我是导演策划：Hanako 先坐下，然后说话
```

**期望输出：**
```json
{
  "semantic": {
    "goal": "hanako_sit_and_speak",
    "participants": ["Hanako"],
    "atoms": [
      {"id": "hanako_sit", "type": "MoveTo", "actor": "Hanako", "target": "chair_hanako"},
      {"id": "hanako_speak", "type": "Speak", "actor": "Hanako", "content": "...", "tone": "neutral"}
    ]
  },
  "constraints": {
    "ordering_constraints": [
      {"type": "finish_to_start", "from": "hanako_sit", "to": "hanako_speak"}
    ],
    "sync_constraints": [],
    "interrupt_constraints": []
  },
  "timing": {
    "duration_sources": [
      {"atom_id": "hanako_sit", "mode": "animation", "hint": 2.5},
      {"atom_id": "hanako_speak", "mode": "resource", "hint": 3.0}
    ],
    "tempo_modifiers": {
      "style": "medium",
      "pause_scale": 1.0,
      "overlap_preference": "low"
    }
  }
}
```

---

## 测试用例 3：并行动作

**输入：**
```
我是导演策划：Hanako 请 V 坐下，同时自顾自喝一口酒
```

**期望输出：**
```json
{
  "semantic": {
    "goal": "hanako_invite_while_drinking",
    "participants": ["Hanako", "V"],
    "atoms": [
      {"id": "hanako_gesture", "type": "Gesture", "actor": "Hanako", "animation": "gesture_sit"},
      {"id": "hanako_speak", "type": "Speak", "actor": "Hanako", "content": "请坐", "tone": "formal"},
      {"id": "hanako_drink", "type": "Gesture", "actor": "Hanako", "animation": "drink_sake"},
      {"id": "v_sit", "type": "MoveTo", "actor": "V", "target": "chair_v"}
    ]
  },
  "constraints": {
    "ordering_constraints": [
      {"type": "finish_to_start", "from": "hanako_gesture", "to": "hanako_speak"},
      {"type": "finish_to_start", "from": "hanako_speak", "to": "v_sit"},
      {"type": "parallel", "from": "hanako_speak", "to": "hanako_drink"}
    ],
    "sync_constraints": [],
    "interrupt_constraints": []
  },
  "timing": {
    "duration_sources": [
      {"atom_id": "hanako_gesture", "mode": "animation", "hint": 1.2},
      {"atom_id": "hanako_speak", "mode": "resource", "hint": 2.0},
      {"atom_id": "hanako_drink", "mode": "animation", "hint": 2.5},
      {"atom_id": "v_sit", "mode": "animation", "hint": 2.4}
    ],
    "tempo_modifiers": {
      "style": "slow",
      "pause_scale": 1.1,
      "overlap_preference": "high"
    }
  }
}
```

---

## 测试用例 4：等待汇合

**输入：**
```
我是导演策划：等 Hanako 和 V 都坐好后，Hanako 开始说话
```

**期望输出：**
```json
{
  "semantic": {
    "goal": "wait_both_seated_then_speak",
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

## 测试用例 5：复杂场景（紧张气氛）

**输入：**
```
我是导演策划：V 拔枪，Hanako 立刻站起来后退，同时看向 V，紧张气氛
```

**期望输出：**
```json
{
  "semantic": {
    "goal": "confrontation_escalation",
    "participants": ["V", "Hanako"],
    "atoms": [
      {"id": "v_draw_weapon", "type": "Gesture", "actor": "V", "animation": "draw_weapon"},
      {"id": "hanako_stand", "type": "MoveTo", "actor": "Hanako", "target": "standing"},
      {"id": "hanako_back", "type": "MoveTo", "actor": "Hanako", "target": "safe_distance"},
      {"id": "hanako_look", "type": "LookAt", "actor": "Hanako", "target": "V"}
    ]
  },
  "constraints": {
    "ordering_constraints": [
      {"type": "finish_to_start", "from": "v_draw_weapon", "to": "hanako_stand"},
      {"type": "finish_to_start", "from": "hanako_stand", "to": "hanako_back"},
      {"type": "start_to_start", "from": "hanako_stand", "to": "hanako_look"}
    ],
    "sync_constraints": [],
    "interrupt_constraints": []
  },
  "timing": {
    "duration_sources": [
      {"atom_id": "v_draw_weapon", "mode": "animation", "hint": 1.5},
      {"atom_id": "hanako_stand", "mode": "animation", "hint": 0.8},
      {"atom_id": "hanako_back", "mode": "animation", "hint": 1.2},
      {"atom_id": "hanako_look", "mode": "animation", "hint": 0.5}
    ],
    "tempo_modifiers": {
      "style": "urgent",
      "pause_scale": 0.5,
      "overlap_preference": "high"
    }
  }
}
```

---

## 测试用例 6：舒缓气氛

**输入：**
```
我是导演策划：Hanako 坐在酒吧前摆手让守卫离开，对 V 说"你可以安心"，气氛是舒缓
```

**期望输出：**
```json
{
  "semantic": {
    "goal": "create_safe_atmosphere",
    "participants": ["Hanako", "Guard", "V"],
    "atoms": [
      {"id": "hanako_sit", "type": "MoveTo", "actor": "Hanako", "target": "bar_counter"},
      {"id": "hanako_wave", "type": "Gesture", "actor": "Hanako", "animation": "wave_dismiss"},
      {"id": "guard_leave", "type": "MoveTo", "actor": "Guard", "target": "exit"},
      {"id": "hanako_speak", "type": "Speak", "actor": "Hanako", "content": "你可以安心", "tone": "reassuring"}
    ]
  },
  "constraints": {
    "ordering_constraints": [
      {"type": "finish_to_start", "from": "hanako_sit", "to": "hanako_wave"},
      {"type": "finish_to_start", "from": "hanako_wave", "to": "guard_leave"},
      {"type": "finish_to_start", "from": "guard_leave", "to": "hanako_speak"}
    ],
    "sync_constraints": [],
    "interrupt_constraints": []
  },
  "timing": {
    "duration_sources": [
      {"atom_id": "hanako_sit", "mode": "animation", "hint": 2.5},
      {"atom_id": "hanako_wave", "mode": "animation", "hint": 1.0},
      {"atom_id": "guard_leave", "mode": "animation", "hint": 3.0},
      {"atom_id": "hanako_speak", "mode": "resource", "hint": 2.5}
    ],
    "tempo_modifiers": {
      "style": "slow",
      "pause_scale": 1.3,
      "overlap_preference": "low"
    }
  }
}
```

---

## 验证清单

每个测试用例都应该检查：

### 语义层验证
- [ ] goal 是简短的英文短语
- [ ] participants 包含所有参与角色
- [ ] atoms 中所有 id 符合命名规则（小写字母和下划线）
- [ ] atoms 中所有 type 在可用类型列表中
- [ ] 根据 type 填写了必需的字段

### 约束层验证
- [ ] ordering_constraints 的 type 明确（不能模糊）
- [ ] from 和 to 引用的 atom id 存在
- [ ] sync_constraints 的条件清晰
- [ ] interrupt_constraints 的触发条件明确

### 时序层验证
- [ ] 每个 atom 都有对应的 duration_sources
- [ ] mode 是有效的时长来源模式
- [ ] hint 是合理的时长（秒）
- [ ] tempo_modifiers 的 style 符合节奏风格
- [ ] pause_scale 是合理的数值（0.5-2.0）
- [ ] overlap_preference 是有效的重叠偏好

### JSON 格式验证
- [ ] JSON 格式正确，可以被解析
- [ ] 三层结构完整
- [ ] 没有多余的字段
