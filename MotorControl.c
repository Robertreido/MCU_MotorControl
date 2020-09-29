/∗∗∗ Include Files ∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗/

#include <t89c51ac3.h> 
#include <string .h> 
#include "phys340libkeil .h" 
#include <stdio.h>
/∗∗∗ Declare Vaiables ∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗/ 
int char RPMread [33];
int char Error [33];
int char input [33];
int previous =0;
int current = 0;
int time = 0 ;
int speed = 0;
int in;
int speeds[5];
int err;
int intergral ;
int derivative ;
int RPM;
int prev_error = 0;
int prevT;

void MyIntHandler(void) interrupt 6{  //external interrupt address set 
  EA=0; //disable interupts
  if (CCF0 == 1){                 //only if specific interrupt occurs
    current = (CCAP0H << 8)| CCAP0L; //16bit value for time
    if (current > previous){
    time = current − previous ;//calculate time between interrupts 
    previous = current ; //update value
    }
    else{
    time = 0xFF00 − previous + current;       //time between interrupts for past overflow 
    previous = current ; //update value
     }
  }
  CF = 0; //clears overflow flag
  CCF0 = 0; //clear PCA capture interupts 
  EA = 1; //Enable all interupts
}
void MotorIn(int in){
  CCON = 0x40 ;    //Turn on PCA
  CCAPM2 = 0x42 ;    //Enable compare module and PWM 
  CCAP2H = 255 − in;    //set PWM from 0x00 − 0xFF4
    //255 − speed as stores down from 255, stores from right to left in the address
 }
 
 void captInter (){  
  EA = 1; //enable interupts
  EC = 1; //enable PCA interupt 
  CR = 1;//turn PCA timer on
  CMOD = 0x01; //enable CF bit in CCOn register to generate interrupt
  CCAPM0 = 0x21 ; // positive edge triggered , ECCF enabled 
  }
  
int slid_avg(int speeds [5]){
  int k;
  int sum = 0; //sum of all values
  int average = 0; //declare average value to be returned
  for(k = 0; k < 5; k++){ //iterate through array of values and increment sum
    sum = sum + speeds[k];
   }
    average = sum/5; // calculate average 
    return average ; //return average
  }
/∗∗∗ Main Function ∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗/ 
void main(){
  int i;
      //declare the float variables for the controller , floats as < 1 
  float Kp=0.3; //Proportional Variable
  float Ki=0.1; //Intergral Variable
  float Kd=0.05; //Derivative Variable
  initLCD() ; // initialise LCD display
  captInter(); // initialise interupts
  in = P2; //set the input value from the P2 Port
  
  while (1){
    int Target =P2;//set target to input from I/O module
            //set the the intergral back to zero every time the input speed changes
    if(Target != prevT){
     intergral = 0; //set to zero
     prevT = Target ; // store previous speed
    }
    for(i =0; i<=4;i++){
      speeds [ i]=speeds [ i +1]; //increment array of speeds
    }
    speeds[4] = time; //add new value
    
    RPM = 125000/slid_avg(speeds);//take average and calculate RPM
    if (RPM <= 20){RPM=125000/time;}//dont use average below 20 as it causes errors
    err = Target − RPM; //calculate error value
    intergral = intergral + err ; //calculate intergral , value adds on from previous 
    derivative = err − prev_error; //calclate derivative term between errors
    
    
    if (derivative < 0){derivative =0;} //limit der term to zero so cant go ngtve 
    //if(intergral > 3000){intergral =3000;} //limit intgl term to stop int windup
    // calculate the control value
    
    
    in = (Kp ∗ err) + ( Ki ∗ intergral)+(Kd∗derivative);
    if(Target <=15){in=Target;RPM=Target;} //limit PID to 15rpm as below causes errors
    if (Target <=10){in=0;RPM=0;} //limit speed to 10 RPM as doesnt go below this .


    MotorIn(in); //set speed with PID control 
    prev_error = err ; //set previous error for next loop
    sprintf(RPMread,"RPM: %d ", RPM); //Covert RPM to int
    sprintf (input ,"Tgt: %d " , Target) ; //Convert Target to int 
    sprintf(Error,"Err: %d", Target−RPM); //Calcualte Error and display as int
    // Write Display , delay , then clear
    writeLineLCD (RPMread) ;
    writeLineLCD ( input ) ;
    writeLineLCD ( Error ) ;
    delaya (10000) ;
    clearLCD() ;
  } 
}
