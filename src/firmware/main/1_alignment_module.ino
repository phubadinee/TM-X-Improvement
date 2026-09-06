int read_st188(){
  int st188_val = analogRead(st188Pin);
  // Serial.print("ST188 Value : ");        
  // Serial.println(st188_val);        
  int st188_val_map = map(st188_val, 0, 1023, 0, 100);
  Serial.print("ST188 Value Map : ");        
  Serial.println(st188_val_map); 

  return st188_val_map;
}

int runDetectPart(){
  showActionMessage("Detecting Part...");
  Serial.println("======= [Start] Detecting Part =======");
  delay(1000);
  int detect_val = read_st188(); 

  while (detect_val >= 80){
    showActionMessage("No Part...");
    detect_val = read_st188();
//    Serial.println(detect_val);
  }
  showActionMessage("Part Detected !!!");
  Serial.println("======= [End] Detecting Part =======");Serial.println();
  beep();
  delay(1000);

  // return detect_status;
}

void runAlignPart() {
  showActionMessage("Aligning Part...");  
  Serial.println("======= [Start] Part aligned in center =======");
  load_beep();
  Serial.println("======= [End] Part aligned in center =======");Serial.println();
}
