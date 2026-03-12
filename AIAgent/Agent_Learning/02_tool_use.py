"""
第二步：工具使用（Tool Use / Function Calling）

这是 Agent 的核心能力！

普通 ChatBot：只能回答问题
Agent：可以调用工具执行实际操作

这个示例展示：
1. 如何定义工具
2. AI 如何决定调用哪个工具
3. 如何执行工具并返回结果
"""

import os
import json
import requests
import pandas as pd
from anthropic import Anthropic

# 确保 API Key 是纯 ASCII 字符串
API_KEY = os.environ.get("ANTHROPIC_API_KEY", "your_api_key_here")


# 初始化客户端时确保所有参数都是 ASCII
client = Anthropic(
    api_key=API_KEY.strip(),  # 去除可能的空格
    max_retries=2
)

AMAP_API_KEY = "3c97d809d4053369a783e053ff88c405"  # 请替换为 Web 服务类型的 API Key

# 加载城市编码表
CITY_CODE_PATH = r"F:\Data\UE_Project\FlowSolo\FlowAndEditorTool\AIAgent\Agent_Learning\Tool\AMap_adcode_citycode.xlsx"
city_df = None

def load_city_codes():
    """加载城市编码表到内存"""
    global city_df
    try:
        city_df = pd.read_excel(CITY_CODE_PATH)
        print(f"✓ 已加载 {len(city_df)} 条城市编码数据")
    except Exception as e:
        print(f"✗ 加载城市编码表失败: {e}")

def get_adcode_from_city(city_name: str) -> str:
    """从城市名称查询 adcode"""
    if city_df is None:
        load_city_codes()

    # 移除"市"、"区"、"县"等后缀进行模糊匹配
    city_clean = city_name.replace("市", "").replace("区", "").replace("县", "")

    # 精确匹配
    result = city_df[city_df['中文名'].str.contains(city_clean, na=False)]

    if not result.empty:
        # 优先返回市级编码（通常以00结尾）
        city_level = result[result['adcode'].astype(str).str.endswith('00')]
        if not city_level.empty:
            adcode = str(city_level.iloc[0]['adcode'])
            matched_name = city_level.iloc[0]['中文名']
            print(f"   📍 匹配到城市: {matched_name} -> adcode: {adcode}")
            return adcode
        else:
            # 返回第一个匹配结果
            adcode = str(result.iloc[0]['adcode'])
            matched_name = result.iloc[0]['中文名']
            print(f"   📍 匹配到区域: {matched_name} -> adcode: {adcode}")
            return adcode

    return None
# ============================================
# 定义工具函数（实际执行的代码）
# ============================================

def get_real_weather(city: str, unit: str = "celsius") -> dict:
    """
    真实获取天气信息：从本地编码表查询 adcode，然后调用高德天气API
    """
    try:
        # 步骤1：从本地 Excel 表格查询城市编码
        adcode = get_adcode_from_city(city)
        if not adcode:
            return {"error": f"无法在编码表中找到城市: {city}，请检查城市名称"}

        # 步骤2：用城市编码调用天气API
        weather_url = "https://restapi.amap.com/v3/weather/weatherInfo"
        weather_params = {
            "key": AMAP_API_KEY,
            "city": adcode,
            "extensions": "base",  # base=实时天气，all=包含预报
            "output": "json"
        }

        # 发送HTTP请求获取实时天气
        weather_resp = requests.get(weather_url, params=weather_params, timeout=10)
        weather_resp.raise_for_status()
        weather_data = weather_resp.json()

        # 校验天气数据
        if weather_data.get("status") != "1" or not weather_data.get("lives"):
            return {"error": f"无法获取{city}的天气数据，API返回: {weather_data}"}

        # 提取核心天气信息
        live_weather = weather_data["lives"][0]
        return {
            "city": city,
            "temperature": float(live_weather["temperature"]),  # 实时温度
            "condition": live_weather["weather"],  # 天气状况（晴/雨/多云）
            "wind": f"{live_weather['winddirection']} {live_weather['windpower']}级",  # 风向风力
            "humidity": live_weather["humidity"],  # 湿度
            "update_time": live_weather["reporttime"],  # 数据更新时间
            "unit": unit
        }

    except requests.exceptions.Timeout:
        return {"error": "天气API请求超时，请稍后重试"}
    except requests.exceptions.RequestException as e:
        return {"error": f"网络请求失败：{str(e)}"}
    except Exception as e:
        return {"error": f"获取天气出错：{str(e)}"}


