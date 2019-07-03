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


//LPF: Y(n) = (1-ﬂ)*Y(n-1) + (ﬂ*X(n))) = Y(n-1) - (ﬂ*(Y(n-1)-X(n)));
//cut off freq at 60Hz
void lowPass(int input){
    filtered = filtered - (getAlpha(60) * (filtered - input));
}


// gain values
double kp = 0.2;
double ki = 0.01;
double kd = 0.02;

double setPoint;

//unsigned long currentTime = 0, previousTime;
double elapsedTime;
double error;
double lastError;
double input, output, setPoint;
double cumError, rateError;

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
    lowPass(value);
    return filtered;
}


void setupPID(){
        setPoint = 1700;                          
        
        error = 0.0f;
        lastError =  1000.0f;  // TODO: get actual last error
        elapsedTime = 0.05;    // TODO: get actual elapsed time
}    

 
double computePID(double inp){
       
       error = setPoint - inp;   
       
       if(abs(error - lastError)  > 600)
           error = lastError;
       
       //  feedback
                   
       cumError += error * elapsedTime;                // compute integral
       rateError = (error - lastError)/elapsedTime;   // compute derivative
 
       double out = kp*error + ki*cumError + kd*rateError;                //PID output               
 
       lastError = error;                                //remember current error
       error = 0;
       cumError = 0;
       rateError = 0;
       
       return abs(out);                                        //have function return the PID output
}

// 20 ms
void __attribute__((interrupt)) _T3Interrupt (void) {
    
     // TODO: check deadline
    
    float value = readFiltered(touchDirection);
    int duty =  computePID(value);
    
    float valuey = readFiltered(1);
    int dutyy =  computePID(valuey);
    
  // setDutyCycle(CH_X, dutyy); 
   setDutyCycle(CH_Y, duty); 
    
    lcd_locate(0,5);
    lcd_printf("%d", duty);
    
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
    setupPID();
   

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
