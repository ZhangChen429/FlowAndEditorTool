# 从 Python 到 UE C++ 的迁移指南

学完 Python 版本的 Agent 后，这个文档帮助你理解如何在 Unreal Engine C++ 插件中实现相同的功能。

## 核心概念映射

### 1. HTTP 请求

**Python (anthropic SDK):**
```python
from anthropic import Anthropic
client = Anthropic(api_key="...")
response = client.messages.create(...)
```

**UE C++:**
```cpp
// 使用 UE 的 HTTP 模块
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
Request->SetURL("https://api.anthropic.com/v1/messages");
Request->SetVerb("POST");
Request->SetHeader("x-api-key", ApiKey);
Request->SetHeader("anthropic-version", "2023-06-01");
Request->SetHeader("content-type", "application/json");
Request->SetContentAsString(JsonPayload);
Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) {
    // 处理响应
});
Request->ProcessRequest();
```

### 2. JSON 处理

**Python:**
```python
import json
data = {"role": "user", "content": "Hello"}
json_str = json.dumps(data)
parsed = json.loads(json_str)
```

**UE C++:**
```cpp
// 使用 UE 的 JSON 模块
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"

// 创建 JSON
TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
JsonObject->SetStringField("role", "user");
JsonObject->SetStringField("content", "Hello");

FString OutputString;
TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

// 解析 JSON
TSharedPtr<FJsonObject> ParsedObject;
TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
FJsonSerializer::Deserialize(Reader, ParsedObject);
```

### 3. 工具定义

**Python:**
```python
tools = [
    {
        "name": "get_weather",
        "description": "获取天气",
        "input_schema": {
            "type": "object",
            "properties": {
                "city": {"type": "string"}
            }
        }
    }
]
```

**UE C++:**
```cpp
// 定义工具结构
USTRUCT()
struct FAgentTool
{
    GENERATED_BODY()

    UPROPERTY()
    FString Name;

    UPROPERTY()
    FString Description;

    UPROPERTY()
    FString InputSchema; // JSON 字符串

    // 工具执行函数指针
    TFunction<FString(const FString&)> ExecuteFunction;
};

// 注册工具
TArray<FAgentTool> Tools;
FAgentTool WeatherTool;
WeatherTool.Name = "get_weather";
WeatherTool.Description = "获取天气信息";
WeatherTool.InputSchema = R"({"type":"object","properties":{"city":{"type":"string"}}})";
WeatherTool.ExecuteFunction = [](const FString& Input) -> FString {
    // 执行工具逻辑
    return "{\"result\":\"晴天\"}";
};
Tools.Add(WeatherTool);
```

### 4. Agent 循环

**Python:**
```python
while iteration < max_iterations:
    response = client.messages.create(...)
    if response.stop_reason == "tool_use":
        # 执行工具
        result = execute_tool(...)
        # 继续循环
    elif response.stop_reason == "end_turn":
        # 完成
        break
```

**UE C++:**
```cpp
class UAgentExecutor : public UObject
{
public:
    void RunAgent(const FString& UserRequest, int32 MaxIterations)
    {
        CurrentIteration = 0;
        Messages.Empty();

        // 添加用户消息
        FAgentMessage UserMsg;
        UserMsg.Role = "user";
        UserMsg.Content = UserRequest;
        Messages.Add(UserMsg);

        // 开始循环
        SendNextRequest();
    }

private:
    void SendNextRequest()
    {
        if (CurrentIteration >= MaxIterations)
        {
            OnAgentCompleted.Broadcast("达到最大迭代次数");
            return;
        }

        CurrentIteration++;

        // 构建请求
        FString JsonPayload = BuildRequestJson();

        // 发送 HTTP 请求
        TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
        Request->SetURL("https://api.anthropic.com/v1/messages");
        Request->SetVerb("POST");
        Request->SetHeader("x-api-key", ApiKey);
        Request->SetHeader("anthropic-version", "2023-06-01");
        Request->SetHeader("content-type", "application/json");
        Request->SetContentAsString(JsonPayload);

        Request->OnProcessRequestComplete().BindUObject(this, &UAgentExecutor::OnResponseReceived);
        Request->ProcessRequest();
    }

    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
    {
        if (!bSuccess)
        {
            OnAgentCompleted.Broadcast("请求失败");
            return;
        }

        // 解析响应
        FString ResponseContent = Response->GetContentAsString();
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);
        FJsonSerializer::Deserialize(Reader, JsonObject);

        FString StopReason = JsonObject->GetStringField("stop_reason");

        if (StopReason == "tool_use")
        {
            // 提取工具调用
            const TArray<TSharedPtr<FJsonValue>>* ContentArray;
            JsonObject->TryGetArrayField("content", ContentArray);

            for (const TSharedPtr<FJsonValue>& ContentValue : *ContentArray)
            {
                TSharedPtr<FJsonObject> ContentObj = ContentValue->AsObject();
                FString Type = ContentObj->GetStringField("type");

                if (Type == "tool_use")
                {
                    FString ToolName = ContentObj->GetStringField("name");
                    TSharedPtr<FJsonObject> ToolInput = ContentObj->GetObjectField("input");

                    // 执行工具
                    FString ToolResult = ExecuteTool(ToolName, ToolInput);

                    // 添加工具结果到消息历史
                    // ...
                }
            }

            // 继续下一轮
            SendNextRequest();
        }
        else if (StopReason == "end_turn")
        {
            // 提取最终回复
            FString FinalText = ExtractTextFromResponse(JsonObject);
            OnAgentCompleted.Broadcast(FinalText);
        }
    }

    FString ExecuteTool(const FString& ToolName, TSharedPtr<FJsonObject> Input)
    {
        // 查找并执行工具
        for (const FAgentTool& Tool : Tools)
        {
            if (Tool.Name == ToolName)
            {
                FString InputJson;
                TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&InputJson);
                FJsonSerializer::Serialize(Input.ToSharedRef(), Writer);

                return Tool.ExecuteFunction(InputJson);
            }
        }
        return "{\"error\":\"未知工具\"}";
    }

public:
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnAgentCompleted, const FString&);
    FOnAgentCompleted OnAgentCompleted;

private:
    TArray<FAgentMessage> Messages;
    TArray<FAgentTool> Tools;
    int32 CurrentIteration;
    int32 MaxIterations;
    FString ApiKey;
};
```

