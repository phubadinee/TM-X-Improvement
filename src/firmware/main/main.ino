#include <Wire.h>
#include <U8g2lib.h>
#include <Servo.h>
#include <AccelStepper.h>
#include <EEPROM.h>

// ==============================================================================
// [1] PIN DEFINITIONS (Hardware Mapping) - กำหนดขาใช้งาน
// ==============================================================================
// -- Rotary Encoder & Buttons --
#define CLK_PIN 2           // TRA (Rotary)
#define DT_PIN 3            // TRB (Rotary)
#define SW_PIN 4            // Rotary Button (Enter)
#define STOP_BTN_PIN 5      // Stop / Back Button
const int intEmer = 19;     // Emergency Stop Button (Interrupt)

// -- Sensors & IO --
const int st188Pin = A1;    // ST188 Sensor (Detect Part)
const int buzzerPin = 53;   // Buzzer

// -- Stepper Motor (Sorter) --
const int enPin = 16;       // Stepper Enable
const int stepPin = 17;     // Stepper Step
const int dirPin = 18;      // Stepper Direction
const int limit_sorter = 25;// Stepper Homing Limit Switch

// -- Linear Actuator (Align Part) --
const int feedbackPin = A3; // Actuator Feedback (Analog)
const int rcPin = 7;        // Actuator PWM Signal

// -- Small Servo (Transition Push) --
const int servoPin = 11;    // Small Servo PWM
const int limit_servo = 24; // Servo Limit Switch

// ==============================================================================
// [2] HARDWARE OBJECTS - ประกาศอุปกรณ์
// ==============================================================================
U8G2_SSD1309_128X64_NONAME2_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
AccelStepper stepper(1, stepPin, dirPin);
Servo actuator;
Servo myServo;

// ==============================================================================
// [3] SYSTEM SETTINGS & CALIBRATION - สำหรับตั้งค่าการทำงานต่างๆ
// ==============================================================================
// --- EEPROM ---
const int EEPROM_INIT_FLAG = 0xAA;
const int EEPROM_ADDR_ACTUATOR = 20;

// --- Linear Actuator: ระยะยืด-หด (ตั้งค่าผ่านหน้าจอได้) ---
int POS_RETRACTED    = 600;   // ระยะหดสุด (กลับจุดเริ่มต้น)
int POS_EXTEND_SHORT = 300;   // ระยะยืดสั้น (สำหรับกล่องใหญ่ 9x9)
int POS_EXTEND_MID   = 150;   // ระยะยืดปานกลาง (สำหรับกล่องกลาง 7x7)
int POS_EXTEND_LONG  = 50;    // ระยะยืดยาวสุด (สำหรับกล่องเล็ก 4x4)

// --- Linear Actuator: ความเร็วและเวลา ---
const int fastInterval = 1;       // ความเร็วช่วงขยับไว (ตัวเลขน้อย = เร็ว)
const int slowInterval = 5;       // ความเร็วช่วงขยับช้า
const int fastExtendStep = 10;    // แต้ม PWM ที่บวกเพิ่มตอนยืดเร็ว
const int slowExtendStep = 1;     // แต้ม PWM ที่บวกเพิ่มตอนยืดช้า (เพื่อความนุ่มนวล)
const int fastRetractStep = 30;   // แต้ม PWM ที่ลดลงตอนหดกลับเร็ว
const unsigned long pauseDuration = 500; // เวลาหยุดพักเมื่อยืดสุด (มิลลิวินาที)

// --- Package Groups: กลุ่มขนาดกล่อง (เพิ่ม/ลด ขนาดกล่องที่นี่) ---
// --- Package Communication Variable ---
String lastReceivedPkg = "---";

String pkgGroup1[] = {"9x9", "10x10"};   // ใช้ระยะยืด Short
int sizeGroup1 = 2;

String pkgGroup2[] = {"7x7", "8x8", "5x5"};     // ใช้ระยะยืด Mid
int sizeGroup2 = 3;

String pkgGroup3[] = {"4x5"};     // ใช้ระยะยืด Long
int sizeGroup3 = 1;

// --- Small Servo: ความเร็วและองศาการผลัก ---
int servo_stop = 90;
int servo_forward_slow = 50;
int servo_forward_fast = 0;
int servo_backward = 120;

// --- Stepper Sorter: ตำแหน่งแต่ละช่อง ---
long positions[4] = {0, 885, 1500, 2400}; // ตำแหน่งช่องที่ 1, 2, 3, 4


