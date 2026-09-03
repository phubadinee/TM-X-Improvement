void system_homing(){
    // When start/stop system
    // Linear, Servo, Stepper Homing
}

void start(){
    // Press start from Physical/Web button
}

void detect_part(){
    // Wait part
    // Read digital/analog from ST188
    // Debounce for confirm part

    /* 

    while (digitalRead(ST188) == 0){
       return 0
    }
    return 1
    
    */
}

void manual_detect_part(){
    // Wait part
    // Read digital/analog from ST188
    // Debounce for confirm part

    /* 

    while (digitalRead(switch) == 0){
       return 0
    }
    return 1
    
    */
}

void align_part(){
    // Actuator
}

void trigger_and_wait_tmx(){
    // Send cmd for trigger TM-X
}

void transition_push(){
    // Servo running
}

void sort_execute(int result){
    // Input result (OK=1/NG=0)
}

void emergency_halt(){
    // Emergency Interrupt
}