"""
可复用的工具定义和执行器

这个模块展示如何组织 Agent 的工具代码，
使其可以在多个 Agent 中复用
"""

import json
from typing import Callable, Dict, Any

# ============================================
# 工具注册表
# ============================================

class ToolRegistry:
    """
    工具注册表
    用于管理所有可用的工具
    """

    def __init__(self):
        self._tools: Dict[str, Callable] = {}
        self._schemas: Dict[str, dict] = {}

    def register(self, name: str, function: Callable, schema: dict):
        """
        注册一个工具

        参数：
            name: 工具名称
            function: 工具函数
            schema: 工具的 JSON Schema 定义
        """
        self._tools[name] = function
        self._schemas[name] = schema

    def execute(self, name: str, input_data: dict) -> str:
        """
        执行工具

        参数：
            name: 工具名称
            input_data: 工具输入参数

        返回：
            JSON 格式的执行结果
        """
        if name not in self._tools:
            return json.dumps({"error": f"未知工具: {name}"}, ensure_ascii=False)

        try:
            result = self._tools[name](**input_data)
            return json.dumps(result, ensure_ascii=False)
        except Exception as e:
            return json.dumps({"error": f"工具执行失败: {str(e)}"}, ensure_ascii=False)

    def get_schemas(self) -> list:
        """获取所有工具的 Schema 定义"""
        return list(self._schemas.values())

    def get_tool_names(self) -> list:
        """获取所有工具名称"""
        return list(self._tools.keys())


# ============================================
# 全局工具注册表实例
# ============================================

registry = ToolRegistry()


# ============================================
# 装饰器：简化工具注册
# ============================================

def tool(name: str, description: str, input_schema: dict):
    """
    工具装饰器

    使用示例：
    @tool(
        name="get_weather",
        description="获取天气信息",
        input_schema={
            "type": "object",
            "properties": {
                "city": {"type": "string"}
            },
            "required": ["city"]
        }
    )
    def get_weather(city: str) -> dict:
        return {"city": city, "temp": 20}
    """
    def decorator(func: Callable):
        schema = {
            "name": name,
            "description": description,
            "input_schema": input_schema
        }
        registry.register(name, func, schema)
        return func
    return decorator


# ============================================
# 示例工具定义
# ============================================

@tool(
    name="calculator",
    description="执行基础数学运算",
    input_schema={
        "type": "object",
        "properties": {
            "operation": {
                "type": "string",
                "enum": ["add", "subtract", "multiply", "divide"],
                "description": "运算类型"
            },
            "a": {"type": "number", "description": "第一个数"},
            "b": {"type": "number", "description": "第二个数"}
        },
        "required": ["operation", "a", "b"]
    }
)
def calculator(operation: str, a: float, b: float) -> dict:
    """计算器工具"""
    operations = {
        "add": a + b,
        "subtract": a - b,
        "multiply": a * b,
        "divide": a / b if b != 0 else None
    }

    if operation not in operations:
        return {"success": False, "error": f"不支持的操作: {operation}"}

    result = operations[operation]
    if result is None:
        return {"success": False, "error": "除数不能为0"}

    return {
        "success": True,
        "operation": operation,
        "a": a,
        "b": b,
        "result": result
    }


@tool(
    name="text_analyzer",
    description="分析文本的基本信息",
    input_schema={
        "type": "object",
        "properties": {
            "text": {"type": "string", "description": "要分析的文本"}
        },
        "required": ["text"]
    }
)
def text_analyzer(text: str) -> dict:
    """文本分析工具"""
    words = text.split()
    lines = text.split('\n')

    return {
        "success": True,
        "characters": len(text),
        "words": len(words),
        "lines": len(lines),
        "avg_word_length": sum(len(w) for w in words) / len(words) if words else 0
    }


# ============================================
# Agent 基类
# ============================================

class BaseAgent:
    """
    Agent 基类
    提供通用的 Agent 循环逻辑
    """

    def __init__(self, client, model: str = "claude-3-5-sonnet-20241022"):
        self.client = client
        self.model = model
        self.tool_registry = registry

    def run(self, user_request: str, max_iterations: int = 10, verbose: bool = True):
        """
        运行 Agent

        参数：
            user_request: 用户请求
            max_iterations: 最大迭代次数
            verbose: 是否打印详细信息
        """
        if verbose:
            print(f"\n{'='*70}")
            print(f"🤖 Agent 启动: {user_request}")
            print(f"{'='*70}\n")

        messages = [{"role": "user", "content": user_request}]
        iteration = 0

        while iteration < max_iterations:
            iteration += 1
            if verbose:
                print(f"\n--- 第 {iteration} 轮 ---")

            response = self.client.messages.create(
                model=self.model,
                max_tokens=4096,
                tools=self.tool_registry.get_schemas(),
                messages=messages
            )

            if response.stop_reason == "tool_use":
                messages.append({"role": "assistant", "content": response.content})
                tool_results = []

                for content_block in response.content:
                    if content_block.type == "text" and verbose:
                        print(f"💭 {content_block.text}")

                    elif content_block.type == "tool_use":
                        tool_name = content_block.name
                        tool_input = content_block.input

                        if verbose:
                            print(f"🛠️  {tool_name}({json.dumps(tool_input, ensure_ascii=False)})")

                        result = self.tool_registry.execute(tool_name, tool_input)

                        if verbose:
                            print(f"   ✓ {result[:100]}...")

                        tool_results.append({
                            "type": "tool_result",
                            "tool_use_id": content_block.id,
                            "content": result
                        })

                messages.append({"role": "user", "content": tool_results})

            elif response.stop_reason == "end_turn":
                final_text = ""
                for content_block in response.content:
                    if content_block.type == "text":
                        final_text += content_block.text

                if verbose:
                    print(f"\n{'='*70}")
                    print(f"✅ 完成")
                    print(f"{'='*70}")
                    print(f"{final_text}")
                    print(f"{'='*70}")

                return final_text

            else:
                if verbose:
                    print(f"⚠️  停止原因: {response.stop_reason}")
                break

        if verbose:
            print(f"\n⚠️  达到最大迭代次数")
        return None


# ============================================
# 使用示例
# ============================================

if __name__ == "__main__":
    import os
    from anthropic import Anthropic

    API_KEY = os.environ.get("ANTHROPIC_API_KEY", "your_api_key_here")
    client = Anthropic(api_key=API_KEY)

    # 创建 Agent
    agent = BaseAgent(client)

    # 运行任务
    print("=" * 70)
    print("工具库使用示例")
    print("=" * 70)

    agent.run("计算 123 加 456，然后分析结果数字的文本特征")

    print("\n" + "=" * 70)
    print("✓ 工具库演示完成")
    print("\n这个模块展示了：")
    print("  - 如何使用注册表管理工具")
    print("  - 如何使用装饰器简化工具定义")
    print("  - 如何创建可复用的 Agent 基类")
    print("=" * 70)
