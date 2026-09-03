#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <ESP32Servo.h>
#include <Stepper.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- กำหนดขาอุปกรณ์ (Hardware Pins) ---
// Stepper Motor 28BYJ-48 (ผ่าน Driver เช่น ULN2003)
const int stepsPerRevolution = 2048; 
Stepper myStepper(stepsPerRevolution, 19, 18, 5, 17); // ขา IN1, IN2, IN3, IN4

// Micro Servo
Servo transitionServo;
const int servoPin = 13; // ขาควบคุม Servo บน ESP32

// --- ตัวแปรสำหรับเก็บค่า Calibration ---
int calibStepperSteps = 512; // ค่าเริ่มต้นจำนวนสเต็ป
int calibServoAngle = 90;    // ค่าเริ่มต้นมุมของ Micro Servo

// --- EEPROM Addresses ---
const int EEPROM_SIZE = 512;
const int ADDR_STEPPER = 0;   
const int ADDR_SERVO = 10;    

// --- Menu Structure ---
int currentMenu = 0; // 0: Main Menu, 1: Operation, 2: PM, 3: Calibration

void setup() {
  Serial.begin(115200);
  
  // เริ่มต้นใช้งาน EEPROM
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println(F("failed to initialise EEPROM"));
    delay(1000);
  }

  // โหลดค่าที่เคยบันทึกไว้ใน EEPROM
  loadCalibrationData();

  // ตั้งค่าอุปกรณ์
  myStepper.setSpeed(25); // ปรับความเร็ว Stepper ให้เร็วขึ้น (28BYJ-48 เหมาะที่สุดช่วง 20-25 RPM ไม่ให้ตกร่อง)
  transitionServo.attach(servoPin);
  transitionServo.write(0); // เซ็ตตำแหน่งเริ่มต้น Servo ที่ 0 องศา

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  printHelp();
  updateDisplay();
}

void loop() {
  if (Serial.available() > 0) {
    char key = Serial.read();
    
    // กรองตัวอักษรขึ้นบรรทัดใหม่
    if (key == '\n' || key == '\r') return; 
    
    handleInput(key);
  }
}

// --- ฟังก์ชันจัดการ EEPROM ---
void loadCalibrationData() {
  int storedStep;
  int storedAngle;
  
  EEPROM.get(ADDR_STEPPER, storedStep);
  EEPROM.get(ADDR_SERVO, storedAngle);

  if (storedStep > 0 && storedStep <= 4096) {
    calibStepperSteps = storedStep;
  }
  if (storedAngle >= 0 && storedAngle <= 180) {
    calibServoAngle = storedAngle;
  }
}

void saveCalibrationData() {
  EEPROM.put(ADDR_STEPPER, calibStepperSteps);
  EEPROM.put(ADDR_SERVO, calibServoAngle);
  EEPROM.commit(); 
  Serial.println(">> Calibration Data Saved to EEPROM successfully!");
}

void printHelp() {
  Serial.println("\n========================================");
  Serial.println(" TM-X CONTROL MENU (Type number & Enter)");
  Serial.println("========================================");
  if (currentMenu == 0) {
    Serial.println(" [1] Operation");
    Serial.println(" [2] PM (Preventive Maintenance)");
    Serial.println(" [3] Calibration");
  } else if (currentMenu == 1) {
    Serial.println(" [1] system_homing()");
    Serial.println(" [2] start()");
    Serial.println(" [3] detect_part() [Uses 28BYJ-48 Stepper]");
    Serial.println(" [4] align_part()");
    Serial.println(" [5] trigger_and_wait_tmx()");
    Serial.println(" [6] transition_push() [Uses Micro Servo]");
    Serial.println(" [7] sort_execute()");
    Serial.println(" [8] emergency_halt()");
    Serial.println(" [0] < Back to Main Menu");
  } else if (currentMenu == 2) {
    Serial.println(" [1] manual_jogging_mode()");
    Serial.println(" [2] io_testing()");
    Serial.println(" [3] dry_run()");
    Serial.println(" [0] < Back to Main Menu");
  } else if (currentMenu == 3) {
    Serial.print(" [1] set_actuator_stroke_lim() [Current Steps: ");
    Serial.print(calibStepperSteps); Serial.println("]");
    Serial.println(" [2] set_sorter_offset()");
    Serial.print(" [3] set_servo_transition() [Current Angle: ");
    Serial.print(calibServoAngle); Serial.println(" deg]");
    Serial.println(" [0] < Back to Main Menu");
  }
  Serial.println("----------------------------------------");
}

