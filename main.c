#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"

#define DSCK_AD1    AD1PCFGLbits.PCFG11
#define DSCK_AD2    AD2PCFGLbits.PCFG11
#define DSCK_TRIS   TRISBbits.TRISB11

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

// configure DAC pins
/*
#define DSCK_AD1 DA1PCFGLbits.PCFG11
#define DSCK_AD2 DA1PCFGLbits.PCFG11
#define DSCK_TRIS TRISBbits.TRISB11

#define DSDI_AD1 DA1PCFGLbits.PCFG10
#define DSDI_AD2 DA1PCFGLbits.PCFG10
#define DSDI_TRIS TRISBbits.TRISB10

#define DSCK_AD1 DA1PCFGLbits.PCFG13
#define DSCK_AD2 DA1PCFGLbits.PCFG13
#define DSCK_TRIS TRISBbits.TRISB13
*/

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
int iter = 0;
int time = 0;

void __attribute__((interrupt)) _T3Interrupt (void) {
    
    
    if (time == timerTask[iter]){
        time = 0; 
        if (iter == 2){
            iter = 0;
               
        } else {
            iter++; 
        }
    }
    time++;
    
    IFS0bits.T3IF = 0; // Clear Timer3 Interrupt Flag
   

}

void initTimer3(){
    // Task 3
    // setup Timer 3 to raise an interrupt every 1 ms 
    // Notes: similar configuration to Timer 2
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


void setupADC(){
    CLEARBIT(DSCK_AD1);
    CLEARBIT(DSCK_AD2);
    CLEARBIT(DSCK_TRIS);
    
    SETBIT(DSPI_AD1); // set Pin to Digital 
    SETBIT(DSPI_AD2); // set Pin to Digital 
    CLEARBIT(DSPI_TRIS); // set Pin to Output
    
    SETBIT(DLDAC_AD1); // set Pin to Digital 
    SETBIT(DLDAC_AD2); // set Pin to Digital 
    CLEARBIT(DLDAC_TRIS); // set Pin to Output
    
    //CLEARBIT(DCS_TRIS); // Set the CS bit 
    CLEARBIT(PORTDbits.RD8); // Set the CS bit 
    
    // DAC accepts 15 bits
    int i;
    // write command for (MCP4822) 12-bit DAC
    // A/B = 1 write to DACb, = 0 write to DACa
    // _ don't care
    // GA output gain bit 1 = 1x, 0 = 2x
    // SHDN output shutdown control bit, 1 = active mode
    // D11:D0 DAC input data bits
    int config[16] = {1,1,1,1, 1,1,1,1,1,0,1,0,0,0};
    
    int j=1;
   
    for(i=0; i<16 ; i++)
    {
        //DSPI_AD1
        //DSPI_AD1 |= (config[j] & BV(index)) >> i << 10; 
     
       
            
        
         // set the SDI bit if the input bit is one
        //DSPI_AD1 |= (config[j] & BV(i)) >> 0 << 10; 
        //CLEARBIT(DSPI_AD1); 
        SETBIT(DSPI_AD1); // set Pin to Digital 
        SETBIT(DSPI_AD2); // set Pin to Digital 
        
        // toggle SCK bit
        CLEARBIT(DSCK_AD1);
        CLEARBIT(DSCK_AD2);
        Nop();
        SETBIT(DSCK_AD1);
        SETBIT(DSCK_AD2);
       
        
    }
    j++;
    
    SETBIT(PORTDbits.RD8); // de-select the CS bit 
    
    Nop();
    CLEARBIT(DSPI_AD1); // clear data bit
    CLEARBIT(DSPI_AD2); 
    Nop();
    
    // toggle LDAC port low then high to output new voltage
    CLEARBIT(DLDAC_AD1); // set Pin to Digital 
    Nop();
    SETBIT(DLDAC_AD2); // set Pin to Digital 
    
   
    
    
}

void convert(){
    
}

void convertDAC(){
    
    /*
     // DAC accepts 15 bits
    int i;
    // write command for (MCP4822) 12-bit DAC
    // A/B = 1 write to DACb, = 0 write to DACa
    // _ don't care
    // GA output gain bit 1 = 1x, 0 = 2x
    // SHDN output shutdown control bit, 1 = active mode
    // D11:D0 DAC input data bits
    int config[16] = {1,1,1,1, 1,1,1,1,1,0,1,0,0,0};
    
    int j=1;
   
    for(i=0; i<16 ; i++)
    {
        //DSPI_AD1
        //DSPI_AD1 |= (config[j] & BV(index)) >> i << 10; 
     
       
        
         // set the SDI bit if the input bit is one
        //DSPI_AD1 |= (config[j] & BV(i)) >> 0 << 10; 
        //CLEARBIT(DSPI_AD1); 
        SETBIT(DSPI_AD1); // set Pin to Digital 
        SETBIT(DSPI_AD2); // set Pin to Digital 
        
         // toggle SCK bit
        CLEARBIT(DSCK_AD1);
        CLEARBIT(DSCK_AD2);
        Nop();
        SETBIT(DSCK_AD1);
        SETBIT(DSCK_AD2);
       
        
    }
    j++;
    
    SETBIT(PORTDbits.RD8); // de-select the CS bit 
    Nop();
    CLEARBIT(DSPI_AD1); // clear data bit
    CLEARBIT(DSPI_AD2); 
    Nop();
    // toggle LDAC port low then high to output new voltage
    CLEARBIT(DLDAC_AD1); // set Pin to Digital 
    Nop();
    SETBIT(DLDAC_AD2); // set Pin to Digital 
    
*/
    
}


void setupDAC2(){
    
    SETBIT(PORTBbits.RB11);
    SETBIT(PORTBbits.RB13);
    SETBIT(PORTBbits.RB10);
    
    
    CLEARBIT(DSCK_AD1);
    CLEARBIT(DSCK_AD2);
    CLEARBIT(DSCK_TRIS);
    
    SETBIT(DSPI_AD1); // set Pin to Digital 
    SETBIT(DSPI_AD2); // set Pin to Digital 
    CLEARBIT(DSPI_TRIS); // set Pin to Output
    
    SETBIT(DLDAC_AD1); // set Pin to Digital 
    SETBIT(DLDAC_AD2); // set Pin to Digital 
    CLEARBIT(DLDAC_TRIS); // set Pin to Output
   
}
void convertDAC2(){
    int data[16] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    // chip select
    CLEARBIT(PORTDbits.RD8);
    int i;
    for (i = 0; i < 16; i++){
        PORTBbits.RB10 &= data[i];
        SETBIT(PORTBbits.RB11);
        Nop();
        CLEARBIT(PORTBbits.RB11);
    }
    SETBIT(PORTDbits.RD8);
    Nop();
    CLEARBIT(PORTBbits.RB10);
    Nop();
    CLEARBIT(DLDAC_AD1);
    Nop();
    SETBIT(DLDAC_AD1);
}


void main(){
    
    
	//Init LCD
	__C30_UART=1;	
	lcd_initialize();
	lcd_clear();
	lcd_locate(0,0);
    lcd_printf("hello kuy");

    setupDAC2();
    initTimer3();
	
	while(1){
        convertDAC2();
	}
}

