#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <stdlib.h>
#include <libpic30.h>

#include "lcd.h"
#include "touch.h"
#include "servo.h"


#define SAMPLE_RATE 50

#include <time.h>




///////////////////////////////////////////////////////////////////////////////

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT); 

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);  

///////////////////////////////////////////////////////////////////////////////




void initTimer3(){

        CLEARBIT(T3CONbits.TON); // Disable Timer
         // Notes: the system clock operates at 12.8Mhz
        CLEARBIT(T3CONbits.TCS); // Select internal instruction cycle clock
        CLEARBIT(T3CONbits.TGATE); // Disable Gated Timer mode
        TMR3 = 0x00; // Clear timer register
        T3CONbits.TCKPS = 0b11; // Select 1:256 Prescaler
        // Notes: (period * prescale) / clock freq. = actual time in second
        // (25000 * 256) / 12800000 = 0,500    // 500 ms
        // (10000 * 256) / 12800000 = 0,020    // 20 ms
        // 20 ms
        PR3 = 1000; // Load the period value
        IPC2bits.T3IP = 0x02; // Set Timer3 Interrupt Priority Level
        CLEARBIT(IFS0bits.T3IF); // Clear Timer3 Interrupt Flag
        SETBIT(IEC0bits.T3IE); // Enable Timer3 interrupt
        SETBIT(T3CONbits.TON); // Start Timer
        
}


float filtered;
float constant = 1.25;

float getAlpha(float cutofFreq){
    float RC = 1.0/(cutofFreq * 2);
    float dt = 1.0/SAMPLE_RATE;
    float alpha = dt/(RC+dt);

    return alpha;
}


//LPF: Y(n) = (1-ß)*Y(n-1) + (ß*X(n))) = Y(n-1) - (ß*(Y(n-1)-X(n)));
//cut off freq at 60Hz
void lowPass(int input){
    filtered = filtered - (getAlpha(60) * (filtered - input));
}


// gain values
double kpX = 0.2;
double kiX = 0.01;
double kdX = 0.02;

double kpY = 0.2;
double kiY = 0.01;
double kdY = 0.02;

double setPointX;
double setPointY;

//unsigned long currentTime = 0, previousTime;
double elapsedTime;

double errorX;
double errorY;

double lastErrorX,lastErrorY;
double cumErrorX, rateErrorX;

double cumErrorY, rateErrorY;

int touchDirection = 0;





int readRaw(int direction){
    setTouchMode(direction);
    __delay_ms(10);
    int data = readADC();
    //touchDirection = !touchDirection;
    
    return data;
}

float readFiltered(int direction){
    int value = readRaw(direction);
    int i = 0;
    int sum =0;
    for (i =0; i<5; i++){
         lowPass(value);
         sum += filtered;
    }
    return sum / 5;
}


void setupPID(uint8_t servoNum){
    
      if(servoNum == CH_X)
    {
        setPointX = 1700;                          
        
        errorX = 0.0f;
        lastErrorX =  1000.0f;  // TODO: get actual last error
     
      }
      
      else   if(servoNum == CH_Y)
    {
        setPointY = 1700;                          
        
        errorY = 0.0f;
        lastErrorY =  1000.0f;  // TODO: get actual last error
     
      }
      
         elapsedTime = 0.05;    // TODO: get actual elapsed time
}    

 
double computePID(uint8_t servoNum, double inp){
    
    if(servoNum == CH_X)
    {
       
       errorX = setPointX - inp;   
       
       if(abs(errorX - lastErrorX)  > 600)
           errorX = lastErrorX;
       
       //  feedback
                   
       cumErrorX += errorX * elapsedTime;                // compute integral
       rateErrorX = (errorX - lastErrorX)/elapsedTime;   // compute derivative
 
       double outX = kpX*errorX + kiX*cumErrorX + kdX*rateErrorX;                //PID output               
 
       lastErrorX = errorX;                                //remember current error
       errorX = 0;
       cumErrorX = 0;
       rateErrorX = 0;
       
       return abs(outX);                                        //have function return the PID output
    }
    
    else if(servoNum == CH_Y)
    {
       
       errorY = setPointY - inp;   
       
       if(abs(errorY - lastErrorY)  > 600)
           errorY = lastErrorY;
       
       //  feedback
                   
       cumErrorY += errorY * elapsedTime;                // compute integral
       rateErrorY = (errorY - lastErrorY)/elapsedTime;   // compute derivative
 
       double outY = kpY*errorY + kiY*cumErrorY + kdY*rateErrorY;                //PID output               
 
       lastErrorY = errorY;                                //remember current error
       errorY = 0;
       cumErrorY = 0;
       rateErrorY = 0;
       
       return abs(outY);                                        //have function return the PID output
    }
}

int temp=0;

// 20 ms
void __attribute__((interrupt)) _T3Interrupt (void) {
    
     // TODO: check deadline
    
    float value = readFiltered(touchDirection);
    int duty =  computePID(CH_X,value);
    
    float valuey = readFiltered(1);
    int dutyy =  computePID(CH_Y,valuey);
    
  setDutyCycle(CH_X, dutyy); 
   setDutyCycle(CH_Y, duty); 
    
   if(temp % 2 == 0)
   { lcd_locate(0,5);
    lcd_printf("%d", duty);}
   
  
    
    IFS0bits.T3IF = 0; // Clear Timer3 Interrupt Flag   
   
}


void main(){
	//Init LCD
	__C30_UART=1;	
	lcd_initialize();
	lcd_clear();
	
    //TODO: ----------------- init adc -----------------

    initADC();
    initTouchScreen();
    
     // ----------------- init servo -----------------
    setupServo(CH_X); 
    setupServo(CH_Y); 
    
    
    initTimer3();
    setupPID(CH_X);
      setupPID(CH_Y);
   

	while(1){
        //ZERO = 45 NINETY = 75 ONEEIGHTY = 105
      
        float value = readFiltered(touchDirection);
        lcd_locate(0,2);
        lcd_printf("%.02f", value);
        __delay_ms(500);
        
        //read from rotary encoder connected to A0
  
	}
}



/* PID example AR DU AI NO
 
 //PID constants
double kp = 2
double ki = 5
double kd = 1
 
unsigned long currentTime, previousTime;
double elapsedTime;
double error;
double lastError;
double input, output, setPoint;
double cumError, rateError;
 
void setup(){
        setPoint = 0;                          //set point at zero degrees
}    
 
void loop(){
        input = analogRead(A0);                //read from rotary encoder connected to A0
        output = computePID(input);
        delay(100);
        analogWrite(3, output);                //control the motor based on PID value
 
}
 
double computePID(double inp){     
        currentTime = millis();                //get current time
        elapsedTime = (double)(currentTime - previousTime);        //compute time elapsed from previous computation
        
        error = Setpoint - inp;                                // determine error
        cumError += error * elapsedTime;                // compute integral
        rateError = (error - lastError)/elapsedTime;   // compute derivative
 
        double out = kp*error + ki*cumError + kd*rateError;                //PID output               
 
        lastError = error;                                //remember current error
        previousTime = currentTime;                        //remember current time
 
        return out;                                        //have function return the PID output
}
 
 */
