# AI Agent 学习项目

这个项目帮助你从零开始理解和开发 AI Agent。

## 什么是 AI Agent？

**Agent（智能体）** = 能自主决策并使用工具完成任务的 AI 系统

核心特征：
- 🧠 **感知**：接收用户输入和环境状态
- 🤔 **决策**：分析当前情况，选择下一步行动
- 🛠️ **行动**：调用工具、执行操作
- 🔄 **循环**：持续执行直到完成目标

## 学习路径

### 第一步：基础 API 调用
- `01_basic_chat.py` - 最简单的 Claude API 对话
- 理解：请求/响应、消息格式、API Key 配置

### 第二步：工具使用（Tool Use）
- `02_tool_use.py` - 给 Agent 添加工具能力
- 理解：函数定义、工具调用、结果返回

### 第三步：Agent 循环
- `03_agent_loop.py` - 实现自主决策循环
- 理解：多轮对话、状态管理、终止条件

### 第四步：实际应用
- `04_file_agent.py` - 文件操作 Agent
- `05_code_agent.py` - 代码分析 Agent

## 环境准备

### 1. 安装 Python 依赖
```bash
pip install -r requirements.txt
```

### 2. 配置 API Key
创建 `.env` 文件：
```
ANTHROPIC_API_KEY=your_api_key_here
```

或者在代码中直接设置（仅用于学习）

### 3. 运行示例
```bash
python 01_basic_chat.py
python 02_tool_use.py
# ... 依次运行
```

## 项目结构

```
Agent_Learning/
├── README.md           # 本文件
├── requirements.txt    # Python 依赖
├── .env               # API Key 配置（需自己创建）
├── 01_basic_chat.py   # 基础对话
├── 02_tool_use.py     # 工具使用
├── 03_agent_loop.py   # Agent 循环
├── 04_file_agent.py   # 文件操作示例
├── 05_code_agent.py   # 代码分析示例
└── utils/             # 工具函数
    ├── __init__.py
    └── tools.py       # 可复用的工具定义
```

## 核心概念

### 1. 消息格式
```python
messages = [
    {"role": "user", "content": "你好"},
    {"role": "assistant", "content": "你好！有什么可以帮你的？"},
    {"role": "user", "content": "今天天气怎么样？"}
]
```

### 2. 工具定义
```python
tools = [
    {
        "name": "get_weather",
        "description": "获取指定城市的天气",
        "input_schema": {
            "type": "object",
            "properties": {
                "city": {"type": "string", "description": "城市名称"}
            },
            "required": ["city"]
        }
    }
]
```

### 3. Agent 循环伪代码
```
while not task_completed:
    1. 发送消息给 AI
    2. AI 返回：文本回复 或 工具调用请求
    3. 如果是工具调用：
        - 执行工具
        - 将结果返回给 AI
        - 继续循环
    4. 如果是文本回复：
        - 显示给用户
        - 判断是否完成任务
```

## 下一步：迁移到 UE C++

学完 Python 版本后，核心思想可以迁移到 C++：
- 使用 HTTP 库（libcurl、cpp-httplib）调用 Claude API
- 用 JSON 库（nlohmann/json）处理请求/响应
- 在 UE Editor 插件中集成 Agent 逻辑
- 提供 Slate UI 界面交互

## 参考资源

- [Anthropic API 文档](https://docs.anthropic.com/)
- [Tool Use 指南](https://docs.anthropic.com/en/docs/build-with-claude/tool-use)
- [Agent 设计模式](https://docs.anthropic.com/en/docs/agents)