def calculate(operation: str, a: float, b: float) -> dict:
    """
    执行数学计算
    """
    operations = {
        "add": a + b,
        "subtract": a - b,
        "multiply": a * b,
        "divide": a / b if b != 0 else "错误：除数不能为0"
    }

    if operation in operations:
        return {
            "operation": operation,
            "a": a,
            "b": b,
            "result": operations[operation]
        }
    else:
        return {"error": f"不支持的操作: {operation}"}


# ============================================
# 定义工具描述（告诉 AI 有哪些工具可用）
# ============================================

tools = [
    {
        "name": "get_weather",
        "description": "获取指定城市的真实实时天气信息，包括温度、天气状况、风向风力等",
        "input_schema": {
            "type": "object",
            "properties": {
                "city": {
                    "type": "string",
                    "description": "城市名称，例如：北京、上海、深圳"
                },
                "unit": {
                    "type": "string",
                    "enum": ["celsius", "fahrenheit"],
                    "description": "温度单位，默认为摄氏度"
                }
            },
            "required": ["city"]
        }
    },
    {
        "name": "calculate",
        "description": "执行基础数学运算：加减乘除",
        "input_schema": {
            "type": "object",
            "properties": {
                "operation": {
                    "type": "string",
                    "enum": ["add", "subtract", "multiply", "divide"],
                    "description": "运算类型"
                },
                "a": {
                    "type": "number",
                    "description": "第一个数字"
                },
                "b": {
                    "type": "number",
                    "description": "第二个数字"
                }
            },
            "required": ["operation", "a", "b"]
        }
    }
]

# ============================================
# 工具调度器（根据工具名执行对应函数）
# ============================================

def execute_real_tool(tool_name: str, tool_input: dict) -> str:
    """
    执行工具并返回结果
    """
    if tool_name == "get_weather":
        result = get_real_weather(**tool_input)
    elif tool_name == "calculate":
        result = calculate(**tool_input)
    else:
        result = {"error": f"未知工具：{tool_name}"}
    return json.dumps(result, ensure_ascii=False, indent=2)


# ============================================
# 带工具的对话函数
# ============================================

def chat_with_tools(user_message: str):
    """
    支持工具调用的对话
    """
    print(f"\n用户: {user_message}")
    print("=" * 60)

    messages = [{"role": "user", "content": user_message}]

    # 发送请求（包含工具定义）
    response = client.messages.create(
        model="claude-3-5-sonnet-20241022",
        max_tokens=1024,
        tools=tools,  # 关键：传入工具定义
        messages=messages
    )

    print(f"\nAI 的响应类型: {response.stop_reason}")

    # 检查 AI 是否要调用工具
    if response.stop_reason == "tool_use":
        # AI 决定使用工具
        for content_block in response.content:
            if content_block.type == "tool_use":
                tool_name = content_block.name
                tool_input = content_block.input

                print(f"\n🛠️  AI 决定调用工具: {tool_name}")
                print(f"   参数: {json.dumps(tool_input, ensure_ascii=False)}")

                # 执行工具
                tool_result = execute_real_tool(tool_name, tool_input)
                print(f"   结果: {tool_result}")

                # 将工具结果返回给 AI
                messages.append({"role": "assistant", "content": response.content})
                messages.append({
                    "role": "user",
                    "content": [{
                        "type": "tool_result",
                        "tool_use_id": content_block.id,
                        "content": tool_result
                    }]
                })

                # AI 根据工具结果生成最终回复
                final_response = client.messages.create(
                    model="claude-3-5-sonnet-20241022",
                    max_tokens=1024,
                    tools=tools,
                    messages=messages
                )

                final_text = final_response.content[0].text
                print(f"\n💬 AI 最终回复: {final_text}")

    else:
        # AI 直接回复文本（不需要工具）
        text_response = response.content[0].text
        print(f"\n💬 AI 回复: {text_response}")

    print("=" * 60)

