"""
第五步：代码分析 Agent

这个 Agent 专门用于代码分析，可以：
- 分析代码结构
- 查找函数和类
- 检测潜在问题
- 生成代码文档
- 提供重构建议

这是最接近你要在 UE 插件中实现的功能
"""

import os
import json
import re
from pathlib import Path
from anthropic import Anthropic

API_KEY = os.environ.get("ANTHROPIC_API_KEY", "your_api_key_here")
client = Anthropic(api_key=API_KEY)

# ============================================
# 代码分析工具
# ============================================

def analyze_code_structure(filepath: str) -> dict:
    """分析代码结构（简化版）"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()

        # 简单的正则匹配（实际应用中应该用 AST）
        functions = re.findall(r'def\s+(\w+)\s*\(', content)
        classes = re.findall(r'class\s+(\w+)', content)
        imports = re.findall(r'(?:from\s+[\w.]+\s+)?import\s+([\w,\s]+)', content)

        return {
            "success": True,
            "filepath": filepath,
            "functions": functions,
            "classes": classes,
            "imports": [imp.strip() for imp in ','.join(imports).split(',')],
            "lines": len(content.split('\n')),
            "size": len(content)
        }
    except Exception as e:
        return {"success": False, "error": str(e)}


def find_function(filepath: str, function_name: str) -> dict:
    """查找特定函数的定义"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            lines = f.readlines()

        # 查找函数定义
        for i, line in enumerate(lines):
            if f'def {function_name}' in line:
                # 提取函数体（简化版，只取接下来的几行）
                function_lines = [line]
                j = i + 1
                while j < len(lines) and (lines[j].startswith('    ') or lines[j].strip() == ''):
                    function_lines.append(lines[j])
                    j += 1
                    if j - i > 20:  # 限制最多20行
                        break

                return {
                    "success": True,
                    "function_name": function_name,
                    "line_number": i + 1,
                    "code": ''.join(function_lines)
                }

        return {
            "success": False,
            "error": f"未找到函数: {function_name}"
        }
    except Exception as e:
        return {"success": False, "error": str(e)}


