#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"
#include "touch.h"
#include "servo.h"
#include <stdlib.h>
#include <math.h>


#define SPEED 0.08
#define DEADLINE_COUNT 10


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


// start: Servo configuration functions

/*  set OC8 and OC7 to work in PWM mode and be controlled by Timer 2
command x and y of servo are connected to OC8 and OC7
OC8,OC8 pin will be set to high for ... ms every 20ms. */



int posX = 0;
int posY = 0;


void initTimer3(){

        CLEARBIT(T3CONbits.TON); // Disable Timer
         // Notes: the system clock operates at 12.8Mhz
        CLEARBIT(T3CONbits.TCS); // Select internal instruction cycle clock
        CLEARBIT(T3CONbits.TGATE); // Disable Gated Timer mode
        TMR3 = 0x00; // Clear timer register
        T3CONbits.TCKPS = 0b11; // Select 1:256 Prescaler
        // Notes: (period * prescale) / clock freq. = actual time in second
        // (25000 * 256) / 12800000 = 0,500
        PR3 = 1000; // Load the period value
        IPC2bits.T3IP = 0x02; // Set Timer3 Interrupt Priority Level
        CLEARBIT(IFS0bits.T3IF); // Clear Timer3 Interrupt Flag
        SETBIT(IEC0bits.T3IE); // Enable Timer3 interrupt
        SETBIT(T3CONbits.TON); // Start Timer
        
}


float readLowPass(int direction){
    int init = readADC();
    int data[4] = { init, init, init, init};
    //float a[5] = {1.00, 0.7821, 0.6800, 0.1827, 0.0301};
     float a[2] = {0.1602, 0.1602};
    //float b[5] = {0.1672, 0.6687, 1.0031, 0.6687, 0.1672};
    float b[2] = {1.0, -0.6796};
    float y[4] = {0,0,0,0};
    float x[5] = {init,init,init,init,init};
    
    int i = 0;
    int j = 0;
    int output;
    int bSide;
    int aSide = 0;
    for (i = 0; i< 2; i++){
        //butter (4, 30/50)
        
        x[i] = readADC();
        //calculate b side
        bSide += b[i] * x[i];
        
        if (i > 0){
            aSide += a[i] + y[i-1];
        }
        y[i] = bSide - aSide;
       
    }

    float temp;
    float temp2;
    for(i = 0; i < 1; i++){
        if (i == 0){
            temp = y[i];
            y[i] = bSide - aSide;
        }
        
        else{
            temp2 = y[i];
            y[i] = temp;
            temp = temp2;
        }
    }
    return y[3];
    
}

float readLowPass2(int direction){
    float b[2] = {0.1602, 0.1602};
    float a[2] = {1.0000, -0.6796};
    
    int value1= readADC();
    int value2= readADC();
    float y = (b[0] * value1);
    
    return (b[0] * value2) + (b[1] * value1) - (a[1] * y);;
}

double kp = 0.3;
double ki = 0.1;
double kd = 0.02;


double setPointX;
double setPointY;

double centerX;
double centerY;

double radX;
double radY;

//unsigned long currentTime = 0, previousTime;
double elapsedTime;
double error;
double lastError;
double cumError, rateError;

double tick = 0.0;
void setupPID(){
    // before COS wave
       // setPointX = 2500.0f + 350;            
       /// setPointY = 2270.0f+300;   
       // setPointY = 2270.0f+250;   
    // after COS wave
    
        centerX = 1117;
        centerY = 1287; 

        radX = 1;
        radY = 1;
        
        cumError = 0.0f;
        rateError = 0.0f;
        
        error = 0.0f;
        lastError =  300.0f;   // TODO: get actual last error
        elapsedTime = 0.05;    // TODO: get actual elapsed time
}    

int magic = 0;
double computePID(double inp, uint8_t servoNum){
    
    setPointX = centerX + (radX*cos(tick* SPEED));
    setPointY = centerY + (radY*sin(tick* SPEED)); 
    
    //setPointX = 1105;
    //setPointY = 1284;

    float threshold = 1600;
    
     if(servoNum == CH_X){
        error = setPointX - inp;   
        //kp = 0.4;
        //ki = 0.0;
        //kd = 0.0;
        kp = 0.400;
        ki = 0.00;
        kd = 0.000;
    }
        else if(servoNum == CH_Y){
        error = setPointY - inp; 
        kp = 0.45;
        ki = 0.0;
        kd = 0.000;  
       }
        
       
       if(abs(error) > threshold)
            error = lastError;
       
       cumError += error * elapsedTime;               
       rateError = (error - lastError)/elapsedTime;  

       double out = kp*error + ki*cumError + kd*rateError;                        

       lastError = error;                             
       error = 0.0f;
       cumError = 0.0f;
       rateError = 0.0f;
       
       double result = abs(out);
       
      
       // duty trim

       
       return result;                                        //have function return the PID output
}

  int dutyX;
  int dutyY;
void __attribute__((interrupt)) _T3Interrupt (void) {
    
  dutyX = computePID(posX, CH_X);
  dutyY = computePID(posY, CH_Y);
    setDutyCycle(CH_X, dutyX);
   setDutyCycle(CH_Y, dutyY);
    
    tick+=0.1;
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
    setupPID();
    initTimer3();
   
    int printCount = 0;

	while(1){
        //ZERO = 45 NINETY = 75 ONEEIGHTY = 105
        
        setTouchMode(1);
        posX = readLowPass2(1);
		__delay_ms(10);  
        
        setTouchMode(0);
        posY = readLowPass2(0);
        __delay_ms(10);
        
        if (printCount == 10){
            lcd_locate(0,3);
            lcd_printf("%d  %d", posX, dutyX);
            lcd_locate(0,5);
            lcd_printf("%d  %d", posY,dutyY);
            printCount = 0;
        }
        printCount++;
	}
}
