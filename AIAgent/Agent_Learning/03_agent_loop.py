"""
第三步：Agent 循环（Agentic Loop）

这是真正的 Agent！

前面的例子：用户问 → AI 调用一次工具 → 结束
真正的 Agent：用户给目标 → AI 持续决策和行动 → 直到完成目标

这个示例展示：
1. 如何实现 Agent 循环
2. AI 如何连续调用多个工具
3. 如何设置终止条件
"""

import os
import json
from anthropic import Anthropic

API_KEY = os.environ.get("ANTHROPIC_API_KEY", "your_api_key_here")
client = Anthropic(api_key=API_KEY)

# ============================================
# 定义更多工具
# ============================================

def search_database(query: str) -> dict:
    """模拟数据库搜索"""
    database = {
        "Python": "一种高级编程语言，广泛用于 AI、Web 开发等",
        "C++": "一种高性能编程语言，常用于游戏引擎、系统编程",
        "Unreal Engine": "Epic Games 开发的游戏引擎，使用 C++",
        "AI Agent": "能自主决策并使用工具完成任务的 AI 系统"
    }

    results = []
    for key, value in database.items():
        if query.lower() in key.lower() or query.lower() in value.lower():
            results.append({"title": key, "content": value})

    return {"query": query, "results": results, "count": len(results)}


def save_to_file(filename: str, content: str) -> dict:
    """保存内容到文件（真实执行版）"""
    try:
        # 真实写入文件（默认保存在当前运行脚本的目录下）
        with open(filename, "w", encoding="utf-8") as f:
            f.write(content)
        # 打印提示（可选）
        print(f"   ✅ 已真实保存到文件: {os.path.abspath(filename)}")
        print(f"   📄 内容长度: {len(content)} 字符")
        return {"success": True, "filename": filename, "size": len(content)}
    except Exception as e:
        return {"success": False, "error": str(e)}

def send_notification(message: str, priority: str = "normal") -> dict:
    """发送通知"""
    print(f"   [模拟] 发送通知 [{priority}]: {message}")
    return {"success": True, "message": message, "priority": priority}


# ============================================
# 工具定义
# ============================================

tools = [
    {
        "name": "search_database",
        "description": "在知识库中搜索信息",
        "input_schema": {
            "type": "object",
            "properties": {
                "query": {"type": "string", "description": "搜索关键词"}
            },
            "required": ["query"]
        }
    },
    {
        "name": "save_to_file",
        "description": "将内容保存到文件",
        "input_schema": {
            "type": "object",
            "properties": {
                "filename": {"type": "string", "description": "文件名"},
                "content": {"type": "string", "description": "要保存的内容"}
            },
            "required": ["filename", "content"]
        }
    },
    {
        "name": "send_notification",
        "description": "发送通知消息",
        "input_schema": {
            "type": "object",
            "properties": {
                "message": {"type": "string", "description": "通知内容"},
                "priority": {
                    "type": "string",
                    "enum": ["low", "normal", "high"],
                    "description": "优先级"
                }
            },
            "required": ["message"]
        }
    }
]

# ============================================
# 工具执行器
# ============================================

def execute_tool(tool_name: str, tool_input: dict) -> str:
    """执行工具"""
    tool_functions = {
        "search_database": search_database,
        "save_to_file": save_to_file,
        "send_notification": send_notification
    }

    if tool_name in tool_functions:
        result = tool_functions[tool_name](**tool_input)
    else:
        result = {"error": f"未知工具: {tool_name}"}

    return json.dumps(result, ensure_ascii=False)


# ============================================
# Agent 循环
# ============================================

def run_agent(user_goal: str, max_iterations: int = 10):
    """
    运行 Agent 循环

    参数：
        user_goal: 用户的目标/任务
        max_iterations: 最大循环次数（防止无限循环）
    """
    print(f"\n{'='*70}")
    print(f"🎯 用户目标: {user_goal}")
    print(f"{'='*70}\n")

    messages = [{"role": "user", "content": user_goal}]
    iteration = 0

    while iteration < max_iterations:
        iteration += 1
        print(f"\n--- 第 {iteration} 轮 ---")

        # 调用 AI
        response = client.messages.create(
            model="claude-3-5-sonnet-20241022",
            max_tokens=2048,
            tools=tools,
            messages=messages
        )

        print(f"AI 响应类型: {response.stop_reason}")

        # 处理响应
        if response.stop_reason == "tool_use":
            # AI 要使用工具
            messages.append({"role": "assistant", "content": response.content})

            tool_results = []
            for content_block in response.content:
                if content_block.type == "text":
                    print(f"💭 AI 思考: {content_block.text}")

                elif content_block.type == "tool_use":
                    tool_name = content_block.name
                    tool_input = content_block.input

                    print(f"🛠️  调用工具: {tool_name}")
                    print(f"   参数: {json.dumps(tool_input, ensure_ascii=False)}")

                    # 执行工具
                    result = execute_tool(tool_name, tool_input)
                    print(f"   ✓ 结果: {result}")

                    tool_results.append({
                        "type": "tool_result",
                        "tool_use_id": content_block.id,
                        "content": result
                    })

            # 将工具结果返回给 AI
            messages.append({"role": "user", "content": tool_results})

        elif response.stop_reason == "end_turn":
            # AI 完成任务，给出最终回复
            final_text = ""
            for content_block in response.content:
                if content_block.type == "text":
                    final_text += content_block.text

            print(f"\n{'='*70}")
            print(f"✅ Agent 完成任务！")
            print(f"{'='*70}")
            print(f"💬 最终回复:\n{final_text}")
            print(f"{'='*70}")
            print(f"总共执行了 {iteration} 轮")
            break

        else:
            print(f"⚠️  未预期的停止原因: {response.stop_reason}")
            break

    if iteration >= max_iterations:
        print(f"\n⚠️  达到最大迭代次数 ({max_iterations})，Agent 停止")


# ============================================
# 主函数
# ============================================

if __name__ == "__main__":
    print("=" * 70)
    print("第三步：Agent 循环")
    print("=" * 70)

    # 示例1：简单任务（需要 1-2 步）
    print("\n【示例 1：简单任务】")
    run_agent("搜索一下 Python 的信息")

    # 示例2：复杂任务（需要多步）
    print("\n\n【示例 2：复杂任务】")
    run_agent(
        "帮我搜索 'AI Agent' 的信息，然后把搜索结果保存到 agent_info.txt 文件，"
        "最后发送一个通知告诉我已完成"
    )

    print("\n" + "=" * 70)
    print("✓ 完成！你已经理解了 Agent 循环")
    print("\n关键理解：")
    print("  1. Agent 会持续运行，直到任务完成")
    print("  2. 每一轮：AI 思考 → 决定行动 → 执行工具 → 获取结果")
    print("  3. AI 可以连续调用多个工具来完成复杂任务")
    print("  4. 需要设置最大迭代次数防止无限循环")
    print("\n下一步：运行 04_file_agent.py 看实际应用")
    print("=" * 70)
