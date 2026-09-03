// void servo_setup(){
//   // myServo.setPeriodHertz(50);
//   myServo.attach(servoPin, 500, 2400);
// }

// void servo_pusher(int max_degree, int speed){
//   Serial.println("pusher start");
//   myServo.write(max_degree); 
//   delay(1000); 

//   int i = max_degree;

//   limit_status = limit_sw_Read();
//   while (digitalRead(limit) == 1){
//     myServo.write(i);
//     delay(0); 
    
//     Serial.print("Degree :");
//     Serial.println(i);

//     if (i >= 0){
//       i = i - speed;
//     } else {
//       i = max_degree;
//     }
  
//   }
// }