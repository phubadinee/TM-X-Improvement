import cv2
import numpy as np

# =====================================================================
# CONFIGURATION SECTION (ปรับแต่งค่าทั้งหมดได้ที่นี่)
# =====================================================================

# 1. ค่าการวัดที่ต้องการใส่เอง (Manual Input Values)
MANUAL_VALUES = {
    "1": "5.017 mm",  # ความกว้างสี่เหลี่ยมกลาง
    "2": "5.026 mm",  # ความสูงสี่เหลี่ยมกลาง
    "3": "0.540 mm",  # วงกลมกลางซ้าย
    "4": "0.520 mm",  # วงกลมกลางขวา
}

# 2. การจัดตำแหน่งค่าการวัด (Position Offsets)
POS_CONFIG = {
    # ตำแหน่งตัวอักษร [1] (เทียบจากจุดกึ่งกลางสี่เหลี่ยมกลาง)
    "inner_1_text_offset_x": -45,
    "inner_1_text_offset_y": 100,
    # ตำแหน่งตัวอักษร [2] (เทียบจากจุดกึ่งกลางสี่เหลี่ยมกลาง)
    "inner_2_text_offset_x": 80,
    "inner_2_text_offset_y": 4,
    # ตำแหน่งตัวอักษรวงกลมข้าง [7], [8] (เทียบจากจุดศูนย์กลางวงกลม)
    "circle_text_offset_x": -40,
    "circle_text_offset_y": -40,
    # ระยะร่นขอบลูกศรตรงกลาง (ยิ่งค่ามาก ลูกศรจะยิ่งสั้นลง ไม่ให้ทะลุขอบ)
    "inner_arrow_inset_x": 18,
    "inner_arrow_inset_y": 18,
}

# 3. รูปแบบการแสดงผลและฟอร์แมต (Formatting & Styles)
STYLE_CONFIG = {
    "font_scale": 0.4,  # ขนาดตัวอักษร
    "font_thickness": 1,  # ความหนาตัวอักษร
    "line_thickness": 1,  # ความหนาของเส้นกรอบและเส้นวัด
    "arrow_tip_length": 0.08,  # ขนาดหัวลูกศรตรงกลาง
    "color_green": (0, 255, 0),  # สีเส้นขอบชิ้นงาน (BGR Format: เขียว)
    "color_yellow": (0, 255, 255),  # สีลูกศรวัดระยะตรงกลาง (BGR Format: เหลือง)
    "color_white": (255, 255, 255),  # สีข้อความตัวหนังสือ (BGR Format: ขาว)
}

# =====================================================================
# MAIN PROGRAM (ระบบประมวลผลหลัก)
# =====================================================================


