
void bend (float angle) {     //bender pin movement
  if (angle!=0){              //sets direction of bend based on + or - angle
    Serial.print("Bending ");
    Serial.println(angle);
    boolean dir=cw;
    boolean back=ccw;
    if (angle<0){
      dir = ccw;
      back = cw;
    }
    float rotations = 0;
    angle = abs(angle);
/*
    if (angle <= 90){
      angle = -.0012*angle*angle+.5959*angle+.2452; //converts angle into calibrated motor steps 
      angle = 6000 * (angle/360)+220;
    }
    else if (90 < angle && angle <= 120){
      angle = .0044*angle*angle-.5481*angle+57.981; //converts angle into calibrated motor steps 
      angle = 5960 * (angle/360)+220;
    }
    else if (120 < angle && angle <=180){
      angle = angle-63.26;  //converts angle into calibrated motor steps 
      angle = 5960 * (angle/360)+220;
    }   //calibration will differ depending on set up 
    rotations = angle;
*/ 
 rotations = angle * BEND_MOTOR_STEPS_PER_DEGREE;

    Serial.print ("Steps: ");
    Serial.println (rotations);
    
    digitalWrite (bendMotorDir, dir);  
    for (int i=0; i < rotations  ; i++){ //moves bender bin the desired angle
      digitalWrite(bendMotorPls, HIGH);
      delayMicroseconds (pulseWidth);
      digitalWrite(bendMotorPls, LOW);
      delayMicroseconds (pulseDelay);
    }
    delay (100);
    digitalWrite (bendMotorDir, back);   //moves bender pin back to home position ready for next feed
    for (int i=0; i < rotations  ; i++){
      digitalWrite(bendMotorPls, HIGH);
      delayMicroseconds (pulseWidth);
      digitalWrite(bendMotorPls, LOW);
      delayMicroseconds (pulseDelay);
    }
  }
}//bend ()



//////
void rotatePin (boolean dir, float angle) { //moves bender pin during duck. direction specified from duck subroutine
  float rotations = 0;
   Serial.print("Rotating ");
   Serial.println(angle);
  
/*  angle = 6000 * (angle/360); //converts angle to steps
  rotations = angle;
*/

rotations = angle * BEND_MOTOR_STEPS_PER_DEGREE;
   Serial.print ("Steps: ");
   Serial.println (rotations);
  
  digitalWrite (bendMotorDir, dir);  
  for (int i=0; i <=rotations  ; i++){ //rotates bender motor appropriate number of steps
    digitalWrite(bendMotorPls, HIGH);
    delayMicroseconds (pulseWidth);
    digitalWrite(bendMotorPls, LOW);
    delayMicroseconds (pulseDelay);
  }
}//rotatePin ()


//////
void feed (float dist) { //feeds wire
  if (dist != 0){
    Serial.print("Feeding ");
    Serial.println(dist);
    
    float rotations = 0;
    /*
    float feedCalib = 25.4*PI; //feed mm per revolution of motor
    dist = 2000 * dist/feedCalib;  //dist converted from mm to steps
    rotations = dist;
    */
    rotations = dist * FEED_MOTOR_STEPS_PER_MILIMETER;
    Serial.print ("Steps: ");
    Serial.println (rotations);
    
    digitalWrite (feedMotorDir, 1);  //feed motor only moves forward
    for (int i=0; i < rotations  ; i++){ //rotate feed motor appropriate number of steps 
      digitalWrite(feedMotorPls, HIGH);
      delayMicroseconds (pulseWidth);
      digitalWrite(feedMotorPls, LOW);
      delayMicroseconds (pulseDelay);
    }
  }
}//feed(()


//////
void duck (){  //ducks bender pin under wire
  digitalWrite (benderPin, HIGH); 
  delay (200);
  rotatePin (curDir, 30);
  digitalWrite (benderPin, LOW);  //pin down move under wire
  curDir = !curDir;    //direction reversed for next duck
}//duck()



