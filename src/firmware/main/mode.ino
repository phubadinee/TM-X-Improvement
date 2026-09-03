// void standalone_run(){
//     // Run 1 cycle without sensor detecting

//     system_homing();
//     start();
//     manual_detect_part();
//     align_part();
//     trigger_and_wait_tmx();
//     transition_push();
//     sort_execute(1);
// }

// void main_run(){
//     // Run from information in Dashboard
    
//     system_homing();
//     start();
//     while (detect_part() == 0);
//     align_part();
//     trigger_and_wait_tmx();
//     transition_push();
//     sort_execute(1);
// }