#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"


// start: Servo configurtaion macros

#define ZeroDeg 0
#define NinetyDeg 90
#define OneEightyDeg 180

// end: Servo configuration macros



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

// set OC8 and OC7 to work in PWM mode
//command x and y of servo are connected to OC8 and OC7

void setupOutputCompare()
{
    //sets up OC8 to work in PWM mode and be controlled by Timer 2
    //sets up OC7 to work in PWM mode and be controlled by Timer 3
    //OC8,OC8 pin will be set to high for n ms every 20ms.
    // 0.9ms = 0 degrees
    // 1.5ms = 90 degrees
    // 2.1ms = 180 degrees
    
    //setup Timer 2
    CLEARBIT(T2CONbits.TON); // Disable Timer
    CLEARBIT(T2CONbits.TCS); // Select internal instruction cycle clock
    CLEARBIT(T2CONbits.TGATE); // Disable Gated Timer mode
    TMR2 = 0x00; // Clear timer register
    T2CONbits.TCKPS = 0b10; // Select 1:64 Prescaler
    CLEARBIT(IFS0bits.T2IF); // Clear Timer2 interrupt status flag
    CLEARBIT(IEC0bits.T2IE); // Disable Timer2 interrupt enable control bit
    PR2 = 4000; // Set timer period 20ms:
    // 4000= 40*10^-3 * 12.8*10^6 * 1/64
    
   
    //setup Timer 3
    CLEARBIT(T3CONbits.TON); // Disable Timer
    CLEARBIT(T3CONbits.TCS); // Select internal instruction cycle clock
    CLEARBIT(T3CONbits.TGATE); // Disable Gated Timer mode
    TMR3 = 0x00; // Clear timer register
    T3CONbits.TCKPS = 0b10; // Select 1:64 Prescaler
    CLEARBIT(IFS0bits.T3IF); // Clear Timer3 interrupt status flag
    CLEARBIT(IEC0bits.T3IE); // Disable Timer3 interrupt enable control bit
    PR3 = 4000; // Set timer period 20ms:
    // 4000= 40*10^-3 * 12.8*10^6 * 1/64
    
    
    //setup OC8
    CLEARBIT(TRISDbits.TRISD8); /* Set OC8 as output */
    //OC8R = 1000; /* Set the initial duty cycle to 5ms*/
    //OC8R = 180; /* Set the initial duty cycle to 0.9ms*/
    OC8R = 300; /* Set the initial duty cycle to 1.5ms*/
    OC8RS = 1000; /* Load OCRS: next pwm duty cycle */
    OC8CON = 0x0006; /* Set OC8: PWM, no fault check, Timer2 */
    SETBIT(T2CONbits.TON); /* Turn Timer 2 on */
    
    //setup OC7
    CLEARBIT(TRISDbits.TRISD7); /* Set OC7 as output */
    //CLEARBIT(TRISDbits.TRISD7); /* Set OC8 as output */
    //OC7R = 1000; /* Set the initial duty cycle to 5ms*/
    //OC7R = 180; /* Set the initial duty cycle to 0.9ms*/
    OC7R = 300; /* Set the initial duty cycle to 1.5ms*/
    OC7RS = 1000; /* Load OCRS: next pwm duty cycle */
    OC7CON = 0x0006; /* Set OC7: PWM, no fault check, Timer2 */
    SETBIT(T3CONbits.TON); /* Turn Timer 3 on */
    
}

// servoNum: 0 for X and 1 for Y
void setupServo(int servoNum)
{
    setupOutputCompare();
}

// duty cycle in microsecs
void setDutyCycle(int servoNum, int dutyCycle)
{
    
}




// end: Servo configuration functions

// start: Screen config

void initTouchScreen()
{
    CLEARBIT(PORTEbits.RE1);
    CLEARBIT(PORTEbits.RE2);
    CLEARBIT(PORTEbits.RE3);
}

void touchModeX()
{
    CLEARBIT(PORTEbits.RE1);
    SETBIT(PORTEbits.RE2);
    SETBIT(PORTEbits.RE3);
}

void touchModeY()
{
    SETBIT(PORTEbits.RE1);
    CLEARBIT(PORTEbits.RE2);
    CLEARBIT(PORTEbits.RE3);
}

void touchModeStandby()
{
    SETBIT(PORTEbits.RE1);
    SETBIT(PORTEbits.RE2);
    CLEARBIT(PORTEbits.RE3);
}

void initADC()
{
    CLEARBIT(AD1CON1bits.ADON); //disable ADC

    SETBIT(TRISEbits.TRISE8); //set TRISE AN9 to input
    CLEARBIT(AD1PCFGLbits.PCFG15);

    CLEARBIT(AD1CON1bits.AD12B);
    AD1CON1bits.FORM = 0;   //set integer output
    AD1CON1bits.SSRC = 0x7; //automatic conversion

    AD1CON2 = 0; //no scan sampling

    CLEARBIT(AD1CON3bits.ADRC); // internal clock source
    AD1CON3bits.SAMC = 0x1F;    //sample to conversion clock = 31 tad
    AD1CON3bits.ADCS = 0x2;

    SETBIT(AD1CON1bits.ADON); //enable ADC
}

void initADC2()
{
    CLEARBIT(AD2CON1bits.ADON); //disable ADC

    SETBIT(TRISEbits.TRISE8); //set TRISE AN9 to input
    CLEARBIT(AD2PCFGLbits.PCFG9);

    CLEARBIT(AD2CON1bits.AD12B);
    AD2CON1bits.FORM = 0;   //set integer output
    AD2CON1bits.SSRC = 0x7; //automatic conversion

    AD1CON2 = 0; //no scan sampling

    CLEARBIT(AD2CON3bits.ADRC); // internal clock source
    AD2CON3bits.SAMC = 0x1F;    //sample to conversion clock = 31 tad
    AD2CON3bits.ADCS = 0x2;

    SETBIT(AD2CON1bits.ADON); //enable ADC
}

void readADC()
{
    AD1CHS0bits.CH0SA = 0x014;
    SETBIT(AD1CON1bits.SAMP);
    while (!AD1CON1bits.DONE)
        ;
    CLEARBIT(AD1CON1bits.DONE);
    return ADC1BUF0;
}

void readADC2()
{
    AD2CHS0bits.CH0SA = 0x014;
    SETBIT(AD2CON1bits.SAMP);
    while (!AD2CON1bits.DONE);
    CLEARBIT(AD2CON1bits.DONE);
    return ADC1BUF0;
}

// end: Screen config



void main(){
	//Init LCD
	__C30_UART=1;	
	lcd_initialize();
	lcd_clear();
	lcd_locate(0,0);
	lcd_printf("Hello World!");	
	
	while(1){
		
	}
}

