import pandas as pd

# 读取 Excel 文件查看结构
excel_path = r"F:\Data\UE_Project\FlowSolo\FlowAndEditorTool\AIAgent\Agent_Learning\Tool\AMap_adcode_citycode.xlsx"
df = pd.read_excel(excel_path)

print("Excel 列名：")
print(df.columns.tolist())
print("\n前 5 行数据：")
print(df.head())
print(f"\n总行数：{len(df)}")
