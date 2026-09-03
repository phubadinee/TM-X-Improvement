#include <Wire.h>
#include <Adafruit_GFX.h>
#include <DIYables_OLED_SSD1309.h>
#include <Servo.h>

// --- ตั้งค่าจอ OLED 1309 ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C  // ต่อ SDA ขา 20 และ SCL ขา 21 ของ Mega 2560
DIYables_OLED_SSD1309 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- ตั้งค่าขา Rotary Encoder ---
#define CLK_PIN 2
#define DT_PIN 3
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
const char* mainMenu[] = { "1. Operation", "2. PM", "3. Calibration" };
const int mainMenuSize = sizeof(mainMenu) / sizeof(mainMenu[0]);

const char* opMenu[] = { "< Back", "System Homing", "Start", "Detect Part", "Align Part", "Trig & Wait TM-X", "Transition Push", "Sort Execute", "Emergency Halt" };
const int opMenuSize = sizeof(opMenu) / sizeof(opMenu[0]);

const char* pmMenu[] = { "< Back", "Manual Jogging", "IO Testing", "Dry Run" };
const int pmMenuSize = sizeof(pmMenu) / sizeof(pmMenu[0]);

const char* calMenu[] = { "< Back", "Actuator Stroke", "Sorter Offset", "Servo Transition" };
const int calMenuSize = sizeof(calMenu) / sizeof(calMenu[0]);


// Sensor
const int buzzerPin = 53;
const int servoPin = 26;
int limit_servo = 30;
int limit_servo_state = 0;
Servo myServo;
int servo_stop = 90;
int servo_forward = 50;
int servo_backward = 100;

const int st188Pin = A1;

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

  // เริ่มต้นจอ OLED
  if (!display.begin(SSD1309_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1309 allocation failed"));
    for (;;)
      ;
  }

  // myServo.write(90);
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

  // 3. อัปเดตหน้าจอเฉพาะเมื่อตำแหน่งเคอร์เซอร์หรือเมนูเปลี่ยนเท่านั้น (ลดภาระ I2C)
  if (cursorIndex != lastCursorIndex || currentMenu != lastMenu) {
    updateDisplay();
    lastCursorIndex = cursorIndex;
    lastMenu = currentMenu;
  }

  // int limit_servo_val = digitalRead(limit_servo);
  // Serial.println(limit_servo_val);

  // int st188_val = analogRead(st188Pin);
  // // Serial.print("ST188 Value : ");        
  // // Serial.println(st188_val);        
  // int st188_val_map = map(st188_val, 0, 1023, 0, 100);
  // Serial.print("ST188 Value Map : ");        
  // Serial.println(st188_val_map); 
  // delay(100);
}
