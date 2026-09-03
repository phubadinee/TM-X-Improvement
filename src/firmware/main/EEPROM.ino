// #include <EEPROM.h> 

// // Define variables to save
// int sensorThreshold = 850;      // An int takes 2 bytes on the Mega 2560
// float calibrationValue = 1.25;  // A float takes 4 bytes on the Mega 2560

// void setup() {
//   Serial.begin(115200);
  
//   int address = 0; // Start at EEPROM address 0 (range is 0 to 4095 on Mega 2560)

//   // --- WRITING TO EEPROM ---
//   // Store the integer at address 0
//   EEPROM.put(address, sensorThreshold);
  
//   // Shift the address forward by the size of the integer (2 bytes)
//   address += sizeof(sensorThreshold);
  
//   // Store the float at the new address (address 2)
//   EEPROM.put(address, calibrationValue);


//   // --- READING FROM EEPROM ---
//   int readAddress = 0;
//   int loadedThreshold;
//   float loadedCalibration;

//   // Read the integer from address 0
//   EEPROM.get(readAddress, loadedThreshold);
  
//   // Shift the address forward to read the next variable
//   readAddress += sizeof(loadedThreshold);
  
//   // Read the float from address 2
//   EEPROM.get(readAddress, loadedCalibration);

//   // Print results
//   Serial.print("Loaded Threshold: ");
//   Serial.println(loadedThreshold);
  
//   Serial.print("Loaded Calibration: ");
//   Serial.println(loadedCalibration);
// }

// void loop() {
//   // Main program logic
// }