void handleInput(char key) {
  int choice = key - '0';

  if (currentMenu == 0) {
    if (choice >= 1 && choice <= 3) {
      currentMenu = choice;
      updateDisplay();
      printHelp();
    } else {
      Serial.println(">> Invalid Choice! Please select 1-3.");
    }
  } 
  else {
    if (choice == 0) {
      currentMenu = 0; // กลับหน้าหลัก
      updateDisplay();
      printHelp();
    } 
    else {
      // เมนู Calibration (รับค่าพารามิเตอร์ใหม่)
      if (currentMenu == 3) {
        // เคลียร์ค่าขยะใน Serial Buffer ก่อนเริ่มรับข้อความใหม่
        while (Serial.available() > 0) {
          Serial.read();
        }

        if (choice == 1) {
          Serial.print(">> Enter new Stepper Steps (Current: ");
          Serial.print(calibStepperSteps);
          Serial.println("): ");
          
          // รอรับค่าจากผู้ใช้ พร้อมระบบ Timeout ป้องกันค้าง
          unsigned long startTime = millis();
          while (Serial.available() == 0) {
            if (millis() - startTime > 15000) { // 15 วินาที
              Serial.println(">> Input Timeout!");
              return;
            }
          }
          
          int newSteps = Serial.parseInt();
          while (Serial.available() > 0) Serial.read(); // ล้างบรรทัดใหม่ที่ค้างอยู่

          if (newSteps > 0) {
            calibStepperSteps = newSteps;
            saveCalibrationData();
            Serial.print(">> Updated Stepper Steps to: "); Serial.println(calibStepperSteps);
          }
        } 
        else if (choice == 3) {
          Serial.print(">> Enter new Servo Angle 0-180 (Current: ");
          Serial.print(calibServoAngle);
          Serial.println("): ");
          
          unsigned long startTime = millis();
          while (Serial.available() == 0) {
            if (millis() - startTime > 15000) {
              Serial.println(">> Input Timeout!");
              return;
            }
          }
          
          int newAngle = Serial.parseInt();
          while (Serial.available() > 0) Serial.read(); // ล้างบรรทัดใหม่ที่ค้างอยู่

          if (newAngle >= 0 && newAngle <= 180) {
            calibServoAngle = newAngle;
            saveCalibrationData();
            transitionServo.write(calibServoAngle);
            delay(500);
            transitionServo.write(0); // ทดสอบหมุนไปแล้วกลับมา 0
            Serial.print(">> Updated & Tested Servo Angle to: "); Serial.println(calibServoAngle);
          } else {
            Serial.println(">> Invalid Angle! Must be between 0 and 180.");
          }
        } 
        else if (choice == 2) {
          Serial.println(">> set_sorter_offset() executed.");
        }
      } 
      // เมนู Operation สั่งรันฟังก์ชันจริง
      else if (currentMenu == 1) {
        Serial.print(">> Executing: ");
        switch (choice) {
          case 1: Serial.println("system_homing()"); break;
          case 2: Serial.println("start()"); break;
          case 3: 
            Serial.println("detect_part() -> Running Stepper 28BYJ-48");
            detect_part_action(); 
            break;
          case 4: Serial.println("align_part()"); break;
          case 5: Serial.println("trigger_and_wait_tmx()"); break;
          case 6: 
            Serial.println("transition_push() -> Running Micro Servo");
            transition_push_action(); 
            break;
          case 7: Serial.println("sort_execute()"); break;
          case 8: Serial.println("emergency_halt()"); break;
          default: Serial.println("Unknown Command!"); break;
        }
      } 
      // เมนู PM
      else if (currentMenu == 2) {
        Serial.print(">> Executing: ");
        switch (choice) {
          case 1: Serial.println("manual_jogging_mode()"); break;
          case 2: Serial.println("io_testing()"); break;
          case 3: Serial.println("dry_run()"); break;
          default: Serial.println("Unknown Command!"); break;
        }
      }
      
      showPopup("Executed!", 1000);
      updateDisplay();
      printHelp();
    }
  }
}

