/*
 * File:   servo.c
 * Author: group4session3
 *
 * Created on June 26, 2019, 5:23 PM
 */


#include "servo.h"

void setupServo(uint8_t servoNum)
{
    //setup Timer 2
    CLEARBIT(T2CONbits.TON); // Disable Timer
    CLEARBIT(T2CONbits.TCS); // Select internal instruction cycle clock
    CLEARBIT(T2CONbits.TGATE); // Disable Gated Timer mode
    TMR2 = 0x00; // Clear timer register
    T2CONbits.TCKPS = 0b11; // Select 1:64 Prescaler
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