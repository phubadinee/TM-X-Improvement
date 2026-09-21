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
  if (EEPROM.read(EEPROM_ADDR_ACTUATOR) == EEPROM_INIT_FLAG) {
    EEPROM.get(EEPROM_ADDR_ACTUATOR + 1, POS_RETRACTED);    
    EEPROM.get(EEPROM_ADDR_ACTUATOR + 5, POS_EXTEND_SHORT); 
    EEPROM.get(EEPROM_ADDR_ACTUATOR + 9, POS_EXTEND_MID);   
    EEPROM.get(EEPROM_ADDR_ACTUATOR + 13, POS_EXTEND_LONG); 
    Serial.println("Loaded Actuator Stroke from EEPROM");
  } else {
    Serial.println("No Actuator EEPROM data found, writing defaults.");
    saveActuatorToEEPROM(); 
  }
}

void saveActuatorToEEPROM() {
  EEPROM.update(EEPROM_ADDR_ACTUATOR, EEPROM_INIT_FLAG); 
  EEPROM.put(EEPROM_ADDR_ACTUATOR + 1, POS_RETRACTED);
  EEPROM.put(EEPROM_ADDR_ACTUATOR + 5, POS_EXTEND_SHORT);
  EEPROM.put(EEPROM_ADDR_ACTUATOR + 9, POS_EXTEND_MID);
  EEPROM.put(EEPROM_ADDR_ACTUATOR + 13, POS_EXTEND_LONG);
  Serial.println("Saved Actuator Stroke to EEPROM");
}