// --- ฟังก์ชันการทำงานจริงในโหมด Operation ---

void detect_part_action() {
  myStepper.setSpeed(25); // ตั้งความเร็วสูงสุดที่มอเตอร์ 28BYJ-48 ทำงานได้เสถียร
  
  Serial.print("-> Stepper moving forward by "); Serial.print(calibStepperSteps); Serial.println(" steps.");
  myStepper.step(calibStepperSteps); // หมุนจากจุดเริ่มต้นไปตามระยะที่ตั้งไว้
  delay(1000);
  
  Serial.println("-> Stepper returning to home (0 steps).");
  myStepper.step(-calibStepperSteps); // หมุนกลับตำแหน่งเริ่มต้น
}

void transition_push_action() {
  Serial.println("-> Servo moving from 0 to target angle.");
  transitionServo.write(0);             // เริ่มต้นที่ 0 องศา
  delay(200);
  transitionServo.write(calibServoAngle); // หมุนไปยังองศาที่ตั้งค่าไว้
  delay(500);                           // รอให้ทำงานเสร็จ
  
  Serial.println("-> Servo returning to 0 degrees.");
  transitionServo.write(0);             // กลับมาที่ตำแหน่ง 0 องศา
  delay(500);
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  if (currentMenu == 0) {
    display.print("--- MAIN MENU ---");
    display.setCursor(0, 16); display.print("1. Operation");
    display.setCursor(0, 28); display.print("2. PM");
    display.setCursor(0, 40); display.print("3. Calibration");
  } 
  else if (currentMenu == 1) {
    display.print("--- OPERATION ---");
    display.setCursor(0, 12); display.print("1:Homing  5:Trg&Wait");
    display.setCursor(0, 22); display.print("2:Start   6:Trans");
    display.setCursor(0, 32); display.print("3:Detect  7:Sort");
    display.setCursor(0, 42); display.print("4:Align   8:Emergency");
    display.setCursor(0, 54); display.print("0: Back");
  } 
  else if (currentMenu == 2) {
    display.print("--- P.M. MODE ---"); // Fix minor typo if needed, keeping display strings clean
    display.setCursor(0, 16); display.print("1. Manual Jogging");
    display.setCursor(0, 28); display.print("2. IO Testing");
    display.setCursor(0, 40); display.print("3. Dry Run");
    display.setCursor(0, 52); display.print("0. Back");
  } 
  else if (currentMenu == 3) {
    display.print("--- CALIBRATION ---");
    display.setCursor(0, 16); display.print("1. Act. Steps: "); display.print(calibStepperSteps);
    display.setCursor(0, 28); display.print("2. Sorter Offset");
    display.setCursor(0, 40); display.print("3. Servo Deg: "); display.print(calibServoAngle);
    display.setCursor(0, 52); display.print("0. Back");
  }
  
  display.display();
}

void showPopup(String msg, int delayMs) {
  display.fillRect(24, 22, 80, 20, SSD1306_BLACK);
  display.drawRect(24, 22, 80, 20, SSD1306_WHITE);
  display.setCursor(36, 28);
  display.print(msg);
  display.display();
  delay(delayMs);
}