// ==============================================================================
// [4] RUNTIME VARIABLES - ตัวแปรระบบภายใน (ไม่จำเป็นต้องแก้ไข)
// ==============================================================================
// --- Menu Content ---
const char* mainMenu[] = { "1. Start", "2. System Homing", "3. Calibration", "4. PM", "5. Module Function" };
const int mainMenuSize = sizeof(mainMenu) / sizeof(mainMenu[0]);

const char* calMenu[] = { "Actuator Stroke", "Sorter Offset"};
const int calMenuSize = sizeof(calMenu) / sizeof(calMenu[0]);

const char* pmMenu[] = {"IO Testing", "Manual Jogging", "Commu Testing", "Dry Run" };
const int pmMenuSize = sizeof(pmMenu) / sizeof(pmMenu[0]);

const char* modMenu[] = {
  "Detect Part", "Align Part", "Align Retract",
  "Align Extend Short", "Align Extend Mid", "Align Extend Long",
  "Trig & Wait TM-X", "Transition Push", "Sort Execute"
};
const int modMenuSize = sizeof(modMenu) / sizeof(modMenu[0]);

// --- Menu Navigation States ---
int currentMenu = 0;
int cursorIndex = 0;
int lastCursorIndex = -1;
int lastMenu = -1;
int scrollOffset = 0;
const int maxVisibleItems = 4;

// --- Rotary Encoder States ---
int currentStateCLK;
int lastStateCLK;
unsigned long lastRotaryTime = 0;
const unsigned long debounceDelay = 5;

// --- Sorter Counting ---
int slotCounts[4] = {0, 0, 0, 0};

// --- Actuator State Machine ---
int servoPWM = 998;
unsigned long lastMoveTime = 0;
unsigned long pauseStartTime = 0;
enum State { FAST_EXTEND, SLOW_EXTEND, PAUSE, FAST_RETRACT };
State currentState = FAST_EXTEND;

// --- Servo IO States ---
int limit_servo_state = 0;

// --- Communication States ---
unsigned long lastSendTime = 0;

void setup() {
  Serial.begin(115200);

  //  digitalWrite(RESET_PIN, HIGH);
  //  pinMode(RESET_PIN, OUTPUT);

  digitalWrite(enPin, HIGH);
  pinMode(enPin, OUTPUT);

  pinMode(limit_sorter, INPUT_PULLUP);
  stepper.setMaxSpeed(800);
  stepper.setAcceleration(400);

  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);
  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(STOP_BTN_PIN, INPUT_PULLUP);

  digitalWrite(buzzerPin, LOW); // ปิด Buzzer ก่อนเสมอ
  pinMode(buzzerPin, OUTPUT);

  pinMode(limit_servo, INPUT);
  pinMode(st188Pin, INPUT_PULLUP);
  pinMode(intEmer, INPUT_PULLUP);

  loadPositionsFromEEPROM();
  loadActuatorFromEEPROM();

  myServo.write(servo_stop); // ⚠️ สั่งตำแหน่งเริ่มต้นก่อน attach (กันเซอร์โวดีด)
  myServo.attach(servoPin);

  // Linear Actuator
  servoPWM = 998;
  actuator.writeMicroseconds(servoPWM);
  actuator.attach(rcPin);

  u8g2.begin();
  updateDisplay();
  lastStateCLK = digitalRead(CLK_PIN); // อ่านค่าเริ่มต้นหลังไฟนิ่งแล้ว

  //  attachInterrupt(digitalPinToInterrupt(intEmer), runEmergencyHalt, FALLING);

  for_beep_fast();
  runHoming();
  for_beep_fast();

  Serial.println("======================= MCU Started =======================");
}

void loop() {


  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); // อ่านข้อความจนเจอการขึ้นบรรทัดใหม่
    command.trim(); // ตัดช่องว่างหรือตัวอักษรซ่อน (เช่น \r) ทิ้ง

    if (command == "<START>") {
      Serial.println("Command Received: Starting Machine...");
      runStart(); // เรียกฟังก์ชัน Start ทันที

      // หลังจากรันจบ บังคับให้หน้าจอกลับมาวาดเมนูเดิมอีกครั้ง
      lastCursorIndex = -1;
    }
  }

  handleRotaryMenu();

  if (digitalRead(SW_PIN) == LOW) {
    delay(50);  // รอสัญญาณนิ่ง
    if (digitalRead(SW_PIN) == LOW) {
      executeMenuAction();
      while (digitalRead(SW_PIN) == LOW)
        ;  // รอจนกว่าจะปล่อยปุ่ม
      delay(50);
    }
  }

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

  if (cursorIndex != lastCursorIndex || currentMenu != lastMenu) {
    updateDisplay();
    lastCursorIndex = cursorIndex;
    lastMenu = currentMenu;
  }
}