def check_code_issues(filepath: str) -> dict:
    """检查代码中的潜在问题（简化版）"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
            lines = content.split('\n')

        issues = []

        # 检查1：过长的行
        for i, line in enumerate(lines, 1):
            if len(line) > 100:
                issues.append({
                    "type": "style",
                    "line": i,
                    "message": f"行过长 ({len(line)} 字符)"
                })

        # 检查2：TODO 注释
        for i, line in enumerate(lines, 1):
            if 'TODO' in line or 'FIXME' in line:
                issues.append({
                    "type": "todo",
                    "line": i,
                    "message": line.strip()
                })

        # 检查3：可能的异常处理缺失
        if 'open(' in content and 'try:' not in content:
            issues.append({
                "type": "warning",
                "line": 0,
                "message": "文件操作可能缺少异常处理"
            })

        return {
            "success": True,
            "filepath": filepath,
            "issues": issues,
            "count": len(issues)
        }
    except Exception as e:
        return {"success": False, "error": str(e)}


def generate_documentation(filepath: str) -> dict:
    """为代码生成文档（简化版）"""
    try:
        analysis = analyze_code_structure(filepath)
        if not analysis["success"]:
            return analysis

        doc = f"# {Path(filepath).name} 文档\n\n"
        doc += f"## 概述\n"
        doc += f"- 总行数: {analysis['lines']}\n"
        doc += f"- 文件大小: {analysis['size']} 字节\n\n"

        if analysis['classes']:
            doc += f"## 类\n"
            for cls in analysis['classes']:
                doc += f"- `{cls}`\n"
            doc += "\n"

        if analysis['functions']:
            doc += f"## 函数\n"
            for func in analysis['functions']:
                doc += f"- `{func}()`\n"
            doc += "\n"

        if analysis['imports']:
            doc += f"## 依赖\n"
            for imp in analysis['imports'][:10]:  # 只显示前10个
                doc += f"- {imp}\n"

        return {
            "success": True,
            "filepath": filepath,
            "documentation": doc
        }
    except Exception as e:
        return {"success": False, "error": str(e)}


# ============================================
# 工具定义
# ============================================

tools = [
    {
        "name": "analyze_code_structure",
        "description": "分析代码文件的结构，包括函数、类、导入等",
        "input_schema": {
            "type": "object",
            "properties": {
                "filepath": {"type": "string", "description": "代码文件路径"}
            },
            "required": ["filepath"]
        }
    },
    {
        "name": "find_function",
        "description": "在代码文件中查找特定函数的定义",
        "input_schema": {
            "type": "object",
            "properties": {
                "filepath": {"type": "string", "description": "代码文件路径"},
                "function_name": {"type": "string", "description": "函数名"}
            },
            "required": ["filepath", "function_name"]
        }
    },
    {
        "name": "check_code_issues",
        "description": "检查代码中的潜在问题和改进建议",
        "input_schema": {
            "type": "object",
            "properties": {
                "filepath": {"type": "string", "description": "代码文件路径"}
            },
            "required": ["filepath"]
        }
    },
    {
        "name": "generate_documentation",
        "description": "为代码文件生成文档",
        "input_schema": {
            "type": "object",
            "properties": {
                "filepath": {"type": "string", "description": "代码文件路径"}
            },
            "required": ["filepath"]
        }
    }
]

# ============================================
# 工具执行器
# ============================================

def execute_tool(tool_name: str, tool_input: dict) -> str:
    """执行工具"""
    tool_functions = {
        "analyze_code_structure": analyze_code_structure,
        "find_function": find_function,
        "check_code_issues": check_code_issues,
        "generate_documentation": generate_documentation
    }

    if tool_name in tool_functions:
        result = tool_functions[tool_name](**tool_input)
    else:
        result = {"error": f"未知工具: {tool_name}"}

    return json.dumps(result, ensure_ascii=False)


# ============================================
# 代码分析 Agent
# ============================================

def run_code_agent(user_request: str, max_iterations: int = 10):
    """运行代码分析 Agent"""
    print(f"\n{'='*70}")
    print(f"💻 代码分析请求: {user_request}")
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

                    print(f"🛠️  {tool_name}")

                    result = execute_tool(tool_name, tool_input)
                    print(f"   ✓ 完成")

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
            print(f"✅ 分析完成")
            print(f"{'='*70}")
            print(f"{final_text}")
            print(f"{'='*70}")
            break

        else:
            break


# ============================================
# 主函数
# ============================================

if __name__ == "__main__":
    print("=" * 70)
    print("第五步：代码分析 Agent")
    print("=" * 70)

    # 使用前面创建的示例文件作为分析对象
    test_file = "01_basic_chat.py"

    if not Path(test_file).exists():
        print(f"\n⚠️  找不到测试文件: {test_file}")
        print("请确保在 Agent_Learning 目录中运行此脚本")
    else:
        # 示例1：分析代码结构
        print("\n\n【示例 1：分析代码结构】")
        run_code_agent(f"分析 {test_file} 的代码结构，告诉我有哪些函数")

        # 示例2：查找特定函数
        print("\n\n【示例 2：查找函数】")
        run_code_agent(f"在 {test_file} 中找到 basic_chat 函数，并解释它的作用")

        # 示例3：检查代码问题
        print("\n\n【示例 3：代码检查】")
        run_code_agent(f"检查 {test_file} 中是否有潜在问题或改进建议")

    print("\n" + "=" * 70)
    print("🎉 恭喜！你已经完成了所有基础学习")
    print("=" * 70)
    print("\n你现在理解了：")
    print("  ✓ 什么是 AI Agent")
    print("  ✓ 如何定义和使用工具")
    print("  ✓ 如何实现 Agent 循环")
    print("  ✓ 如何构建实际应用")
    print("\n下一步：")
    print("  1. 尝试修改这些示例，添加你自己的工具")
    print("  2. 思考如何在 UE C++ 插件中实现类似功能")
    print("  3. 查看 utils/tools.py 学习如何组织代码")
    print("=" * 70)
