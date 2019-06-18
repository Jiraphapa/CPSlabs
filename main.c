#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <stdio.h>
#include <time.h>

#include <math.h>
#include <libpic30.h>

#include "lcd.h"
#include "led.h"

#define _POSIX_C_SOURCE 200809L

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF &POSCMD_XT);

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);

/*
void print_current_time_with_ms (void){
    long    ms;
    time_t  s;
    struct timespec spec;
    
    clock_gettime(CLOCK_REALTIME, &spec);
    
    s = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6);
    if (ms > 999){
        s++;
        ms = 0;
    }
        
}
 */

void __attribute__((interrupt)) _T3Interrupt(void)
{

    IFS0bits.T3IF = 0; // Clear Timer3 Interrupt Fla
}

void initTimer3()
{
    // Task 3
    // setup Timer 3 to raise an interrupt every 1 ms
    // Notes: similar configuration to Timer 2
    CLEARBIT(T3CONbits.TON);   // Disable Timer
                               // Notes: the system clock operates at 12.8Mhz
    CLEARBIT(T3CONbits.TCS);   // Select internal instruction cycle clock
    CLEARBIT(T3CONbits.TGATE); // Disable Gated Timer mode
    TMR3 = 0x00;               // Clear timer register
    T3CONbits.TCKPS = 0b11;    // Select 1:256 Prescaler
    // Notes: (period * prescale) / clock freq. = actual time in second
    // (50 * 256) / 12800000 = 0,001
    PR3 = 50;                // Load the period value
    IPC2bits.T3IP = 0x02;    // Set Timer3 Interrupt Priority Level
    CLEARBIT(IFS0bits.T3IF); // Clear Timer3 Interrupt Flag
    SETBIT(IEC0bits.T3IE);   // Enable Timer3 interrupt
    SETBIT(T3CONbits.TON);   // Start Timer
}

void initTimers()
{
    initTimer3();
}

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
    SETBIT(PORTEBits.RE2);
    CLEARSBIT(PORTEbits.RE3);
}

void initADC()
{
    CLEARBIT(AD1CON1bits.ADON); //disable ADC

    SETBIT(TRISEbits.TRISE15); //set TRISE AN9 to input
    CLEARBIT(AD1PCFGLbits.PCFG15);

    CLEARBIT(AD1CON1bits.AD12B);
    AD1CON1bits.FORM = 0;   //set integer output
    AD1CON1bits.SSRC = 0x7; //automatic conversion

    AD1CON2 = 0; //no scan sampling

    CLEARBIT(AD1CON3Bits.ADRC); // internal clock source
    AD1CON3bits.SAMC = 0x1F;    //sample to conversion clock = 31 tad
    AD1CON3bits.ADCS = 0x2;

    SETBIT(AD1CON1bits.ADON); //enable ADC
}

void initADC2()
{
    CLEARBIT(AD2CON1bits.ADON); //disable ADC

    SETBIT(TRISEbits.TRISE9); //set TRISE AN9 to input
    CLEARBIT(AD2PCFGLbits.PCFG9);

    CLEARBIT(AD2CON1bits.AD12B);
    AD2CON1bits.FORM = 0;   //set integer output
    AD2CON1bits.SSRC = 0x7; //automatic conversion

    AD1CON2 = 0; //no scan sampling

    CLEARBIT(AD2CON3Bits.ADRC); // internal clock source
    AD2CON3bits.SAMC = 0x1F;    //sample to conversion clock = 31 tad
    AD2CON3bits.ADCS = 0x2;

    SETBIT(AD2CON1bits.ADON); //enable ADC
}

void readADC()
{
    AD1CHS0bits.CS0SA = 0x014;
    SETBIT(AD1CON1bits.SAMP);
    while (!AD1CON1bits.DONE)
        ;
    CLEARBIT(AD1CON1bits.DONE);
    return ADC1BUF0;
}

void readADC2()
{
    AD2CHS0bits.CS0SA = 0x014;
    SETBIT(AD2CON1bits.SAMP);
    while (!AD2CON1bits.DONE)
        ;
    CLEARBIT(AD2CON1bits.DONE);
    return ADC1BUF0;
}

void main()
{
    __C30_UART = 1;

    while (1)
    {
    }
}
