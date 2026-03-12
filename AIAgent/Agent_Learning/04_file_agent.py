"""
第四步：实际应用 - 文件操作 Agent

这个 Agent 可以：
- 读取文件内容
- 分析文件结构
- 修改文件
- 创建新文件
- 搜索文件中的内容

这是一个更接近实际应用的例子
"""

import os
import json
from pathlib import Path
from anthropic import Anthropic

API_KEY = os.environ.get("ANTHROPIC_API_KEY", "your_api_key_here")
client = Anthropic(api_key=API_KEY)

# ============================================
# 文件操作工具
# ============================================

def read_file(filepath: str) -> dict:
    """读取文件内容"""
    try:
        path = Path(filepath)
        if not path.exists():
            return {"success": False, "error": f"文件不存在: {filepath}"}

        with open(path, 'r', encoding='utf-8') as f:
            content = f.read()

        return {
            "success": True,
            "filepath": filepath,
            "content": content,
            "size": len(content),
            "lines": len(content.split('\n'))
        }
    except Exception as e:
        return {"success": False, "error": str(e)}


def write_file(filepath: str, content: str) -> dict:
    """写入文件"""
    try:
        path = Path(filepath)
        path.parent.mkdir(parents=True, exist_ok=True)

        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

        return {
            "success": True,
            "filepath": filepath,
            "size": len(content)
        }
    except Exception as e:
        return {"success": False, "error": str(e)}


def list_files(directory: str, pattern: str = "*") -> dict:
    """列出目录中的文件"""
    try:
        path = Path(directory)
        if not path.exists():
            return {"success": False, "error": f"目录不存在: {directory}"}

        files = []
        for item in path.glob(pattern):
            files.append({
                "name": item.name,
                "path": str(item),
                "is_file": item.is_file(),
                "size": item.stat().st_size if item.is_file() else 0
            })

        return {
            "success": True,
            "directory": directory,
            "pattern": pattern,
            "files": files,
            "count": len(files)
        }
    except Exception as e:
        return {"success": False, "error": str(e)}


def search_in_file(filepath: str, keyword: str) -> dict:
    """在文件中搜索关键词"""
    try:
        path = Path(filepath)
        if not path.exists():
            return {"success": False, "error": f"文件不存在: {filepath}"}

        with open(path, 'r', encoding='utf-8') as f:
            lines = f.readlines()

        matches = []
        for line_num, line in enumerate(lines, 1):
            if keyword.lower() in line.lower():
                matches.append({
                    "line_number": line_num,
                    "content": line.strip()
                })

        return {
            "success": True,
            "filepath": filepath,
            "keyword": keyword,
            "matches": matches,
            "count": len(matches)
        }
    except Exception as e:
        return {"success": False, "error": str(e)}


# ============================================
# 工具定义
# ============================================

tools = [
    {
        "name": "read_file",
        "description": "读取文件的完整内容",
        "input_schema": {
            "type": "object",
            "properties": {
                "filepath": {"type": "string", "description": "文件路径"}
            },
            "required": ["filepath"]
        }
    },
    {
        "name": "write_file",
        "description": "将内容写入文件（会覆盖原有内容）",
        "input_schema": {
            "type": "object",
            "properties": {
                "filepath": {"type": "string", "description": "文件路径"},
                "content": {"type": "string", "description": "要写入的内容"}
            },
            "required": ["filepath", "content"]
        }
    },
    {
        "name": "list_files",
        "description": "列出目录中的文件",
        "input_schema": {
            "type": "object",
            "properties": {
                "directory": {"type": "string", "description": "目录路径"},
                "pattern": {"type": "string", "description": "文件匹配模式，如 *.py"}
            },
            "required": ["directory"]
        }
    },
    {
        "name": "search_in_file",
        "description": "在文件中搜索关键词，返回匹配的行",
        "input_schema": {
            "type": "object",
            "properties": {
                "filepath": {"type": "string", "description": "文件路径"},
                "keyword": {"type": "string", "description": "搜索关键词"}
            },
            "required": ["filepath", "keyword"]
        }
    }
]

# ============================================
# 工具执行器
# ============================================

def execute_tool(tool_name: str, tool_input: dict) -> str:
    """执行工具"""
    tool_functions = {
        "read_file": read_file,
        "write_file": write_file,
        "list_files": list_files,
        "search_in_file": search_in_file
    }

    if tool_name in tool_functions:
        result = tool_functions[tool_name](**tool_input)
    else:
        result = {"error": f"未知工具: {tool_name}"}

    return json.dumps(result, ensure_ascii=False)


# ============================================
# 文件 Agent
# ============================================

def run_file_agent(user_request: str, max_iterations: int = 10):
    """运行文件操作 Agent"""
    print(f"\n{'='*70}")
    print(f"📁 文件操作请求: {user_request}")
    print(f"{'='*70}\n")

    messages = [{"role": "user", "content": user_request}]
    iteration = 0

    while iteration < max_iterations:
        iteration += 1
        print(f"\n--- 第 {iteration} 轮 ---")

        response = client.messages.create(
            model="claude-3-5-sonnet-20241022",
            max_tokens=4096,
            tools=tools,
            messages=messages
        )

        if response.stop_reason == "tool_use":
            messages.append({"role": "assistant", "content": response.content})
            tool_results = []

            for content_block in response.content:
                if content_block.type == "text":
                    print(f"💭 {content_block.text}")

                elif content_block.type == "tool_use":
                    tool_name = content_block.name
                    tool_input = content_block.input

                    print(f"🛠️  {tool_name}({json.dumps(tool_input, ensure_ascii=False)})")

                    result = execute_tool(tool_name, tool_input)
                    result_obj = json.loads(result)

                    if result_obj.get("success"):
                        print(f"   ✓ 成功")
                    else:
                        print(f"   ✗ 失败: {result_obj.get('error')}")

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

            print(f"\n{'='*70}")
            print(f"✅ 任务完成")
            print(f"{'='*70}")
            print(f"{final_text}")
            print(f"{'='*70}")
            break

        else:
            break

    if iteration >= max_iterations:
        print(f"\n⚠️  达到最大迭代次数")


# ============================================
# 主函数
# ============================================

if __name__ == "__main__":
    print("=" * 70)
    print("第四步：文件操作 Agent")
    print("=" * 70)

    # 创建测试目录
    test_dir = Path("./test_files")
    test_dir.mkdir(exist_ok=True)

    # 创建测试文件
    test_file = test_dir / "example.txt"
    test_file.write_text("这是一个测试文件\nPython 是一种编程语言\nAI Agent 很有趣", encoding='utf-8')

    print(f"\n已创建测试文件: {test_file}")

    # 示例1：读取并分析文件
    print("\n\n【示例 1：读取文件】")
    run_file_agent(f"读取 {test_file} 的内容，并告诉我有多少行")

    # 示例2：搜索文件内容
    print("\n\n【示例 2：搜索文件】")
    run_file_agent(f"在 {test_file} 中搜索包含 'Python' 的行")

    # 示例3：创建新文件
    print("\n\n【示例 3：创建文件】")
    run_file_agent(
        f"创建一个新文件 {test_dir}/summary.txt，内容是对 {test_file} 的总结"
    )

    print("\n" + "=" * 70)
    print("✓ 完成！你已经看到了实际的文件操作 Agent")
    print("\n这个 Agent 可以：")
    print("  - 自主决定需要调用哪些文件操作")
    print("  - 处理文件读写错误")
    print("  - 根据文件内容做出分析和决策")
    print("\n下一步：运行 05_code_agent.py 看代码分析 Agent")
    print("=" * 70)