## UE 插件架构建议

### 1. 模块结构

```
AIChatSupport/
├── Source/
│   ├── AIChatSupportEditor/
│   │   ├── Private/
│   │   │   ├── Agent/
│   │   │   │   ├── AgentExecutor.cpp        # Agent 执行器
│   │   │   │   ├── AgentToolRegistry.cpp    # 工具注册表
│   │   │   │   └── AgentTools.cpp           # 具体工具实现
│   │   │   ├── API/
│   │   │   │   ├── ClaudeAPIClient.cpp      # API 客户端
│   │   │   │   └── ClaudeAPITypes.cpp       # API 数据类型
│   │   │   └── UI/
│   │   │       └── SAIChatWindow.cpp        # UI 界面
│   │   └── Public/
│   │       ├── Agent/
│   │       │   ├── AgentExecutor.h
│   │       │   ├── AgentToolRegistry.h
│   │       │   └── IAgentTool.h             # 工具接口
│   │       └── API/
│   │           ├── ClaudeAPIClient.h
│   │           └── ClaudeAPITypes.h
```

### 2. 关键类设计

**IAgentTool (工具接口):**
```cpp
class IAgentTool
{
public:
    virtual ~IAgentTool() = default;

    virtual FString GetName() const = 0;
    virtual FString GetDescription() const = 0;
    virtual FString GetInputSchema() const = 0;
    virtual FString Execute(const FString& InputJson) = 0;
};
```

**UAgentToolRegistry (工具注册表):**
```cpp
class UAgentToolRegistry : public UObject
{
public:
    void RegisterTool(TSharedPtr<IAgentTool> Tool);
    FString ExecuteTool(const FString& ToolName, const FString& InputJson);
    TArray<FString> GetToolSchemas() const;

private:
    TMap<FString, TSharedPtr<IAgentTool>> Tools;
};
```

**UClaudeAPIClient (API 客户端):**
```cpp
class UClaudeAPIClient : public UObject
{
public:
    void SendMessage(
        const TArray<FAgentMessage>& Messages,
        const TArray<FString>& ToolSchemas,
        const FOnResponseReceived& Callback
    );

private:
    FString ApiKey;
    FString ApiUrl;
};
```

### 3. Slate UI 集成

```cpp
class SAIChatWindow : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SAIChatWindow) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    // Agent 执行器
    TSharedPtr<UAgentExecutor> AgentExecutor;

    // UI 组件
    TSharedPtr<SMultiLineEditableTextBox> InputTextBox;
    TSharedPtr<SScrollBox> MessageScrollBox;

    // 发送消息
    FReply OnSendClicked();

    // Agent 回调
    void OnAgentThinking(const FString& ThinkingText);
    void OnAgentToolCall(const FString& ToolName, const FString& ToolInput);
    void OnAgentCompleted(const FString& FinalResponse);
};
```

## 异步处理

UE 中的 HTTP 请求是异步的，需要使用回调或委托：

```cpp
// 使用委托
DECLARE_DELEGATE_OneParam(FOnToolExecuted, const FString&);

void ExecuteToolAsync(const FString& ToolName, const FString& Input, FOnToolExecuted Callback)
{
    // 异步执行
    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ToolName, Input, Callback]()
    {
        FString Result = DoToolExecution(ToolName, Input);

        // 回到游戏线程
        AsyncTask(ENamedThreads::GameThread, [Result, Callback]()
        {
            Callback.ExecuteIfBound(Result);
        });
    });
}
```

## 配置管理

```cpp
// 在项目设置中添加 API Key 配置
UCLASS(config=Editor, defaultconfig)
class UAIChatSupportSettings : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, Category = "API")
    FString ClaudeAPIKey;

    UPROPERTY(Config, EditAnywhere, Category = "API")
    FString APIEndpoint = "https://api.anthropic.com/v1/messages";

    UPROPERTY(Config, EditAnywhere, Category = "Agent")
    int32 MaxIterations = 10;

    UPROPERTY(Config, EditAnywhere, Category = "Agent")
    FString Model = "claude-3-5-sonnet-20241022";
};
```

## 下一步

1. **先完成 Python 版本的学习**，确保理解 Agent 的核心概念
2. **在 UE 中实现简单的 HTTP 请求**，测试 Claude API 连接
3. **实现基础的消息发送和接收**
4. **逐步添加工具系统**
5. **实现完整的 Agent 循环**
6. **集成到 Slate UI**

## 参考资源

- [UE HTTP 模块文档](https://docs.unrealengine.com/en-US/API/Runtime/HTTP/)
- [UE JSON 处理](https://docs.unrealengine.com/en-US/API/Runtime/Json/)
- [Slate UI 框架](https://docs.unrealengine.com/en-US/ProgrammingAndScripting/Slate/)
- [Claude API 文档](https://docs.anthropic.com/)
