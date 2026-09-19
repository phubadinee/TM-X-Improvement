void loadPositionsFromEEPROM() {
  // ตรวจสอบว่าเคยเซฟค่าไว้หรือไม่ (เช็คไบต์แรก)
  if (EEPROM.read(0) == EEPROM_INIT_FLAG) {
    EEPROM.get(1, positions[0]);
    EEPROM.get(5, positions[1]);
    EEPROM.get(9, positions[2]);
    EEPROM.get(13, positions[3]);
    Serial.println("Loaded Stepper Positions from EEPROM");
  } else {
    Serial.println("No EEPROM data found, writing defaults.");
    savePositionsToEEPROM();
  }
}

void savePositionsToEEPROM() {
  EEPROM.update(0, EEPROM_INIT_FLAG); // บันทึก Flag
  EEPROM.put(1, positions[0]);        // เริ่มที่ Address 1 (Long ใช้ 4 bytes)
  EEPROM.put(5, positions[1]);        // เริ่มที่ Address 5
  EEPROM.put(9, positions[2]);        // เริ่มที่ Address 9
  EEPROM.put(13, positions[3]);       // เริ่มที่ Address 13
  Serial.println("Saved Stepper Positions to EEPROM");
}

void loadActuatorFromEEPROM() {
  // ตรวจสอบว่าเคยเซฟค่าไว้หรือไม่ (เช็ค Flag ที่ Address 20)
  if (EEPROM.read(EEPROM_ACTUATOR_START) == EEPROM_INIT_FLAG) {
    EEPROM.get(EEPROM_ACTUATOR_START + 1, POS_RETRACTED);   // อ่านค่าที่ 1
    EEPROM.get(EEPROM_ACTUATOR_START + 5, POS_SLOW_START);  // อ่านค่าที่ 2 
    EEPROM.get(EEPROM_ACTUATOR_START + 9, POS_EXTENDED);    // อ่านค่าที่ 3
    Serial.println("Loaded Actuator Positions from EEPROM");
  } else {
    Serial.println("No Actuator EEPROM data found, writing defaults.");
    saveActuatorToEEPROM(); // ถ้ายังไม่เคยเซฟ ให้เซฟค่า Default ปัจจุบันลงไปเลย
  }
}

void saveActuatorToEEPROM() {
  EEPROM.update(EEPROM_ACTUATOR_START, EEPROM_INIT_FLAG); // บันทึก Flag ลง Address 20
  
  // ใช้ EEPROM.put แบบเว้นระยะ 4 Bytes (เผื่อขนาดตัวแปร) เหมือนของ Sorter
  EEPROM.put(EEPROM_ACTUATOR_START + 1, POS_RETRACTED);   // เริ่มที่ Address 21
  EEPROM.put(EEPROM_ACTUATOR_START + 5, POS_SLOW_START);  // เริ่มที่ Address 25
  EEPROM.put(EEPROM_ACTUATOR_START + 9, POS_EXTENDED);    // เริ่มที่ Address 29
  
  Serial.println("Saved Actuator Positions to EEPROM");
}