def process_and_measure(image_path):
  # โหลดภาพ
  original_img = cv2.imread(image_path)
  if original_img is None:
    print(f"Error: ไม่พบไฟล์ {image_path}")
    return

  gray = cv2.cvtColor(original_img, cv2.COLOR_BGR2GRAY)

  # ทำ Threshold เพื่อแยกชิ้นงาน
  _, thresh = cv2.threshold(
      gray, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU
  )
  kernel = np.ones((3, 3), np.uint8)
  thresh = cv2.morphologyEx(thresh, cv2.MORPH_OPEN, kernel, iterations=1)

  contours, _ = cv2.findContours(
      thresh, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE
  )
  output_img = cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)

  detected_square = None
  inner_square = None
  image_center_x = original_img.shape[1] // 2
  image_center_y = original_img.shape[0] // 2

  # ค้นหาสี่เหลี่ยมหลัก และสี่เหลี่ยมตรงกลาง
  for cnt in contours:
    area = cv2.contourArea(cnt)
    if area > 5000:
      x, y, w_box, h_box = cv2.boundingRect(cnt)
      perimeter = cv2.arcLength(cnt, True)
      approx = cv2.approxPolyDP(cnt, 0.04 * perimeter, True)

      if len(approx) == 4:
        if (
            x < image_center_x < x + w_box
            and y < image_center_y < y + h_box
            and area > 50000
        ):
          detected_square = (x, y, w_box, h_box, cnt)

  for cnt in contours:
    area = cv2.contourArea(cnt)
    if 1000 < area < 25000:
      x, y, w_box, h_box = cv2.boundingRect(cnt)
      if (
          abs((x + w_box / 2) - image_center_x) < 50
          and abs((y + h_box / 2) - image_center_y) < 50
      ):
        perimeter = cv2.arcLength(cnt, True)
        approx = cv2.approxPolyDP(cnt, 0.04 * perimeter, True)
        if len(approx) >= 4:
          inner_square = (x, y, w_box, h_box, cnt)
          break

  # ดึงค่า Style มาใช้งาน
  c_green = STYLE_CONFIG["color_green"]
  c_yellow = STYLE_CONFIG["color_yellow"]
  c_white = STYLE_CONFIG["color_white"]
  f_scale = STYLE_CONFIG["font_scale"]
  f_thick = STYLE_CONFIG["font_thickness"]
  l_thick = STYLE_CONFIG["line_thickness"]
  tip_len = STYLE_CONFIG["arrow_tip_length"]

  # วาดขอบสี่เหลี่ยมใหญ่
  if detected_square:
    _, _, _, _, main_cnt = detected_square
    cv2.drawContours(output_img, [main_cnt], -1, c_green, l_thick)

  # วาดการวัดสี่เหลี่ยมกลางรูป (Cross Arrows)
  if inner_square:
    in_x, in_y, in_w, in_h, in_cnt = inner_square
    cv2.drawContours(output_img, [in_cnt], -1, c_green, l_thick)

    center_x_in = in_x + in_w // 2
    center_y_in = in_y + in_h // 2

    inset_x = POS_CONFIG["inner_arrow_inset_x"]
    inset_y = POS_CONFIG["inner_arrow_inset_y"]

    # [1] ลูกศรแนวนอน (ซ้าย-ขวา)
    cv2.arrowedLine(
        output_img,
        (center_x_in, center_y_in),
        (in_x + inset_x, center_y_in),
        c_yellow,
        l_thick,
        tipLength=tip_len,
    )
    cv2.arrowedLine(
        output_img,
        (center_x_in, center_y_in),
        (in_x + in_w - inset_x, center_y_in),
        c_yellow,
        l_thick,
        tipLength=tip_len,
    )
    cv2.putText(
        output_img,
        f"[1] {MANUAL_VALUES.get('1', '')}",
        (
            center_x_in + POS_CONFIG["inner_1_text_offset_x"],
            center_y_in + POS_CONFIG["inner_1_text_offset_y"],
        ),
        cv2.FONT_HERSHEY_SIMPLEX,
        f_scale,
        c_white,
        f_thick,
    )

    # [2] ลูกศรแนวตั้ง (บน-ล่าง)
    cv2.arrowedLine(
        output_img,
        (center_x_in, center_y_in),
        (center_x_in, in_y + inset_y),
        c_yellow,
        l_thick,
        tipLength=tip_len,
    )
    cv2.arrowedLine(
        output_img,
        (center_x_in, center_y_in),
        (center_x_in, in_y + in_h - inset_y),
        c_yellow,
        l_thick,
        tipLength=tip_len,
    )
    cv2.putText(
        output_img,
        f"[2] {MANUAL_VALUES.get('2', '')}",
        (
            center_x_in + POS_CONFIG["inner_2_text_offset_x"],
            center_y_in + POS_CONFIG["inner_2_text_offset_y"],
        ),
        cv2.FONT_HERSHEY_SIMPLEX,
        f_scale,
        c_white,
        f_thick,
    )

  # วาดวงกลมด้านข้างเฉพาะตำแหน่ง [7] และ [8]
  circles = cv2.HoughCircles(
      gray,
      cv2.HOUGH_GRADIENT,
      dp=1,
      minDist=50,
      param1=150,
      param2=30,
      minRadius=10,
      maxRadius=100,
  )

  if circles is not None and detected_square:
    circles = np.uint16(np.around(circles))
    sq_x, sq_y, sq_w, sq_h, _ = detected_square

    y_min_limit = sq_y + (sq_h * 0.35)
    y_max_limit = sq_y + (sq_h * 0.65)

    for pt in circles[0, :]:
      center_x, center_y, radius = pt[0], pt[1], pt[2]

      if y_min_limit < center_y < y_max_limit:
        cv2.circle(output_img, (center_x, center_y), radius, c_green, l_thick)

        key = "3" if center_x < image_center_x else "4"
        val_str = MANUAL_VALUES.get(key, "")

        cv2.line(
            output_img,
            (center_x - radius, center_y),
            (center_x + radius, center_y),
            c_green,
            l_thick,
        )
        cv2.putText(
            output_img,
            f"[{key}] {val_str}",
            (
                center_x + POS_CONFIG["circle_text_offset_x"],
                center_y + POS_CONFIG["circle_text_offset_y"],
            ),
            cv2.FONT_HERSHEY_SIMPLEX,
            f_scale,
            c_white,
            f_thick,
        )

  # แสดงผลลัพธ์
  cv2.imshow("Configured Measurement Result", output_img)
  cv2.waitKey(0)
  cv2.destroyAllWindows()


# --- รันโปรแกรม ---
process_and_measure("images\\alpl5x5full.png")