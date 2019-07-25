#include <includes.h>
#include <touch.h>
#include <motor.h>
#include <stdio.h>

/*
*********************************************************************************************************
*                                                CONSTANTS
*********************************************************************************************************
*/

// control task frequency (Hz)
#define RT_FREQ 50

//setpoint parameters

#define RADIUS 350
#define CENTER_X 1520 //TODO: make sure that these parameters match your touchscreen
#define CENTER_Y 1300 //TODO: make sure that these parameters match your touchscreen

/*
*********************************************************************************************************
*                                                VARIABLES
*********************************************************************************************************
*/


OS_EVENT *ResourceMutex;
OS_STK  AppStartTaskStk[APP_TASK_START_STK_SIZE];
// TODO define task stacks
OS_STK LCDDisplayTaskStk[LCD_DISPLAY_TASK_STK_SIZE];
OS_STK TouchscreenTaskStk[TOUCHSCREEN_TASK_STK_SIZE];
OS_STK PIDControlTaskStk[PID_CONTROL_TASK_STK_SIZE];

// control setpoint
double Xpos_set = 1520.0, Ypos_set = 1300.0; //TODO: make sure that these parameters match your touchscreen

// raw, unfiltered X and Y position of the ball
CPU_INT16U Xpos, Ypos;

// filtered X and Y position of the ball
CPU_INT16U Xposf, Yposf;

CPU_INT08U select = X_DIM;
CPU_FP32 tick = 0.0;





/*
*********************************************************************************************************
*                                            FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void AppStartTask(void *p_arg);
static  void AppTaskCreate(void);
// TODO declare function prototypes
static  void LCDDisplayTask(void *p_arg);
static  void TouchscreenTask(void *p_arg);
static  void PIDControlTask(void *p_arg);

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.
* Arguments   : none
*********************************************************************************************************
*/



CPU_INT16S  main (void)
{
    CPU_INT08U  err;
    INT8U mutexErr;

    BSP_IntDisAll();                                                    /* Disable all interrupts until we are ready to accept them */
    OSInit();                                                           /* Initialize "uC/OS-II, The Real-Time Kernel"              */
    
              
    OSTaskCreateExt(AppStartTask,                                       /* Create the start-up task for system initialization       */
                    (void *)0,
                    (OS_STK *)&AppStartTaskStk[0],
                    APP_TASK_START_PRIO,
                    APP_TASK_START_PRIO,
                    (OS_STK *)&AppStartTaskStk[APP_TASK_START_STK_SIZE-1],
                    APP_TASK_START_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    OSTaskNameSet(APP_TASK_START_PRIO, (CPU_INT08U *)"Start Task", &err);
    

    OSStart();                                                          /* Start multitasking (i.e. give control to uC/OS-II)       */
	return (-1);                                                        /* Return an error - This line of code is unreachable       */
}


/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppStartTask()' by 'OSTaskCreate()'.
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*               2) Interrupts are enabled once the task start because the I-bit of the CCR register was
*                  set to 0 by 'OSTaskCreate()'.
*********************************************************************************************************
*/

static  void  AppStartTask (void *p_arg)
{
	(void)p_arg;
	
    BSP_Init();                                                         /* Initialize BSP functions                                 */
    OSStatInit();                                                       /* Determine CPU capacity                                   */
    DispInit();
    // initialize touchscreen and motors
    initADC();
    initTouchScreen();
   
    setupPID();
    // initialize LCD
  
    DispClrScr();
     OSTimeDlyHMSM(0, 0, 0,500);
    // initialize LED
   
    //init Timer
   
    
    AppTaskCreate();                                                    /* Create additional user tasks                             */

    while (DEF_TRUE) {
	    OSTimeDlyHMSM(0, 0, 5, 0);
    }
}


/*
*********************************************************************************************************
*                              CREATE ADDITIONAL APPLICATION TASKS
*********************************************************************************************************
*/

