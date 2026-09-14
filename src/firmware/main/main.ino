#include <Wire.h>
#include <U8g2lib.h>
#include <Servo.h>
#include <AccelStepper.h>
#include <EEPROM.h>


const int EEPROM_INIT_FLAG = 0xAA;

// --- OLED ---
// ต่อ SDA ขา 20 และ SCL ขา 21 ของ Mega 2560
U8G2_SSD1309_128X64_NONAME2_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// --- Rotary Encoder ---
#define CLK_PIN 2  // TRA
#define DT_PIN 3   // TRB
#define SW_PIN 4
#define STOP_BTN_PIN 5 // ปุ่มใหม่สำหรับ Stop / Back (ต่อเข้า Digital 6 กับ GND)

//=================================================================================================
// --- Linear Actuator ---
const int feedbackPin = A4;  // สาย Feedback ต่อเข้า A4
const int rcPin = 44;        // สาย RC (PWM) ต่อเข้า Pin 44
Servo actuator;

const int POS_RETRACTED = 600;  // ค่า Feedback เมื่อหดกลับสุด (สำหรับเริ่มวนลูปใหม่)
const int POS_SLOW_START = 300; // ค่า Feedback ที่เริ่มเปลี่ยนเป็นความเร็วช้า
const int POS_EXTENDED = 100;   // ค่า Feedback เป้าหมายเมื่อยืดสุด

// ==========================================
// 3. กำหนดค่าความเร็วและระยะเวลา (Speed & Timing)
// ==========================================
const int fastInterval = 1;     // ความเร็วช่วงขยับไว (มิลลิวินาทีต่อสเตป)
const int slowInterval = 5;    // ความเร็วช่วงขยับช้า (มิลลิวินาทีต่อสเตป)

const int fastExtendStep = 10;   // แต้ม PWM ที่บวกเพิ่มตอนยืดเร็ว
const int slowExtendStep = 1;   // แต้ม PWM ที่บวกเพิ่มตอนยืดช้า (นุ่มนวล)
const int fastRetractStep = 30;  // แต้ม PWM ที่ลดลงตอนหดกลับเร็ว

const unsigned long pauseDuration = 1000; // เวลาหยุดพักเมื่อยืดสุด (5 วินาที)

// ตัวแปรระบบภายใน
int servoPWM = 998;
unsigned long lastMoveTime = 0;
unsigned long pauseStartTime = 0;

// กำหนดสถานะการทำงาน (State Machine)
enum State { FAST_EXTEND, SLOW_EXTEND, PAUSE, FAST_RETRACT };
State currentState = FAST_EXTEND;
//=================================================================================================

// --- Stepper ---
const int stepPin = 17;
const int dirPin = 18;
const int limit_sorter = 31;
const int enPin = 16;

AccelStepper stepper(1, stepPin, dirPin);

long positions[4] = {0, 885, 1500, 2400}; // ตำแหน่งช่องที่ 1, 2, 3, 4
int slotCounts[4] = {0, 0, 0, 0};          // ตัวนับจำนวนของในแต่ละช่อง (สูงสุด 4 ชิ้น)

//=================================================================================================

// --- Menu ---
int currentMenu = 0;
int cursorIndex = 0;
int lastCursorIndex = -1;
int lastMenu = -1;
int scrollOffset = 0;
const int maxVisibleItems = 4;

// --- Rotary Encoder ---
int currentStateCLK;
int lastStateCLK;
unsigned long lastRotaryTime = 0;       // เพิ่มตัวแปรเก็บเวลา Debounce
const unsigned long debounceDelay = 5;  // หน่วงเวลา 5 มิลลิวินาทีป้องกันสัญญาณสั่น


// --- Menu Structure ---
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
const int servoPin = 11;
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
  pinMode(STOP_BTN_PIN, INPUT_PULLUP); // ตั้งค่าปุ่ม Stop/Back เพิ่มเติม\

  pinMode(enPin, OUTPUT);
  pinMode(limit_sorter, INPUT_PULLUP);
  digitalWrite(enPin, HIGH);
  //
  stepper.setMaxSpeed(800);
  stepper.setAcceleration(400);

  loadPositionsFromEEPROM();

  for_beep_fast();
  runHoming();
  for_beep_fast();
  //
  actuator.attach(rcPin);
  myServo.attach(servoPin);

  //  // อ่านค่าสถานะเริ่มต้นของ CLK
  lastStateCLK = digitalRead(CLK_PIN);
  //
  //  // เริ่มต้นจอ OLED ด้วย U8g2
  u8g2.begin();
  updateDisplay();
  //

  //  actuator.attach(rcPin);

  // เซ็ตตำแหน่งเริ่มต้นที่จุดหดสุด
  servoPWM = 998;
  actuator.writeMicroseconds(servoPWM);
  delay(500);
  //
  for_beep();
}

void loop() {
  //  pauseMotor();
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

  // 2. ตรวจจับการกดปุ่ม STOP/BACK เพิ่มเติม (เมื่ออยู่ใน Sub-menu แล้วกดปุ่มนี้ จะเด้งกลับหน้าหลักทันที)
  if (digitalRead(STOP_BTN_PIN) == LOW) {
    delay(50);
    if (digitalRead(STOP_BTN_PIN) == LOW) {
      if (currentMenu > 0) {
        currentMenu = 0;
        cursorIndex = 0;
        scrollOffset = 0;
        updateDisplay();
      }
      while (digitalRead(STOP_BTN_PIN) == LOW);
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
