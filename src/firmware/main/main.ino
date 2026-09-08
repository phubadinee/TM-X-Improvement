#include <Wire.h>
#include <U8g2lib.h>
#include <Servo.h>

// --- ตั้งค่าจอ OLED 1309 ด้วย U8g2 (ใช้ Hardware I2C, Full Buffer) ---
// ต่อ SDA ขา 20 และ SCL ขา 21 ของ Mega 2560
U8G2_SSD1309_128X64_NONAME2_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// --- ตั้งค่าขา Rotary Encoder ---
#define CLK_PIN 2  // TRA
#define DT_PIN 3   // TRB
#define SW_PIN 4

// --- ตัวแปรควบคุมเมนู ---
int currentMenu = 0;
int cursorIndex = 0;
int lastCursorIndex = -1;
int lastMenu = -1;
int scrollOffset = 0;
const int maxVisibleItems = 4;

// --- ตัวแปรสำหรับเช็คสถานะ Rotary Encoder ---
int currentStateCLK;
int lastStateCLK;
unsigned long lastRotaryTime = 0;       // เพิ่มตัวแปรเก็บเวลา Debounce
const unsigned long debounceDelay = 5;  // หน่วงเวลา 5 มิลลิวินาทีป้องกันสัญญาณสั่น

// --- โครงสร้างเมนู ---
// --- โครงสร้างเมนู ---
// ลำดับที่ 0 และ 1 จะเป็นคำสั่งทำงานทันที ส่วนลำดับที่ 2, 3, 4 จะเข้า Sub-menu
const char* mainMenu[] = { "1. Start", "2. System Homing", "3. Calibration", "4. PM", "5. Module Function" };
const int mainMenuSize = sizeof(mainMenu) / sizeof(mainMenu[0]);

const char* calMenu[] = { "< Back", "Actuator Stroke", "Sorter Offset", "Servo Transition" };
const int calMenuSize = sizeof(calMenu) / sizeof(calMenu[0]);

const char* pmMenu[] = { "< Back", "Manual Jogging", "IO Testing", "Comm Testing", "Dry Run" };
const int pmMenuSize = sizeof(pmMenu) / sizeof(pmMenu[0]);

const char* modMenu[] = { "< Back", "Detect Part", "Align Part", "Trig & Wait TM-X", "Transition Push", "Sort Execute" };
const int modMenuSize = sizeof(modMenu) / sizeof(modMenu[0]);

// Sensor
const int buzzerPin = 53;
const int servoPin = 26;
int limit_servo = 30;
int limit_servo_state = 0;
Servo myServo;
int servo_stop = 90;
int servo_forward_slow = 50;
int servo_forward_fast = 0;
int servo_backward = 100;

const int st188Pin = A1;

// Communication
unsigned long lastSendTime = 0;

void setup() {
  Serial.begin(115200);

  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);
  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(limit_servo, INPUT);
  pinMode(st188Pin, INPUT_PULLUP);

  myServo.attach(servoPin);

  // อ่านค่าสถานะเริ่มต้นของ CLK
  lastStateCLK = digitalRead(CLK_PIN);

  // เริ่มต้นจอ OLED ด้วย U8g2
  u8g2.begin();

  updateDisplay();
  for_beep();
}

void loop() {
  handleRotaryMenu();

  // 2. ตรวจจับการกดปุ่ม (SW) แบบหน่วงเวลาป้องกันการกดเบิ้ล
  if (digitalRead(SW_PIN) == LOW) {
    delay(50);  // รอสัญญาณนิ่ง
    if (digitalRead(SW_PIN) == LOW) {
      executeMenuAction();
      while (digitalRead(SW_PIN) == LOW)
        ;  // รอจนกว่าจะปล่อยปุ่ม
      delay(50);
    }
  }

  // 3. อัปเดตหน้าจอเฉพาะเมื่อตำแหน่งเคอร์เซอร์หรือเมนูเปลี่ยนเท่านั้น
  if (cursorIndex != lastCursorIndex || currentMenu != lastMenu) {
    updateDisplay();
    lastCursorIndex = cursorIndex;
    lastMenu = currentMenu;
  }


  //   int detect_val = read_st188();
  //   while (detect_val >= 80){
  //     showActionMessage("No Part...");
  //     detect_val = read_st188();
  // //    Serial.println(detect_val);
  //   }
  //   beep();
  //   runTrigWaitTMX();
}
