# UnrealGenAISupport MCP 架构原理分析

## 目录
- [1. 什么是 MCP](#1-什么是-mcp)
- [2. 整体架构](#2-整体架构)
- [3. 通信协议详解](#3-通信协议详解)
- [4. 数据流转过程](#4-数据流转过程)
- [5. 关键技术实现](#5-关键技术实现)
- [6. 暴露的能力](#6-暴露的能力)
- [7. 线程安全与同步](#7-线程安全与同步)
- [8. 配置与启动](#8-配置与启动)

---

## 1. 什么是 MCP

**MCP (Model Context Protocol)** 是 Anthropic 推出的一个开放协议，用于 AI 应用（如 Claude Desktop、Cursor IDE）与外部工具/数据源之间的通信。

### 核心概念

```
┌──────────────┐
│ AI 应用      │  (Claude Desktop / Cursor)
│ (MCP Client) │
└──────┬───────┘
       │ MCP Protocol (stdio/HTTP)
       ↓
┌──────────────┐
│ MCP Server   │  (Python/Node.js/任意语言)
│ (Tool Host)  │
└──────┬───────┘
       │ 自定义协议 (Socket/HTTP/gRPC)
       ↓
┌──────────────┐
│ 目标系统     │  (Unreal Engine / VSCode / 数据库 等)
└──────────────┘
```

**关键角色：**
- **MCP Client**：AI 应用，通过标准化协议调用工具
- **MCP Server**：工具提供者，暴露函数（Tools）给 AI
- **Transport**：Client 和 Server 之间的通信方式（stdio / Server-Sent Events）

---

## 2. 整体架构

UnrealGenAISupport 插件使用 **两层架构** 实现 MCP 集成：

```
┌─────────────────────────────────────────────────────────────┐
│                    Claude Desktop / Cursor                  │
│                     (MCP Client)                            │
└────────────────────────┬────────────────────────────────────┘
                         │
                         │ stdio (stdin/stdout)
                         │ MCP Protocol
                         ↓
┌─────────────────────────────────────────────────────────────┐
│               mcp_server.py (FastMCP)                       │
│  - 运行在 Python 独立进程                                    │
│  - 使用 @mcp.tool() 暴露 40+ 工具                           │
│  - 接收 Claude 的调用请求                                    │
└────────────────────────┬────────────────────────────────────┘
                         │
                         │ TCP Socket (JSON)
                         │ localhost:9877
                         ↓
┌─────────────────────────────────────────────────────────────┐
│         unreal_socket_server.py (Python Thread)             │
│  - 运行在 UE Python 环境内                                   │
│  - 监听端口 9877，接收 JSON 命令                             │
│  - 将命令入队列，等待主线程处理                              │
└────────────────────────┬────────────────────────────────────┘
                         │
                         │ Slate Tick Callback
                         │ (主线程调度)
                         ↓
┌─────────────────────────────────────────────────────────────┐
│              C++ Utility Classes                            │
│  - UGenBlueprintNodeCreator (Blueprint 节点操作)            │
│  - UGenActorUtils (Actor 生成/修改)                         │
│  - UGenWidgetUtils (UI Widget 生成)                         │
│  - UGenObjectProperties (属性编辑)                          │
└─────────────────────────────────────────────────────────────┘
```

### 为什么需要两层？

| 层级 | 职责 | 运行环境 | 原因 |
|------|------|----------|------|
| **MCP Server** (mcp_server.py) | 实现 MCP 协议，暴露工具给 Claude | 独立 Python 进程 | MCP 协议需要 stdio 通信，必须是独立进程 |
| **Socket Server** (unreal_socket_server.py) | 接收命令，调度到 UE 主线程执行 | UE Python 环境 | 直接访问 UE API (`unreal` 模块)，必须在 UE 进程内 |

**关键问题：为什么不直接在 UE Python 里实现 MCP？**
- UE Python 环境不支持 stdio 重定向（MCP Client 通过 stdin/stdout 通信）
- MCP FastMCP 库需要异步 I/O，与 UE 主线程事件循环冲突
- 独立进程更稳定，崩溃不影响 UE 编辑器

---

## 3. 通信协议详解

### 3.1 MCP Client ↔ MCP Server (stdio)

**协议：** MCP 标准协议（基于 JSON-RPC 2.0）

**Transport：** stdin/stdout

**示例流程：**

Claude Desktop 配置文件 (`claude_desktop_config.json`)：
```json
{
  "mcpServers": {
    "unreal-handshake": {
      "command": "python",
      "args": [
        "F:/Data/UE_Plugin/UnrealGenAISupport/Content/Python/mcp_server.py"
      ],
      "env": {
        "UNREAL_HOST": "localhost",
        "UNREAL_PORT": "9877"
      }
    }
  }
}
```

Claude 启动时会执行 `python mcp_server.py`，并通过 stdin/stdout 通信：

**请求（stdin）：**
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "tools/call",
  "params": {
    "name": "add_node_to_blueprint",
    "arguments": {
      "blueprint_path": "/Game/MyBlueprint",
      "node_type": "PrintString",
      "node_properties": {
        "InString": "Hello World"
      }
    }
  }
}
```

**响应（stdout）：**
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "success": true,
    "node_id": "K2Node_CallFunction_123",
    "message": "Node added successfully"
  }
}
```

### 3.2 MCP Server ↔ UE Socket Server (TCP Socket)

**协议：** 自定义 JSON over TCP

**端口：** `localhost:9877`

**消息格式：**

**请求（MCP → UE）：**
```json
{
  "type": "add_node",
  "blueprint_path": "/Game/MyBlueprint",
  "graph_name": "EventGraph",
  "node_type": "PrintString",
  "node_properties": {
    "InString": "Hello World"
  },
  "position": {"x": 100, "y": 200}
}
```

**响应（UE → MCP）：**
```json
{
  "success": true,
  "node_id": "K2Node_CallFunction_123",
  "message": "Node created successfully"
}
```

**Python 实现（mcp_server.py）：**

```python
def send_to_unreal(command):
    """发送命令到 UE Socket Server"""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # 连接到 UE 监听的端口
        s.connect(('localhost', 9877))

        # 发送 JSON 命令
        json_str = json.dumps(command)
        s.sendall(json_str.encode('utf-8'))

        # 接收响应（处理分片传输）
        response_data = b""
        while True:
            chunk = s.recv(8192)
            if not chunk:
                break
            response_data += chunk

            # 尝试解析 JSON，完整则退出
            try:
                json.loads(response_data.decode('utf-8'))
                break
            except json.JSONDecodeError:
                continue  # 继续接收

        return json.loads(response_data.decode('utf-8'))
```

**为什么使用 TCP Socket 而不是 HTTP？**
- 更低延迟（无 HTTP 头开销）
- 双向长连接（可选）
- 简单实现，不需要 Web 框架

---

## 4. 数据流转过程

### 完整调用链示例：Claude 要求添加 Blueprint 节点

#### 步骤 1：用户输入
```
用户: "在 MyBlueprint 的 EventGraph 中添加一个 Print String 节点，输出 Hello World"
```

#### 步骤 2：Claude 解析并调用 MCP Tool
Claude 内部推理后选择工具：
```python
add_node_to_blueprint(
    blueprint_path="/Game/MyBlueprint",
    graph_name="EventGraph",
    node_type="PrintString",
    node_properties={"InString": "Hello World"},
    position={"x": 100, "y": 200}
)
```

#### 步骤 3：MCP Server 接收请求
`mcp_server.py` 中的工具函数被触发：

```python
@mcp.tool()
async def add_node_to_blueprint(
    blueprint_path: str,
    graph_name: str,
    node_type: str,
    node_properties: dict = None,
    position: dict = None
) -> dict:
    """Add a node to a blueprint graph"""

    # 构造发送给 UE 的命令
    command = {
        "type": "add_node",
        "blueprint_path": blueprint_path,
        "graph_name": graph_name,
        "node_type": node_type,
        "node_properties": node_properties or {},
        "position": position or {"x": 0, "y": 0}
    }

    # 通过 TCP Socket 发送
    response = send_to_unreal(command)
    return response
```

#### 步骤 4：Socket Server 接收命令
`unreal_socket_server.py` 后台线程监听：

```python
def socket_server_thread():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('localhost', 9877))
    server_socket.listen(1)

    while True:
        conn, addr = server_socket.accept()
        data = conn.recv(8192)

        # 解析 JSON 命令
        command = json.loads(data.decode('utf-8'))

        # 放入队列（稍后在主线程处理）
        command_queue.put({
            "command": command,
            "connection": conn
        })
```

#### 步骤 5：主线程调度执行
UE 主线程每帧通过 Slate Tick Callback 处理队列：

```python
def process_commands(delta_time):
    """Slate Post Tick 回调，每帧执行一次"""
    if not command_queue.empty():
        item = command_queue.get()
        command = item["command"]
        conn = item["connection"]

        # 分发到对应的 Handler
        result = dispatcher.dispatch(command)

        # 将结果通过 Socket 返回
        conn.sendall(json.dumps(result).encode('utf-8'))
        conn.close()
```

#### 步骤 6：Handler 调用 C++ Utils
`blueprint_commands.py` 处理器：

```python
def handle_add_node(command):
    """处理添加节点命令"""
    from gen_blueprint_node_creator import UGenBlueprintNodeCreator

    creator = UGenBlueprintNodeCreator()
    result = creator.add_node(
        blueprint_path=command["blueprint_path"],
        graph_name=command["graph_name"],
        node_type=command["node_type"],
        node_properties=command["node_properties"],
        position_x=command["position"]["x"],
        position_y=command["position"]["y"]
    )

    return {
        "success": result.success,
        "node_id": result.node_id,
        "message": result.message
    }
```

#### 步骤 7：C++ 执行 Unreal API
`UGenBlueprintNodeCreator::AddNode()`（C++）：

```cpp
FNodeCreationResult UGenBlueprintNodeCreator::AddNode(
    const FString& BlueprintPath,
    const FString& GraphName,
    const FString& NodeType,
    const FString& NodePropertiesJson,
    float PosX, float PosY
) {
    // 加载 Blueprint 资产
    UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *BlueprintPath);

    // 找到指定 Graph
    UEdGraph* Graph = FindGraphByName(Blueprint, GraphName);

    // 创建节点（调用 Unreal Blueprint Editor API）
    UK2Node* NewNode = FBlueprintEditorUtils::CreateNode(
        Graph, NodeClass, FVector2D(PosX, PosY)
    );

    // 设置属性
    ApplyPropertiesFromJson(NewNode, NodePropertiesJson);

    // 通知编辑器刷新
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    return FNodeCreationResult{
        .Success = true,
        .NodeId = NewNode->GetName(),
        .Message = "Node created successfully"
    };
}
```

#### 步骤 8：响应返回给 Claude
逆向传递：
```
C++ Result → Python Handler → Socket Response → MCP Server → Claude
```

Claude 收到结果后告诉用户：
```
✅ 已在 MyBlueprint 的 EventGraph 中添加了 Print String 节点（ID: K2Node_CallFunction_123）
```

---

## 5. 关键技术实现

### 5.1 命令分发器 (Command Dispatcher)

**职责：** 根据 `command["type"]` 路由到不同的 Handler

**实现（unreal_socket_server.py）：**

```python
class CommandDispatcher:
    def __init__(self):
        self.handlers = {
            "handshake": self._handle_handshake,
            "spawn": basic_commands.handle_spawn,
            "create_blueprint": blueprint_commands.handle_create_blueprint,
            "add_node": blueprint_commands.handle_add_node,
            "connect_nodes": blueprint_commands.handle_connect_nodes,
            "delete_node": blueprint_commands.handle_delete_node,
            "execute_python": python_commands.handle_execute_python,
            "take_screenshot": basic_commands.handle_take_screenshot,
            # ... 40+ 命令类型
        }

    def dispatch(self, command):
        """分发命令到对应处理器"""
        cmd_type = command.get("type")
        handler = self.handlers.get(cmd_type)

        if handler:
            try:
                return handler(command)
            except Exception as e:
                return {"success": False, "error": str(e)}
        else:
            return {"success": False, "error": f"Unknown command: {cmd_type}"}
```

### 5.2 线程安全的命令队列

**问题：** Socket 监听在后台线程，UE API 必须在主线程调用

**解决方案：** 使用队列 + Slate Post Tick Callback

```python
import queue
from threading import Thread

# 全局队列
command_queue = queue.Queue()

# 后台线程：接收 Socket 连接
def socket_server_thread():
    while True:
        conn, addr = server_socket.accept()
        data = conn.recv(8192)
        command = json.loads(data.decode('utf-8'))

        # 放入队列，等待主线程处理
        command_queue.put({"command": command, "connection": conn})

# 主线程回调：每帧处理队列
def process_commands(delta_time):
    while not command_queue.empty():
        item = command_queue.get()
        result = dispatcher.dispatch(item["command"])

        # 通过 Socket 返回结果
        item["connection"].sendall(json.dumps(result).encode('utf-8'))
        item["connection"].close()

# 注册到 UE 主线程
unreal.register_slate_post_tick_callback(process_commands)

# 启动后台线程
Thread(target=socket_server_thread, daemon=True).start()
```

### 5.3 超时处理

**问题：** 如果命令执行时间过长，Socket 连接会超时

**解决方案：** 10 秒超时，返回错误响应

```python
import time

def process_commands_with_timeout(delta_time):
    if not command_queue.empty():
        item = command_queue.get()
        start_time = time.time()

        # 执行命令（限时 10 秒）
        try:
            result = dispatcher.dispatch(item["command"])
        except Exception as e:
            result = {"success": False, "error": str(e)}

        # 检查超时
        elapsed = time.time() - start_time
        if elapsed > 10.0:
            result = {"success": False, "error": "Command timeout (>10s)"}

        # 返回结果
        item["connection"].sendall(json.dumps(result).encode('utf-8'))
        item["connection"].close()
```

### 5.4 JSON 属性解析

**问题：** Blueprint 节点属性复杂（嵌套结构、特殊类型）

**解决方案：** C++ 端递归解析 JSON，反射设置属性

```cpp
void UGenBlueprintNodeCreator::ApplyPropertiesFromJson(
    UK2Node* Node,
    const FString& JsonString
) {
    TSharedPtr<FJsonObject> JsonObj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    FJsonSerializer::Deserialize(Reader, JsonObj);

    for (auto& Pair : JsonObj->Values) {
        FString PropName = Pair.Key;
        TSharedPtr<FJsonValue> PropValue = Pair.Value;

        // 查找 UProperty
        FProperty* Prop = Node->GetClass()->FindPropertyByName(*PropName);
        if (!Prop) continue;

        // 根据类型设置值
        if (auto* StrProp = CastField<FStrProperty>(Prop)) {
            StrProp->SetPropertyValue_InContainer(Node, PropValue->AsString());
        }
        else if (auto* IntProp = CastField<FIntProperty>(Prop)) {
            IntProp->SetPropertyValue_InContainer(Node, PropValue->AsNumber());
        }
        // ... 更多类型处理
    }
}
```

---

## 6. 暴露的能力

UnrealGenAISupport 通过 MCP 暴露了 40+ 工具，分为以下类别：

### 6.1 Blueprint 操作

| 工具名 | 功能 | 实现类 |
|--------|------|--------|
| `create_blueprint()` | 创建新 Blueprint 类 | `UGenBlueprintUtils` |
| `add_function_to_blueprint()` | 添加函数（带输入/输出参数） | `UGenBlueprintUtils` |
| `add_node_to_blueprint()` | 添加单个节点 | `UGenBlueprintNodeCreator` |
| `add_nodes_bulk()` | 批量添加节点（JSON 数组） | `UGenBlueprintNodeCreator` |
| `connect_blueprint_nodes()` | 连接节点的 Pin | `UGenBlueprintUtils` |
| `delete_node_from_blueprint()` | 删除节点 | `UGenBlueprintNodeCreator` |
| `get_all_nodes_in_graph()` | 查询图中所有节点信息 | `UGenBlueprintNodeCreator` |
| `add_component_to_blueprint()` | 添加组件（StaticMesh / Camera 等） | `UGenBlueprintUtils` |
| `add_variable_to_blueprint()` | 添加成员变量 | `UGenBlueprintUtils` |

### 6.2 场景控制

| 工具名 | 功能 | 实现类 |
|--------|------|--------|
| `spawn_object()` | 生成 Actor（支持基础形状 / 自定义类） | `UGenActorUtils` |
| `edit_component_property()` | 修改组件属性（颜色 / Transform 等） | `UGenObjectProperties` |
| `set_actor_material()` | 设置 Actor 材质 | `UGenActorUtils` |
| `set_actor_transform()` | 设置 Actor 位置/旋转/缩放 | `UGenActorUtils` |
| `create_material()` | 创建材质资产 | 材质工具类 |
| `take_editor_screenshot()` | 编辑器视口截图 | 截图工具 |

### 6.3 代码执行

| 工具名 | 功能 | 实现 |
|--------|------|------|
| `execute_python_script()` | 执行 Python 代码（UE Python 环境） | `unreal.PythonScriptLibrary` |
| `execute_unreal_command()` | 执行控制台命令（如 `ke *` 显示所有按键绑定） | `unreal.SystemLibrary.ExecuteConsoleCommand()` |

### 6.4 UI 生成

| 工具名 | 功能 | 实现类 |
|--------|------|--------|
| `add_widget_to_user_widget()` | 添加 UI 控件到 Widget Blueprint | `UGenWidgetUtils` |
| `edit_widget_property()` | 修改 Widget 属性（文本 / 颜色 / 大小） | `UGenWidgetUtils` |

### 示例：Claude 生成一个完整的 UI 界面

**用户输入：**
```
创建一个登录界面 Widget，包含：
- 标题 "Login"
- 用户名输入框
- 密码输入框
- 登录按钮
```

**Claude 调用工具序列：**
```python
# 1. 创建 Widget Blueprint
create_widget_blueprint(widget_path="/Game/UI/LoginWidget")

# 2. 添加标题文本
add_widget_to_user_widget(
    widget_path="/Game/UI/LoginWidget",
    widget_type="TextBlock",
    widget_name="TitleText",
    properties={"Text": "Login", "FontSize": 24}
)

# 3. 添加用户名输入框
add_widget_to_user_widget(
    widget_path="/Game/UI/LoginWidget",
    widget_type="EditableTextBox",
    widget_name="UsernameInput",
    properties={"HintText": "Username"}
)

# 4. 添加密码输入框
add_widget_to_user_widget(
    widget_path="/Game/UI/LoginWidget",
    widget_type="EditableTextBox",
    widget_name="PasswordInput",
    properties={"HintText": "Password", "IsPassword": true}
)

# 5. 添加登录按钮
add_widget_to_user_widget(
    widget_path="/Game/UI/LoginWidget",
    widget_type="Button",
    widget_name="LoginButton",
    properties={"Text": "Login"}
)
```

---

## 7. 线程安全与同步

### 7.1 为什么需要线程同步？

**问题背景：**
- **Socket 监听**必须在后台线程（否则阻塞 UE 主线程导致编辑器卡死）
- **UE API 调用**必须在主线程（UObject 操作非线程安全）

**架构设计：**

```
┌─────────────────┐                ┌─────────────────┐
│  Socket Thread  │                │   Main Thread   │
│  (后台守护线程)  │                │  (UE 主线程)    │
└────────┬────────┘                └────────┬────────┘
         │                                  │
         │ accept() 接收连接                │
         │ recv() 接收 JSON                 │
         │                                  │
         │ command_queue.put()              │
         │ ────────────────────────────────>│
         │                                  │
         │                         process_commands()
         │                         (Slate Post Tick)
         │                                  │
         │                         dispatcher.dispatch()
         │                                  │
         │                         调用 C++ Utils
         │                                  │
         │ <─────────────────────────────── │
         │ conn.sendall(response)           │
         │                                  │
         └──────────────────────────────────┘
```

### 7.2 代码实现

**队列声明：**
```python
import queue
command_queue = queue.Queue(maxsize=100)  # 最多缓存 100 个命令
```

**后台线程（不阻塞主线程）：**
```python
def socket_server_thread():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind(('localhost', 9877))
    server_socket.listen(5)

    while True:
        try:
            conn, addr = server_socket.accept()
            data = conn.recv(8192)

            if data:
                command = json.loads(data.decode('utf-8'))

                # 放入队列（线程安全）
                command_queue.put({
                    "command": command,
                    "connection": conn,
                    "timestamp": time.time()
                })
        except Exception as e:
            print(f"Socket error: {e}")
```

**主线程回调（Slate 每帧执行）：**
```python
def process_commands(delta_time):
    """Slate Post Tick Callback - 每帧执行一次"""
    processed_count = 0
    max_per_frame = 5  # 每帧最多处理 5 个命令（避免卡顿）

    while not command_queue.empty() and processed_count < max_per_frame:
        item = command_queue.get()

        # 超时检查
        elapsed = time.time() - item["timestamp"]
        if elapsed > 10.0:
            response = {"success": False, "error": "Command timeout"}
        else:
            # 调用分发器执行命令
            response = dispatcher.dispatch(item["command"])

        # 通过 Socket 返回结果
        try:
            item["connection"].sendall(json.dumps(response).encode('utf-8'))
        finally:
            item["connection"].close()

        processed_count += 1

# 注册到 UE 主线程事件循环
unreal.register_slate_post_tick_callback(process_commands)
```

### 7.3 关键点

| 问题 | 解决方案 |
|------|----------|
| Socket 阻塞主线程 | 后台线程监听，主线程通过队列获取命令 |
| UE API 非线程安全 | 所有 UE API 调用都在主线程（Slate Tick Callback） |
| 命令执行时间过长 | 设置 10 秒超时，返回错误响应 |
| 队列无限增长 | `Queue(maxsize=100)` 限制大小，满了会阻塞 |
| 每帧处理太多卡顿 | 每帧最多处理 5 个命令 |

---

## 8. 配置与启动

### 8.1 Claude Desktop 配置

**文件路径：** `~/.config/claude/claude_desktop_config.json`（Mac/Linux）
或 `%APPDATA%\Claude\claude_desktop_config.json`（Windows）

**内容：**
```json
{
  "mcpServers": {
    "unreal-handshake": {
      "command": "python",
      "args": [
        "F:/Data/UE_Plugin/UnrealGenAISupport/Content/Python/mcp_server.py"
      ],
      "env": {
        "UNREAL_HOST": "localhost",
        "UNREAL_PORT": "9877"
      }
    }
  }
}
```

**说明：**
- `command`: 启动命令（Python 解释器）
- `args`: 传递给 Python 的参数（MCP Server 脚本路径）
- `env`: 环境变量（告诉 MCP Server UE 的 Socket 地址）

### 8.2 UE 插件自动启动

**配置文件：** `Saved/Config/socket_server_config.json`

```json
{
  "auto_start_socket_server": true
}
```

**启动流程（init_unreal.py）：**

```python
def auto_start():
    """编辑器启动时自动执行"""

    # 1. 读取配置
    config = load_config()

    # 2. 启动 Socket Server（UE 内部）
    if config.get("auto_start_socket_server"):
        import unreal_socket_server
        unreal_socket_server.start_server()

    # 3. 可选：启动 MCP Server（独立进程）
    if config.get("auto_start_mcp_server"):
        import subprocess
        subprocess.Popen([
            "python",
            "Content/Python/mcp_server.py"
        ])

# 注册到 UE 启动钩子
unreal.register_python_shutdown_callback(cleanup)
auto_start()
```

### 8.3 Editor Preferences 设置

**C++ 设置类：** `GenerativeAISupportSettings.h`

```cpp
UCLASS(config=EditorPerProjectUserSettings, defaultconfig)
class UGenerativeAISupportSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(config, EditAnywhere, Category="Socket Server")
    bool bAutoStartSocketServer = false;

    UPROPERTY(config, EditAnywhere, Category="Socket Server")
    int32 SocketPort = 9877;

    UPROPERTY(config, EditAnywhere, Category="Socket Server")
    bool bEnableLogging = true;
};
```

**配置路径：** Editor → Preferences → Plugins → Generative AI Support

---

## 9. 总结

### 核心设计原则

| 原则 | 实现方式 |
|------|----------|
| **协议分离** | MCP 协议（stdio）+ 自定义 Socket 协议（TCP JSON） |
| **进程隔离** | MCP Server 独立进程，Socket Server 嵌入 UE |
| **线程安全** | 后台线程接收，主线程执行，队列同步 |
| **可扩展性** | 命令分发器 + Handler 模式，新增功能只需注册 |
| **容错性** | 超时机制、异常捕获、错误响应 |

### 技术栈

| 层级 | 技术 |
|------|------|
| **MCP Client** | Claude Desktop / Cursor IDE |
| **MCP Server** | Python + FastMCP 库（Anthropic 官方） |
| **Transport** | stdio (MCP ↔ Server) + TCP Socket (Server ↔ UE) |
| **UE Socket Server** | Python 多线程 + `unreal` 模块 |
| **命令执行** | C++ Utils (UE Blueprint API / Actor API) |

### 关键文件列表

```
UnrealGenAISupport/
├── Content/Python/
│   ├── mcp_server.py                    # MCP 服务端（独立进程）
│   ├── unreal_socket_server.py          # Socket 服务端（UE 内部）
│   ├── init_unreal.py                   # 自动启动脚本
│   └── handlers/                        # 命令处理器
│       ├── basic_commands.py
│       ├── blueprint_commands.py
│       ├── actor_commands.py
│       └── ...
├── Source/GenerativeAISupportEditor/Public/MCP/
│   ├── GenBlueprintNodeCreator.h        # Blueprint 节点创建
│   ├── GenBlueprintUtils.h              # Blueprint 工具类
│   ├── GenActorUtils.h                  # Actor 工具类
│   ├── GenWidgetUtils.h                 # UI 工具类
│   └── GenObjectProperties.h            # 属性编辑
└── Saved/Config/
    └── socket_server_config.json        # 配置文件
```

### 可借鉴的设计

如果你想为 **AIChatSupport** 添加 MCP 支持，可以采用类似架构：

```
Claude Desktop
     ↓ MCP (stdio)
ai_chat_mcp_server.py (独立进程)
     ↓ TCP Socket (JSON)
AIChatSupportEditor (UE 插件)
     ↓
调用现有的 FAIChatRequestFactory
```

**优势：**
- Claude 可以直接调用 AI Chat 功能
- 用户无需手动输入，Claude 自动选择 Provider / Model / URL
- 可以让 Claude 帮你测试不同的 Prompt 组合

**需要暴露的工具示例：**
```python
@mcp.tool()
async def send_ai_chat_message(
    message: str,
    provider: str = "claude",
    model: str = "claude-3-5-sonnet",
    use_custom_url: bool = False,
    custom_url: str = None
) -> dict:
    """Send a message to AI chat and get response"""
    # 通过 Socket 调用 AIChatSupportEditor 的接口
```

---

**文档版本：** 1.0
**最后更新：** 2026-02-07
**作者：** Claude (Anthropic)
