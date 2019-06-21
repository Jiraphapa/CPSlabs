#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"


// start: Servo configurtaion macros

// 4000= 20*10^-3 * 12.8*10^6 * 1/64
#define PERIOD 4000 // timer period 20ms:
// duty cycle = pulse width / period (100% duty cycle = 4000)
// Notes: servo controllers 'invert' the signal from OC8 and OC7

// 0.9ms = 0 degree
#define ZERO PERIOD - ((0.9*PERIOD)/20)
// 1.5ms = 90 degree
#define NINETY PERIOD - ((1.5*PERIOD)/20)
// 2.1ms = 180 degree
#define ONEEIGHTY PERIOD - ((2.1*PERIOD)/20)

#define CH_X 0
#define CH_Y 1


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

/*  set OC8 and OC7 to work in PWM mode and be controlled by Timer 2
command x and y of servo are connected to OC8 and OC7
OC8,OC8 pin will be set to high for ... ms every 20ms. */

void setupServo(uint8_t servoNum)
{
    //setup Timer 2
    CLEARBIT(T2CONbits.TON); // Disable Timer
    CLEARBIT(T2CONbits.TCS); // Select internal instruction cycle clock
    CLEARBIT(T2CONbits.TGATE); // Disable Gated Timer mode
    TMR2 = 0x00; // Clear timer register
    T2CONbits.TCKPS = 0b10; // Select 1:64 Prescaler
    CLEARBIT(IFS0bits.T2IF); // Clear Timer2 interrupt status flag
    CLEARBIT(IEC0bits.T2IE); // Disable Timer2 interrupt enable control bit
    PR2 = PERIOD; // Set timer period 20ms:
   
    if(servoNum == CH_X)
    {
        //setup OC7
        CLEARBIT(TRISDbits.TRISD7); /* Set OC7 as output */
        OC7R = NINETY; /* Set the initial duty cycle to 1.5 ms (90 degree) */
        OC7RS = NINETY; /* Load OCRS: next pwm duty cycle */
        OC7CON = 0x0006; /* Set OC7: PWM, no fault check, Timer2 */
        SETBIT(T3CONbits.TON); /* Turn Timer 3 on */
    }

    else if(servoNum == CH_Y)
    {
        //setup OC8
        CLEARBIT(TRISDbits.TRISD8); 
        OC8R = NINETY; 
        OC8RS = NINETY; 
        OC8CON = 0x0006; 
        SETBIT(T2CONbits.TON);
    }
    
}

// duty cycle in microsecs
void setDutyCycle(uint8_t servoNum, uint8_t dutyCycle)
{
    if(servoNum == CH_X)
    {
        OC7RS = PERIOD - dutyCycle; // next duty cycle 
    }
    else if(servoNum == CH_Y)
    {
        OC8RS = PERIOD - dutyCycle; 
    }
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
    while (!AD1CON1bits.DONE);
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
	
    //TODO: ----------------- init adc -----------------

    // ----------------- init servo -----------------
    setupServo(CH_X); 
    setupServo(CH_Y); 

    // ----------------- start: define range for x,y
    uint16_t X;
    uint16_t Y;
    // Notes: servo controllers 'invert' the signal from OC8 and OC7
    // 16-bit resolution
    uint16_t maxX = 0x0000;
    uint16_t minX = 0xffff;
    uint16_t maxY = 0x0000;
    uint16_t minY = 0xffff;

    uint16_t pwX;
    uint16_t pwY;

    //  end: define range for x,y -----------------

	while(1){

        // servo X
        // TODO: read ADC for x,y

        // x val read from ADC, assume to be 100
        X = 100;
        // V_digital =  Range * (V_analog - V_min) / (V_max - V_min) 
        // Range = 3820-2580 = 240
        pwX = 2*240L*(X-minX)/(maxX-minX); 
        setDutyCycle(CH_X, pwX);

		
	}
}

