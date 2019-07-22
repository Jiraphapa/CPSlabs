#include <includes.h>
#include <touch.c>
#include <motor.c>

/*
*********************************************************************************************************
*                                                CONSTANTS
*********************************************************************************************************
*/

// control task frequency (Hz)
#define RT_FREQ 50

//setpoint parameters
#define SPEED 0.08  // tested up to .12!
#define RADIUS 350
#define CENTER_X 1520 //TODO: make sure that these parameters match your touchscreen
#define CENTER_Y 1300 //TODO: make sure that these parameters match your touchscreen

/*
*********************************************************************************************************
*                                                VARIABLES
*********************************************************************************************************
*/

OS_STK  AppStartTaskStk[APP_TASK_START_STK_SIZE];
// TODO define task stacks
OS_STK LCDDisplayTaskStk[LCD_DISPLAY_TASK_STK_SIZE];

// control setpoint
double Xpos_set = 1520.0, Ypos_set = 1300.0; //TODO: make sure that these parameters match your touchscreen

// raw, unfiltered X and Y position of the ball
CPU_INT16U Xpos, Ypos;

// filtered X and Y position of the ball
CPU_INT16U Xposf, Yposf;

CPU_INT08U select = X_DIM;


/*
*********************************************************************************************************
*                                            FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppStartTask(void *p_arg);
static  void  AppTaskCreate(void);
// TODO declare function prototypes
static  void LCDDisplayTask(void *p_arg);

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
    initTouchScreen();
    setupServo(CH_X); 
    setupServo(CH_Y); 
    // initialize LCD
    DispInit();
    
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
    CPU_INT08U  err;
    OSTaskCreateExt(LCDDisplayTask,                                       /* Create the task for lcd display      */
                    (void *)0,
                    (OS_STK *)&LCDDisplayTaskStk[0],
                    LCD_DISPLAY_TASK_PRIO,
                    LCD_DISPLAY_TASK_PRIO,
                    (OS_STK *)&LCDDisplayTaskStk[LCD_DISPLAY_TASK_STK_SIZE-1],
                    LCD_DISPLAY_TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    OSTaskNameSet(LCD_DISPLAY_TASK_PRIO, (CPU_INT08U *)"LCD Task", &err);
}


static  void  LCDDisplayTask (void *p_arg)
{
     DispStr(1,1,"Group 4 Session 3");
}


