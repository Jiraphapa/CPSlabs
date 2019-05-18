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
#define LEDTRIS TRISA
#define LED1_TRIS TRISAbits.TRISA4
#define LED1_PORT PORTAbits.RA4
#define LED1 4

#define LED2_TRIS TRISAbits.TRISA5
#define LED2_PORT PORTAbits.RA5
#define LED2 5

#define LED3_TRIS TRISAbits.TRISA9
#define LED3_PORT PORTAbits.RA9
#define LED3 9

#define LED4_TRIS TRISAbits.TRISA10
#define LED4_PORT PORTAbits.RA10
#define LED4 10

#define LED5_TRIS TRISAbits.TRISA0
#define LED5_PORT PORTAbits.RA0
#define LED5 0

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT); 

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);  

void toggleLED(int ledNum){
    if (ledNum == 0){
        
        CLEARLED(LED1_TRIS);
        TOGGLELED(LED1_PORT);
        
    } else if (ledNum == 1){
        
        CLEARLED(LED2_TRIS);
        TOGGLELED(LED2_PORT);
        
    } else if (ledNum == 2){
        
        CLEARLED(LED3_TRIS);
        TOGGLELED(LED3_PORT);
        
    } else if (ledNum == 3){
        
        CLEARLED(LED4_TRIS);
        TOGGLELED(LED4_PORT);
        
    } else if (ledNum == 4){
        
        CLEARLED(LED5_TRIS);
        TOGGLELED(LED5_PORT);
        
    }
}


void turnLED(int ledNum, int on){

    if (ledNum == 0){
        CLEARLED(LED1_TRIS);
        if (on){
            SETLED(LED1_PORT);
        }
        else {
            CLEARLED(LED1_PORT);
        }
    } else if (ledNum == 1){
        CLEARLED(LED2_TRIS);
        if (on){
            SETLED(LED2_PORT);
        }
        else {
            CLEARLED(LED2_PORT);
        }
    } else if (ledNum == 2){
        CLEARLED(LED3_TRIS);
        if (on){
            SETLED(LED3_PORT);
        }
        else {
            CLEARLED(LED3_PORT);
        }
    } else if (ledNum == 3){
        CLEARLED(LED4_TRIS);
        if (on){
            SETLED(LED4_PORT);
        }
        else {
            CLEARLED(LED4_PORT);
        }
    } else if (ledNum == 4){
        CLEARLED(LED5_TRIS);
        if (on){
            SETLED(LED5_PORT);
        }
        else {
            CLEARLED(LED5_PORT);
        }
    }
    
}

int count = 0;
time_t timer;
char buffer[128];
struct tm* tm_info;


void __attribute__((interrupt)) _T1Interrupt (void) {
  
    toggleLED(1);
    IFS0bits.T1IF = 0; // Clear Timer1 Interrupt Flag
   
}

void __attribute__((interrupt)) _T2Interrupt (void) {
  
    toggleLED(0);
    IFS0bits.T2IF = 0; // Clear Timer2 Interrupt Flag
   
}

void __attribute__((interrupt)) _T3Interrupt (void) {
//TODO: print time in format mm:ss.xxx (minute,second and milliseconds respectively.)



//    char buff[100];
//    time_t now = time(0);
//    strftime(buff, 100, "%H:%M:%S", localtime(&now));
//    lcd_printf("%s\n", buff);
   
    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, 128, "%M:%S:%X", tm_info);
    lcd_locate(0,0);
    lcd_printf("%s\n", buffer);
    toggleLED(3);
        
    



    IFS0bits.T3IF = 0; // Clear Timer3 Interrupt Flag

}


void initTimer1(){
    //task 2
     // trigger an interrupt every 1 second.
     //enable LPOSCEN
    __builtin_write_OSCCONL(OSCCONL | 2);
    T1CONbits.TON = 0; //Disable Timer
    //Notes: Timer1 has access to a 32kHz external clock
    T1CONbits.TCS = 1; //Select external clock
    T1CONbits.TSYNC = 0; //Disable Synchronization
    T1CONbits.TCKPS = 0b00; //Select 1:1 Prescaler
    TMR1 = 0x00; //Clear timer register
    PR1 = 32767; //Load the period value
    IPC0bits.T1IP = 0x01; // Set Timer1 Interrupt Priority Level
    IFS0bits.T1IF = 0; // Clear Timer1 Interrupt Flag
    IEC0bits.T1IE = 1;// Enable Timer1 interrupt
    T1CONbits.TON = 1;// Start Timer
}


void initTimer2(){
      // Task 1
      // setup Timer 2 to raise an interrupt every 2 ms 
        CLEARBIT(T2CONbits.TON); // Disable Timer
        // Notes: the system clock operates at 12.8Mhz
        CLEARBIT(T2CONbits.TCS); // Select internal instruction cycle clock
        CLEARBIT(T2CONbits.TGATE); // Disable Gated Timer mode
        TMR2 = 0x00; // Clear timer register
        T2CONbits.TCKPS = 0b11; // Select 1:256 Prescaler
        // Notes: (period * prescale) / clock freq. = actual time in second
        // (100 * 256) / 12800000 = 0.002
        PR2 = 100; // Load the period value
        IPC1bits.T2IP = 0x02; // Set Timer2 Interrupt Priority Level
        CLEARBIT(IFS0bits.T2IF); // Clear Timer2 Interrupt Flag
        SETBIT(IEC0bits.T2IE); // Enable Timer2 interrupt
        SETBIT(T2CONbits.TON); // Start Timer
        
}

void initTimer3(){
    // Task 3
    // setup Timer 3 to raise an interrupt every 5 ms 
    // Notes: similar configuration to Timer 2
        CLEARBIT(T3CONbits.TON); // Disable Timer
         // Notes: the system clock operates at 12.8Mhz
        CLEARBIT(T3CONbits.TCS); // Select internal instruction cycle clock
        CLEARBIT(T3CONbits.TGATE); // Disable Gated Timer mode
        TMR3 = 0x00; // Clear timer register
        T3CONbits.TCKPS = 0b11; // Select 1:256 Prescaler
        // Notes: (period * prescale) / clock freq. = actual time in second
        // (250 * 256) / 12800000 = 0.005
        PR3 = 250; // Load the period value
        IPC2bits.T3IP = 0x02; // Set Timer3 Interrupt Priority Level
        CLEARBIT(IFS0bits.T3IF); // Clear Timer3 Interrupt Flag
        SETBIT(IEC0bits.T3IE); // Enable Timer3 interrupt
        SETBIT(T3CONbits.TON); // Start Timer
        
}

void initTimers(){
    initTimer1();
    initTimer2();
    initTimer3();
}


void main(){
    
    
    __C30_UART=1;	
	lcd_initialize();
	lcd_clear();
	lcd_locate(0,0);
    
        //strftime(buffer, 26, "%MM:%SS P ", tm_info);
    
     //CLEARLED(LED1_TRIS);
    initTimers();
  
       
   
	while(1){
        
        
        
        
       
    }
    
    

}

