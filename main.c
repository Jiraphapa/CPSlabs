#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"
#include "touch.h"
#include "servo.h"


#define SAMPLE_RATE 50

#include <time.h>

#include <stdlib.h>


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
        // (25000 * 256) / 12800000 = 0,500    // 500 ms
        // (10000 * 256) / 12800000 = 0,020    // 20 ms
        // 20 ms
        PR3 = 1000; // Load the period value
        IPC2bits.T3IP = 0x02; // Set Timer3 Interrupt Priority Level
        CLEARBIT(IFS0bits.T3IF); // Clear Timer3 Interrupt Flag
        SETBIT(IEC0bits.T3IE); // Enable Timer3 interrupt
        SETBIT(T3CONbits.TON); // Start Timer
        
}

uint16_t NUM_SAMPLES = 5;
uint16_t samples[5];
int j = 0;
float sum = 0;
uint16_t count = 0;
int i = 0;
int commandX[4] = {ZERO,ONEEIGHTY,ONEEIGHTY,ZERO };
int commandY[4] = {ZERO,ZERO, ONEEIGHTY, ONEEIGHTY};

/////////////////////////////      PID stuffs    ///////////////////////////////




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
double kp = 0.1;
double ki = 0.3;
double kd = 0.05;


int countt = 0;

double setPointX;
double setPointY;

//unsigned long currentTime = 0, previousTime;
double elapsedTime;
double error;
double lastError;
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
        setPointX = 1400 + 550; // 920, 550 IS A MAGIC U KNOW?                          
        setPointY = 2202;   
        
        error = 0.0f;
        //lastError =  2493.0f-1400;  // TODO: get actual last error
        elapsedTime = 0.20;    // TODO: get actual elapsed time
}    


double computePID(double inp, uint8_t servoNum){
        if(servoNum == CH_X){
        error = setPointX - inp;   
       }
        else if(servoNum == CH_Y){
        error = setPointY - inp;   
       }


        //     if(abs(error - lastError)  > 600)
        //    error = lastError;

        //  feedback

       cumError += error * elapsedTime;                // compute integral
       rateError = (error - lastError)/elapsedTime;   // compute derivative

       double out = kp*error + ki*cumError + kd*rateError;                //PID output               

       lastError = error;                                //remember current error
       error = 0;
       cumError = 0;
       rateError = 0;
       
       int returnVal = abs(out); // 2 servo should apply same number huhu ö.ö
       if (returnVal > 73)
           returnVal = 73;
       if (returnVal < 68)
           returnVal = 68;
       return returnVal;                                        //have function return the PID output
}

float readLowPass(int direction){
    setTouchMode(direction);
    int init = readADC();
    int data[4] = { init, init, init, init};
    float a[5] = {1.00, 0.7821, 0.6800, 0.1827, 0.0301};
    float b[5] = {0.1672, 0.6687, 1.0031, 0.6687, 0.1672};
    float y[4] = {init,init,init,init};
    float x[5] = {init,init,init,init,init};
    
    int i = 0;
    int j = 0;
    int output;
    int bSide;
    int aSide = 0;
    for (i = 0; i< 5; i++){
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
    for(i = 0; i < 4; i++){
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

////////////////////////////// end:  PID stuffs //////////////////////////////

void __attribute__((interrupt)) _T3Interrupt (void) {
    
   
    
    ///////////////////////  PID tasks
    
    
    float value = readLowPass(1);
    int duty =  computePID(value, CH_X);

  //  float valuey = readLowPass(0);
   // int dutyy =  computePID(valuey, CH_Y);

    setDutyCycle(CH_X, duty); 
    //setDutyCycle(CH_Y, duty); 

    // lcd_locate(0,5);
    // lcd_printf("%d", duty);
    
    if(countt % 10 == 0){
        
      lcd_locate(0,2);
      lcd_printf("%d", duty);
      
      lcd_locate(0,4);
      lcd_printf("%.02f", value);
    }
    countt++; 

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
    
    // --------- PID tasks
    
    
     setupPID();
    
    /*
      float value = readFiltered(touchDirection);
      lcd_locate(0,2);
      lcd_printf("%.02f", value);
     * */
      
      
   
     
      

	while(1){
        //ZERO = 45 NINETY = 75 ONEEIGHTY = 105
		
	}
}