# ===================== 5. 核心Agent逻辑 =====================
def real_ai_agent_chat(user_question: str, conversation_history: list = None):
    """真实的AI Agent对话流程：提问→工具决策→HTTP请求→结果整理"""
    print(f"\n🤔 用户提问：{user_question}")
    print("-" * 80)

    # 1. 构建对话消息（保留历史对话）
    if conversation_history is None:
        conversation_history = []

    conversation_history.append({"role": "user", "content": user_question})

    # 2. 发送请求给Claude，让它判断是否调用工具
    response = client.messages.create(
        model="claude-3-5-sonnet-20241022",
        max_tokens=2048,
        tools=tools,
        messages=conversation_history
    )

    # 3. 判断AI的决策：是否调用工具
    if response.stop_reason == "tool_use":
        # 3.1 AI决定调用工具
        for content_block in response.content:
            if content_block.type == "tool_use":
                tool_name = content_block.name
                tool_input = content_block.input
                print(f"🛠️ AI决策：调用【{tool_name}】工具")
                print(f"📥 工具参数：{json.dumps(tool_input, ensure_ascii=False)}")

                # 3.2 执行工具（核心：发送HTTP请求获取真实天气）
                tool_result = execute_real_tool(tool_name, tool_input)
                print(f"📤 工具返回结果：\n{tool_result}")

                # 3.3 把工具结果回传给AI，让它整理成自然语言
                conversation_history.append({"role": "assistant", "content": response.content})
                conversation_history.append({
                    "role": "user",
                    "content": [{
                        "type": "tool_result",
                        "tool_use_id": content_block.id,
                        "content": tool_result
                    }]
                })

                # 3.4 AI生成最终回复
                final_response = client.messages.create(
                    model="claude-3-5-sonnet-20241022",
                    max_tokens=1024,
                    tools=tools,
                    messages=conversation_history
                )

                final_answer = final_response.content[0].text
                print("-" * 80)
                print(f"💡 AI最终回复：\n{final_answer}")

                # 保存AI的最终回复到历史
                conversation_history.append({"role": "assistant", "content": final_answer})

    else:
        # 3.1 AI无需调用工具，直接回复
        direct_answer = response.content[0].text
        print(f"💡 AI直接回复：\n{direct_answer}")

        # 保存AI的回复到历史
        conversation_history.append({"role": "assistant", "content": direct_answer})

    print("-" * 80)

    return conversation_history




# ============================================
# 主函数
# ============================================

if __name__ == "__main__":
    print("=" * 60)
    print("AI Agent Tool Use Demo 天气查询助手")
    print("=" * 60)

    # 预加载城市编码表
    load_city_codes()

    # 初始化对话历史
    conversation_history = []

    print("\n💬 开始对话（输入 'exit' 或 'quit' 退出）\n")

    # 持续对话循环
    while True:
        try:
            user_input = input("你: ").strip()

            # 退出条件
            if user_input.lower() in ['exit', 'quit', '退出', '再见']:
                print("\n👋 再见！")
                break

            # 空输入跳过
            if not user_input:
                continue

            # 调用 AI Agent 并保持对话历史
            conversation_history = real_ai_agent_chat(user_input, conversation_history)

        except KeyboardInterrupt:
            print("\n\n👋 对话已中断，再见！")
            break
        except Exception as e:
            print(f"\n❌ 发生错误: {e}")
            print("继续对话...\n")

    print("\n" + "=" * 60)
    print("Demo completed!")
    print("=" * 60)
