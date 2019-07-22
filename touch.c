/*
 * File:   touch.c
 * Author: group4session3
 *
 * Created on June 26, 2019, 5:11 PM
 */


#include "touch.h"



void initTouchScreen()
{
    CLEARBIT(TRISEbits.TRISE1);
    CLEARBIT(TRISEbits.TRISE2);
    CLEARBIT(TRISEbits.TRISE3);
    
    
    SETBIT(PORTEbits.RE1);
    SETBIT(PORTEbits.RE2);
    CLEARBIT(PORTEbits.RE3);
}


void initADC(){

    // 0. Disable ADC1
    CLEARBIT(AD1CON1bits.ADON);

    // 1. set input pins
    SETBIT(TRISBbits.TRISB15); // set input. AN15
    SETBIT(TRISBbits.TRISB9); // set input. AN9

    // 2. set analog pins
    CLEARBIT(AD1PCFGLbits.PCFG15); // set analog. X is hardwired to AN15
    CLEARBIT(AD1PCFGLbits.PCFG9); // set analog. Y is hardwired to AN9

    // 3. Configure AD1CON1
    SETBIT(AD1CON1bits.AD12B); // set 12-bit mode
    AD1CON1bits.FORM = 0; // set integer output
    AD1CON1bits.SSRC = 0x7; // set automatic conversion

    // 4. Configure AD1CON2
    AD1CON2 = 0; // not using scanning sampling

    // 5. Configure AD1CON3
    CLEARBIT(AD1CON3bits.ADRC); // internal clock source
    AD1CON3bits.SAMC = 0x1F; // sample-to-conversion clock = 31Tad
    AD1CON3bits.ADCS = 0x2; // Tad = 3Tcy (Time cycles)

    // 6. Enable ADC1
    SETBIT(AD1CON1bits.ADON);
}

uint16_t readADC(){
    SETBIT(AD1CON1bits.SAMP); // start to sample
    while(!AD1CON1bits.DONE); // wait for conversion to finish
    CLEARBIT(AD1CON1bits.DONE); // MUST HAVE! clear conversion done bit

    return ADC1BUF0;
}

void setDirectionADC(int direction){
    if (direction == 0)
        AD1CHS0bits.CH0SA = 0x000F;
    else
        AD1CHS0bits.CH0SA = 0x0009;
}

void setTouchMode(int direction){
    if (direction == 0){
        CLEARBIT(PORTEbits.RE1);
        Nop();
        SETBIT(PORTEbits.RE2);
        Nop();
        SETBIT(PORTEbits.RE3);
        Nop();
        AD1CHS0bits.CH0SA = 0x000F;
       
    }
    else{
        SETBIT(PORTEbits.RE1); 
        Nop();
        CLEARBIT(PORTEbits.RE2);
        Nop();
        CLEARBIT(PORTEbits.RE3);
        Nop();
        AD1CHS0bits.CH0SA = 0x0009;
    

    }

     
        
    
}