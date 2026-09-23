void loadAllFromEEPROM() {
  // 1. Sorter Positions (Address 0)
  if (EEPROM.read(0) == EEPROM_INIT_FLAG) {
    EEPROM.get(1, positions[0]);
    EEPROM.get(5, positions[1]);
    EEPROM.get(9, positions[2]);
    EEPROM.get(13, positions[3]);
  } else {
    EEPROM.update(0, EEPROM_INIT_FLAG);
    EEPROM.put(1, positions[0]);
    EEPROM.put(5, positions[1]);
    EEPROM.put(9, positions[2]);
    EEPROM.put(13, positions[3]);
  }

  // 2. Actuator Stroke (Address 20)
  if (EEPROM.read(EEPROM_ADDR_ACTUATOR) == EEPROM_INIT_FLAG) {
    EEPROM.get(EEPROM_ADDR_ACTUATOR + 1, POS_RETRACTED);    
    EEPROM.get(EEPROM_ADDR_ACTUATOR + 5, POS_EXTEND_SHORT); 
    EEPROM.get(EEPROM_ADDR_ACTUATOR + 9, POS_EXTEND_MID);   
    EEPROM.get(EEPROM_ADDR_ACTUATOR + 13, POS_EXTEND_LONG); 
  } else {
    EEPROM.update(EEPROM_ADDR_ACTUATOR, EEPROM_INIT_FLAG); 
    EEPROM.put(EEPROM_ADDR_ACTUATOR + 1, POS_RETRACTED);
    EEPROM.put(EEPROM_ADDR_ACTUATOR + 5, POS_EXTEND_SHORT);
    EEPROM.put(EEPROM_ADDR_ACTUATOR + 9, POS_EXTEND_MID);
    EEPROM.put(EEPROM_ADDR_ACTUATOR + 13, POS_EXTEND_LONG);
  }

  // 3. Auto Sort Setting (Address 40)
  if (EEPROM.read(EEPROM_ADDR_AUTOSORT) == EEPROM_INIT_FLAG) {
    EEPROM.get(EEPROM_ADDR_AUTOSORT + 1, autoSortEnabled);
  } else {
    EEPROM.update(EEPROM_ADDR_AUTOSORT, EEPROM_INIT_FLAG);
    EEPROM.put(EEPROM_ADDR_AUTOSORT + 1, autoSortEnabled);
  }

  // 4. ST188 Threshold (Address 50)
  if (EEPROM.read(EEPROM_ADDR_ST188) == EEPROM_INIT_FLAG) {
    EEPROM.get(EEPROM_ADDR_ST188 + 1, st188Threshold);
  } else {
    EEPROM.update(EEPROM_ADDR_ST188, EEPROM_INIT_FLAG);
    EEPROM.put(EEPROM_ADDR_ST188 + 1, st188Threshold);
  }

  Serial.println("Loaded All Settings from EEPROM");
}

void saveAllToEEPROM() {
  // 1. Sorter Positions
  EEPROM.update(0, EEPROM_INIT_FLAG); 
  EEPROM.put(1, positions[0]);        
  EEPROM.put(5, positions[1]);        
  EEPROM.put(9, positions[2]);        
  EEPROM.put(13, positions[3]);       

  // 2. Actuator Stroke
  EEPROM.update(EEPROM_ADDR_ACTUATOR, EEPROM_INIT_FLAG); 
  EEPROM.put(EEPROM_ADDR_ACTUATOR + 1, POS_RETRACTED);
  EEPROM.put(EEPROM_ADDR_ACTUATOR + 5, POS_EXTEND_SHORT);
  EEPROM.put(EEPROM_ADDR_ACTUATOR + 9, POS_EXTEND_MID);
  EEPROM.put(EEPROM_ADDR_ACTUATOR + 13, POS_EXTEND_LONG);

  // 3. Auto Sort Setting
  EEPROM.update(EEPROM_ADDR_AUTOSORT, EEPROM_INIT_FLAG);
  EEPROM.put(EEPROM_ADDR_AUTOSORT + 1, autoSortEnabled);

  // 4. ST188 Threshold
  EEPROM.update(EEPROM_ADDR_ST188, EEPROM_INIT_FLAG);
  EEPROM.put(EEPROM_ADDR_ST188 + 1, st188Threshold);

  Serial.println("Saved All Settings to EEPROM");
}
