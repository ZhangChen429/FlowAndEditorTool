"""
第一步：基础 Claude API 调用

这个示例展示：
1. 如何配置 API Key
2. 如何发送最简单的对话请求
3. 如何处理响应

运行前需要：
- pip install anthropic
- 设置环境变量 ANTHROPIC_API_KEY 或在代码中填入
"""

import os
from anthropic import Anthropic

# ============================================
# 配置 API Key
# ============================================
# 方式1：从环境变量读取（推荐）
# 在终端运行：export ANTHROPIC_API_KEY=your_key_here
# 或创建 .env 文件

# 方式2：直接在代码中设置（仅用于学习测试）
API_KEY = os.environ.get("ANTHROPIC_API_KEY", "your_api_key_here")

# ============================================
# 初始化客户端
# ============================================
client = Anthropic(api_key=API_KEY)

# ============================================
# 发送第一个请求
# ============================================
def basic_chat(user_message: str):
    """
    最基础的对话函数

    参数：
        user_message: 用户输入的消息
    """
    print(f"\n用户: {user_message}")
    print("-" * 50)

    # 调用 Claude API
    response = client.messages.create(
        model="claude-3-5-sonnet-20241022",  # 使用的模型
        max_tokens=1024,                      # 最大返回 token 数
        messages=[
            {"role": "user", "content": user_message}
        ]
    )

    # 提取 AI 的回复
    assistant_message = response.content[0].text
    print(f"AI: {assistant_message}")
    print("-" * 50)

    return assistant_message


# ============================================
# 多轮对话示例
# ============================================
def multi_turn_chat():
    """
    展示如何进行多轮对话
    关键：需要保存历史消息
    """
    print("\n=== 多轮对话示例 ===")

    # 消息历史记录
    messages = []
    round_count = 1
    while True:
        # 第一轮
        prompt_text = f"\n请输入你的第 {round_count} 轮消息（输入 'exit' 退出）："
        user_msg = input(prompt_text).strip()
        # 退出条件
        if user_msg.lower() == "exit":
            print("✅ 退出多轮对话")
            break

        if not user_msg:
            print("❌ 输入不能为空，请重新输入！")
            continue
        # 添加用户消息到历史
        messages.append({"role": "user", "content": user_msg})
        print(f"\n用户: {user_msg}")

        response = client.messages.create(
            model="claude-3-5-sonnet-20241022",
            max_tokens=1024,
            messages=messages
        )

        assistant_msg = response.content[0].text
        messages.append({"role": "assistant", "content": assistant_msg})
        print(f"AI: {assistant_msg}")
        round_count += 1



# ============================================
# 主函数
# ============================================
if __name__ == "__main__":
    print("=" * 50)
    print("第一步：基础 Claude API 调用")
    print("=" * 50)

    # 示例1：单次对话
    print("\n=== 单次对话示例 ===")
    user_input = input("请输入你的问题：").strip()
    basic_chat(user_input)

    print("=" * 50)
    # 示例2：多轮对话
    multi_turn_chat()

    print("\n" + "=" * 50)
    print("✓ 完成！你已经学会了基础的 API 调用")
    print("下一步：运行 02_tool_use.py 学习工具使用")
    print("=" * 50)
