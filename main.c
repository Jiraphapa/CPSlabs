#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"
#include "touch.h"
#include "servo.h"





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





// start: Screen config



// end: Screen config


void initTimer3(){

        CLEARBIT(T3CONbits.TON); // Disable Timer
         // Notes: the system clock operates at 12.8Mhz
        CLEARBIT(T3CONbits.TCS); // Select internal instruction cycle clock
        CLEARBIT(T3CONbits.TGATE); // Disable Gated Timer mode
        TMR3 = 0x00; // Clear timer register
        T3CONbits.TCKPS = 0b11; // Select 1:256 Prescaler
        // Notes: (period * prescale) / clock freq. = actual time in second
        // (25000 * 256) / 12800000 = 0,500
        PR3 = 50000; // Load the period value
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

void __attribute__((interrupt)) _T3Interrupt (void) {
    
    
    if(count == 5){
        count = 0;
        
        setDutyCycle(CH_X, commandX[i] - 10 );
        setDutyCycle(CH_Y, commandY[i] - 10 );
        i++;
        if (i == 4){
            i =0;
        }
        
         setTouchMode(0); // read x
         __delay_ms(10);
        
        for (j = 0; j < NUM_SAMPLES; j++){
            __delay_ms(2);
            sum = readADC();
        }
        
        lcd_locate(0, 1);
        lcd_printf("Y: %.01f\n", sum );
        sum =0;
        
        
        setTouchMode(1);
        __delay_ms(10);
        
        for (j = 0; j < NUM_SAMPLES; j++){
            __delay_ms(2);
            sum = readADC();
        }
        lcd_locate(0, 3);
        lcd_printf("X: %.01f\n", sum );
        sum =0; 
    }
    count++;  
     
    
    

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
   
      

	while(1){
        //ZERO = 45 NINETY = 75 ONEEIGHTY = 105
		
	}
}
