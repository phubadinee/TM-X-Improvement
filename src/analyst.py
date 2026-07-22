import pandas as pd
import numpy as np  # <-- เพิ่มบรรทัดนี้เข้ามา

# ==========================================
# 1. กำหนดค่าพิกัด min / max ที่คุณต้องการเช็ค (แก้ตัวเลขตรงนี้ได้เลยครับ)
x_min_val = 0.0
x_max_val = 10.0
y_min_val = 0.0
y_max_val = 10.0
# ==========================================

src = 'BC48C000.xlsx'
excel_file = pd.ExcelFile(src)

# sheet_names_focus = ['DM  4x4 YOK', 'DM 4x4 JTI 7953 5026', 'DM 5x5   SAWN ', 'DM 3x3.5  yokowo ', 'DM 3.5X3.75  EQT', 'DM 3X3 QFN YOKOWO', 'DM 3X3 QFN SAWN JFM', 'DM ALPL 3.5X5', 'DM 4.25X4.25 EQT ', 'DM 5X5 QFN DIMPLE', 'DM 6X6 QFN', 'DM 8X8   ', 'DM 3.5X3.5', 'DM 3.5X6', 'ALPL DM 5X6', 'ALPL DM4.5x5.75', 'DM  3.5X4', 'DM  4X5', 'DM  7X7', 'V6V8 6X6 JTI', 'V6V8 5X5 0.75 JTI SAWN ', 'V6V8 7X7 0.75 JTI SAWN', 'V6V8 3X3 0.75 JTI SAWN', 'V6V8 4X4 0.75  ', 'V6V8 3.5X5.5 0.75JTI SAWN ROL1 ', 'ALPL HT 4X4 ', 'ALPL Dual  AE04 3.5x3.75', 'ALPL HT 6X6 ', 'HT 6X6 DEMPLE', 'ALPL HT DUAL 3.5X3.75 ', 'HT 3X3 DUAL', 'ALPL HT Dual 6.55X4.3', 'ALPL HT 3.5X4.6', 'HT 3X2.5', 'HT DUAL3.25X7.4', 'HT 3X4', 'ALPL HT 5X5 DUAL  ', 'HT 5X5 PUNCH 0.08', 'ALPL HT dual 4x5', 'HT 5X5 DEMPLE  0.08 ', 'ALPL HT Dual 10x6.5', 'ALPL HT Dual 7x7', 'ALPL HT Dual3.05x7.25', 'ALPL HT 4.5x5.75', 'ALPL DUAL 8x8 1X4', 'ALPL HT Dual 8X8', 'ALPL HT Dual 9x15 ', 'ALPL HT Dual 9x9', 'HT MX 3X3', 'HT MX 4x4', 'HT MX 5X5', 'HT MX 7X7', 'HT MX 8X8', 'HT MX 3.5X4.6', 'HT MX 9x9', 'MT 9510 ALPL 8X8 YKW ', 'ALPL 9510 8X8JTI', 'MT 9510 ALPL 7X7 YKW', 'ALPL 9510 7x7 JTI', 'ALPL 9510 9X9 YKW  ', 'ALPL 9510 5X5 ', 'ALPL6X6  9510', 'ALPL SRM 3x3', 'ALPL SRM 2x2 (2)', 'ALPL SRM 2.1x1.6 ', 'ALPL SRM 2x3 ', 'ALPL9928 4X4', 'ALPL9928 3X3']

count_total=0
count_fail=0
# for sheet in sheet_names_focus:
    
sheet = input("name : ")
df = pd.read_excel(excel_file, sheet_name=sheet)

column_names = df.columns.tolist()
print(column_names[3])

if "วันที่วัด" in column_names:
    df = df.drop(columns="วันที่วัด")
    
df = df.dropna(subset=["x", "y"])

# บังคับ x, y ให้เป็นตัวเลขเพื่อป้องกัน Error
df['x'] = pd.to_numeric(df['x'], errors='coerce')
df['y'] = pd.to_numeric(df['y'], errors='coerce')


xy = input("Input Condition (x_min x_max y_min y_max): ")
if xy != "s":    
    xy_list = xy.split(" ")
    
    if len(xy_list) == 2:
        x_min = float(xy_list[0])
        x_max = float(xy_list[1])
        y_min = float(xy_list[0])
        y_max = float(xy_list[1])
    else:
        # กำหนดค่าเป็น float ก่อนนำไปใช้งาน
        x_min = float(xy_list[0])
        x_max = float(xy_list[1])
        y_min = float(xy_list[2])
        y_max = float(xy_list[3])

    # เช็คเงื่อนไข between
    condition = df["x"].between(x_min, x_max) & df["y"].between(y_min, y_max)


    # ให้คะแนน good / fail
    df['res'] = np.where(condition, 'good', 'fail')
    
    # ดึงคอลัมน์ที่ 4 (Index 3) มาเทียบ
    col_name = df.columns[3]
    df['check_match'] = np.where(df[col_name] == df['res'], 1, 0)

    # นับจำนวน
    count_all = df['check_match'].count()
    count_ones = df['check_match'].sum()
    
    count_total+=count_all
    count_fail+=(count_all - count_ones)
    
    print(f"📍 Sheet: '{sheet}'")
    print(f"Compare: '{col_name}' vs 'res'")
    print(f"  ✅ Correct   : {count_ones}")
    print(f"  ❌ Incorrect : {count_all - count_ones}")
    print("-" * 40)
else:
    pass

print("Error rate = ", (count_fail/count_total)*100,"%")