// 
// Create task and start OS 
//
//
 
#include <includes.h>
#include "Param.h"
#include "IOCon.h"
#include "ADC.h"

INT8U SW1_logic=1,led_stat=0;		//initiate unsigned char for SW1_logic=1, led_stat=0
INT16U *ADCPtr,ADCValue=0;			//initiate unsigned integer for *ADCPtr and ADCValue=0

#define TASK_1_PRIO       5			//define task1 priority lowest
#define TASK_2_PRIO       1			//define task2 priority highest
#define TASK_3_PRIO       3			//define task3 priority second lowest
#define TASK_4_PRIO       2			//define task4 priority second highest



#define TASK_1_STK_SIZE   128
OS_STK Task_1_Stk[TASK_1_STK_SIZE];   /* Task stack */

#define TASK_2_STK_SIZE   128
OS_STK Task_2_Stk[TASK_2_STK_SIZE];   /* Task stack */

#define TASK_3_STK_SIZE   256
OS_STK Task_3_Stk[TASK_3_STK_SIZE];   /* Task stack */

#define TASK_4_STK_SIZE   256
OS_STK Task_4_Stk[TASK_4_STK_SIZE];   /* Task stack */


static void Task_1(void *p_arg);      /* Task function */
static void Task_2(void *p_arg);      /* Task function */
static void Task_3(void *p_arg);      /* Task function */
static void Task_4(void *p_arg);      /* Task function */


CPU_INT16S  main (void)
{  
	CPU_INT08U err;
	//initialization
	OSInit();					
	BSP_Init();				
	IO_Init();
	ADC_Init();  
	//creating tasks
	OSTaskCreate(Task_1, (void *)0, (OS_STK *)&Task_1_Stk[0], TASK_1_PRIO);
	OSTaskCreate(Task_2, (void *)0, (OS_STK *)&Task_2_Stk[0], TASK_2_PRIO);
	OSTaskCreate(Task_3, (void *)0, (OS_STK *)&Task_3_Stk[0], TASK_3_PRIO);
	OSTaskCreate(Task_4, (void *)0, (OS_STK *)&Task_4_Stk[0], TASK_4_PRIO);
	//setting tasks name
	OSTaskNameSet(TASK_1_PRIO,  (CPU_INT08U *)"Task 1", &err);
	OSTaskNameSet(TASK_2_PRIO,  (CPU_INT08U *)"Task 2", &err);
	OSTaskNameSet(TASK_3_PRIO,  (CPU_INT08U *)"Task 3", &err);
	OSTaskNameSet(TASK_4_PRIO,  (CPU_INT08U *)"Task 4", &err);
	//OS start
	OSStart();                     

	return (-1);             
}

static void Task_1(void *p_arg)	//led 3 blinking at 1 hz
{
	(void)p_arg;  /* avoid compiler warning */
	
	LED3=0;
	while (1) 
	{
		LED3=~LED3;
		OSTimeDlyHMSM(0,0,0,500);
	}
}

static void Task_2(void *p_arg)	//led 1 blinking state control and change in stat of switches
{
	(void)p_arg;  /* avoid compiler warning */
	
	while (1)
	{		//error prevention, the loop system only activated when there is a state change of SW1
		if(SW1==1)
		{
			SW1_logic=1;
		}
		else if(SW1==0)					//the system activated once SW1 is pressed
		{
			if(SW1_logic==1)			//the system will only activated when the SW1 changed from high to low
			{		//selecting led_stat based on the number of SW1 state change
				if(led_stat<=3)
				{
					led_stat=led_stat+1;
				}
				if(led_stat>3)			//error prevention, this prevent the led_stat from existing the assignment requirement
				{
					led_stat=0;
				}
				OSTimeDlyResume(TASK_3_PRIO);	//force task3 to resume at the new blinking rate if SW1 is pressed
			
			}
			SW1_logic=0;				//error prevention, reset the SW1_logic to 0 so the led_stat change will only be triggered when the SW1 changes from high to low
		}
		OSTaskResume(TASK_4_PRIO);		//Resume task 4 once it's been terminated
		if(LED2==1&&SW2==1)				//When SW2 is released, the LED2 will turn off
			OSTimeDlyResume(TASK_4_PRIO);
		OSTimeDlyHMSM(0,0,0,20);		
	}
}

static void Task_3(void *p_arg)	//led 1 blinking timing
{
	(void)p_arg;  /* avoid compiler warning */
	
	while (1)
	{
		switch(led_stat)			//determine led_stat's tasks
		{
			case(0):				//Off the LED1
				LED1=0;
				OSTimeDlyHMSM(0,0,1,0);
				break;
			case(1):				//blink at 1Hz
				LED1=~LED1;
				OSTimeDlyHMSM(0,0,0,500);
				break;
			case(2):				//blink at 2Hz
				LED1=~LED1;
				OSTimeDlyHMSM(0,0,0,250);
				break;
			case(3):				//blink at 4Hz
				LED1=~LED1;
				OSTimeDlyHMSM(0,0,0,125);
				break;
				
		}
	}
}

static void Task_4(void *p_arg) //ADC
{
	(void)p_arg;		/* avoid compiler warning */
	while (1)
	{
		if(SW2==0)							//SW2 being pressed
		{
			ADC_ON;
			ADCPtr = (INT16U*)&ADC1BUF0;	//initialize ADC1BUF pointer
			ADCValue = *ADCPtr;				//store the ADC1BUFF value inside ADCValue
			if(ADCValue>=10&&ADCValue<1024)	//determine LED2 blinking rate due to the ADC current value
			{
				LED2=~LED2;
				OSTimeDly(ADCValue);
			}

		}
		else if(SW2==1)						//SW2 released
		{
			ADC_OFF;						//turn off the ADC
			ADCValue = 0;					//clear the ADCValue
			LED2=0;							//off the LED2
			OSTaskSuspend(OS_PRIO_SELF);	//suspend task4
		}
		OSTimeDlyHMSM(0,0,0,20);
	}
}