static  void  AppTaskCreate (void)
{
    CPU_INT08U  pid_err;
    /*  OSTaskCreateExt(PIDControlTask,                                        Create the task for PID control     
                    (void *)0,
                    (OS_STK *)&PIDControlTaskStk[0],
                    PID_CONTROL_TASK_PRIO,
                    PID_CONTROL_TASK_PRIO,
                    (OS_STK *)&PIDControlTaskStk[PID_CONTROL_TASK_STK_SIZE-1],
                    PID_CONTROL_TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    OSTaskNameSet(PID_CONTROL_TASK_PRIO, (CPU_INT08U *)"PID Task", &pid_err);*/
    
    CPU_INT08U  lcd_err;
    OSTaskCreateExt(LCDDisplayTask,                                       /* Create the task for lcd display      */
                    (void *)0,
                    (OS_STK *)&LCDDisplayTaskStk[0],
                    LCD_DISPLAY_TASK_PRIO,
                    LCD_DISPLAY_TASK_PRIO,
                    (OS_STK *)&LCDDisplayTaskStk[LCD_DISPLAY_TASK_STK_SIZE-1],
                    LCD_DISPLAY_TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    OSTaskNameSet(LCD_DISPLAY_TASK_PRIO, (CPU_INT08U *)"LCD Task", &lcd_err);

      CPU_INT08U  touch_err;
    OSTaskCreateExt(TouchscreenTask,                                 
                    (void *)0,
                    (OS_STK *)&TouchscreenTaskStk[0],
                    TOUCHSCREEN_TASK_PRIO,
                    TOUCHSCREEN_TASK_PRIO,
                    (OS_STK *)&TouchscreenTaskStk[TOUCHSCREEN_TASK_STK_SIZE-1],
                    TOUCHSCREEN_TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    OSTaskNameSet(TOUCHSCREEN_TASK_PRIO, (CPU_INT08U *)"Touchscreen Task", &touch_err);
    
    CPU_INT08U timer_err;
    

    
}




void openLED(int num){
    int i = 0;
    for (i=1; i <  6; i++){
        if(num != i)
            LED_Off(i);
        else
            LED_On(i);
    }
}


int time = 0;
int led = 1;

static  void  LCDDisplayTask (void *p_arg)
{
     
    
    DispStr(0,0,"Group 4 Session 3");
    OSTimeDlyHMSM(0,0,0,200);
    while(1)
    {
        
        openLED(led % 6);
        
        
        char num[21];
        
        
        sprintf(num, "t: %d,x: %d,y: %d", time, Xposf, Yposf);
        DispStr(2,0, num);
        
        
        
        OSTimeDlyHMSM(0,0,1,0);
        
        
        time++;
        
        led++;

    }
    
}

static void TouchscreenTask (void *p_arg)
{
    INT8U err;
    
    p_arg = p_arg;
    while (1){

        if(select == X_DIM)
        {

            // 1. Read the touchscreen for the selected dimension.
            Xposf = readADC();
            // Xpos = 
            // 2. Filter the touchscreen readings using a low pass filter.
            // Xposf = 
            // 3. Change the selected dimension.
            setTouchMode(Y_DIM);
            setDirectionADC(Y_DIM);
            select = Y_DIM;
        
        }
        else
        {
            Yposf = readADC();
            // 1. Read the touchscreen for the selected dimension.
            // Ypos = 
            // 2. Filter the touchscreen readings using a low pass filter.
            // Yposf = 
            // 3. Change the selected dimension.
            setTouchMode(X_DIM);
            setDirectionADC(X_DIM);
            select = X_DIM;
         
        }
    }
    
    
}



static void PIDControlTask(void *p_arg)
{
    //int dutyX = computePID(Xposf, CH_X, tick);
    //int dutyY = computePID(Yposf, CH_Y, tick);
    INT8U err;
    
    p_arg = p_arg;
  
    
    
    int dutyX = 2100 / 20;
   
    tick+=0.1;
    
    
    

       // ***Notes*** review predefined macro and TODOs in other file as well 
} 


