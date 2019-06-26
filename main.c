#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"


// start: Servo configurtaion macros

// 4000= 20*10^-3 * 12.8*10^6 * 1/64
// 1000= 20*10^-3 * 12.8*10^6 * 1/256
#define PERIOD 1000 // timer period 20ms:
// duty cycle = pulse width / period 
// 0.9ms = 0 degree
#define ZERO (0.9*PERIOD)/20
// 1.5ms = 90 degree
#define NINETY (1.5*PERIOD)/20
// 2.1ms = 180 degree
#define ONEEIGHTY (2.1*PERIOD)/20

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

// start: Screen config

void initTouchScreen()
{
    CLEARBIT(PORTEbits.RE1);
    CLEARBIT(PORTEbits.RE2);
    CLEARBIT(PORTEbits.RE3);
    
    
    SETBIT(PORTEbits.RE1);
    SETBIT(PORTEbits.RE2);
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
        SETBIT(PORTEbits.RE2);
        SETBIT(PORTEbits.RE3);
    }
    else{
        SETBIT(PORTEbits.RE1); 
        CLEARBIT(PORTEbits.RE2);
        CLEARBIT(PORTEbits.RE3);

    }
    setDirectionADC(direction);
     __delay_ms(10);
        
    
}

// end: Screen config



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
    
    
    
    uint16_t NUM_SAMPLES = 5;
    uint16_t samples[NUM_SAMPLES];
    int i = 0;
    float sum = 0;

	while(1){

        //ZERO = 45 NINETY = 75 ONEEIGHTY = 105
        setDutyCycle(CH_Y, ZERO - 10 ); 
        setDutyCycle(CH_X, NINETY - 10 );
        
  
        /*
        
        setTouchMode(0); // read x
        
        for (i = 0; i < NUM_SAMPLES; i++){
            __delay_ms(2);
            sum += readADC();
        }
        
        
        lcd_locate(0, 1);
        lcd_printf("X: %.01f\n", (sum / NUM_SAMPLES) );
        sum =0;
        
        
        setTouchMode(1);
        for (i = 0; i < NUM_SAMPLES; i++){
            __delay_ms(2);
            sum += readADC();
        }
        lcd_locate(0, 3);
        lcd_printf("Y: %.01f\n", (sum / NUM_SAMPLES) );
        sum =0;        
                

        __delay_ms(100);
         */
		
	}
}
