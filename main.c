#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"

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






//  I/O pins:  SDI (RB10/AN10), SCK (RB11/AN11) and LDAC (RB13/AN13).
// In order to use these pins as digital I/O, these must be configured as digital for both ADC registers
#define DSCK_AD1 AD1PCFGLbits.PCFG11
#define DSCK_AD2 AD2PCFGLbits.PCFG11
#define DSCK_TRIS  TRISBbits.TRISB11

// SCK (RB11/AN11)
#define DSPI_AD1 AD1PCFGLbits.PCFG10
#define DSPI_AD2 AD2PCFGLbits.PCFG10
#define DSPI_TRIS  TRISBbits.TRISB10

// LDAC (RB13/AN13).
#define DLDAC_AD1 AD1PCFGLbits.PCFG13
#define DLDAC_AD2 AD2PCFGLbits.PCFG13
#define DLDAC_TRIS  TRISBbits.TRISB13


#define CS_TRIS TRISDbits.TRISD8
#define CS_PORT PORTDbits.RD8


// Chip select
//#define DCS_TRIS  TRISBbits.TRISD8

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

int timerTask[3] = { 500, 2000, 1000 };
int digitalLevel[3] = { 1000, 2500, 3500 };
int digitalData[16];

int data1[16] = {0,0,0,1,0,1,1,1,1,1,0,0,   1,1,1,1};
int data2[16] = {0,0,1,0,0,0,1,1,1,0,0,1,   1,1,1,1};
//int data2[16] = {1,0,0,1,1,1,0,0,0,1,0,0,   1,1,1,1};
int data3[16] = {0,0,1,1,0,1,0,1,1,0,1,1,   1,1,1,1};
//int data3[16] = {1,1,0,1,1,0,1,0,1,1,0,0,   1,1,1,1};

int iter = 0;
int time = 0;
int data[16];
    
    


void __attribute__((interrupt)) _T3Interrupt (void) {
    
    int i;
    if (time == timerTask[iter]){
        for (i = 0; i < 16; i++){
            if (iter == 0)
                data[i] =  data1[i];
            else if (iter == 1)
                data[i] =  data2[i];
            else if (iter == 2)
                data[i] =  data3[i];
        }

        time = 0; 
        if (iter == 2){
            iter = 0;
	    toggleLED(0);
        } else {
            toggleLED(0);
            iter++; 
        }
    }
    time++;
    
    IFS0bits.T3IF = 0; // Clear Timer3 Interrupt Flag
   

}

void initTimer3(){

        CLEARBIT(T3CONbits.TON); // Disable Timer
         // Notes: the system clock operates at 12.8Mhz
        CLEARBIT(T3CONbits.TCS); // Select internal instruction cycle clock
        CLEARBIT(T3CONbits.TGATE); // Disable Gated Timer mode
        TMR3 = 0x00; // Clear timer register
        T3CONbits.TCKPS = 0b11; // Select 1:256 Prescaler
        // Notes: (period * prescale) / clock freq. = actual time in second
        // (25000 * 256) / 12800000 = 0,500
        PR3 = 50; // Load the period value
        IPC2bits.T3IP = 0x02; // Set Timer3 Interrupt Priority Level
        CLEARBIT(IFS0bits.T3IF); // Clear Timer3 Interrupt Flag
        SETBIT(IEC0bits.T3IE); // Enable Timer3 interrupt
        SETBIT(T3CONbits.TON); // Start Timer
        
}

void setupDAC(){

    
    SETBIT(DSCK_AD1);
    SETBIT(DSCK_AD2);
    CLEARBIT(DSCK_TRIS);
    
    SETBIT(DSPI_AD1); // set Pin to Digital 
    SETBIT(DSPI_AD2); // set Pin to Digital 
    CLEARBIT(DSPI_TRIS); // set Pin to Output
    
    SETBIT(DLDAC_AD1); // set Pin to Digital 
    SETBIT(DLDAC_AD2); // set Pin to Digital 
    CLEARBIT(DLDAC_TRIS); // set Pin to Output

    CLEARBIT(CS_TRIS); // set cs pin to output

   
}


void convertDAC(){

    CLEARBIT(CS_PORT);
    int i;
    
    for (i = 0; i < 16; i++){
        
        PORTBbits.RB10 |=  data[i];
        
        //PORTB |= (data[i] & BV(2)) >> 2 << 10;
        Nop();
        PORTBbits.RB11 = 0;
        Nop();
        PORTBbits.RB11 = 1;
        Nop();
    }
    
    SETBIT(CS_PORT);
    //SETBIT(PORTDbits.RD8);
    Nop();
    PORTB = 0; 
    Nop();
    CLEARBIT(DLDAC_AD1);
    Nop();
    SETBIT(DLDAC_AD1);
    Nop();
    
    CLEARBIT(PORTBbits.RB10);
    Nop();
}


void main(){
    
    
	//Init LCD
	__C30_UART=1;	
	lcd_initialize();
	lcd_clear();
	lcd_locate(0,0);
    lcd_printf("hello kuy");

    setupDAC();
    initTimer3();

 
	
	while(1){
        convertDAC();
	}
}

