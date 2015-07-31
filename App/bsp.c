/*
*********************************************************************************************************
*                                             Microchip dsPIC33FJ
*                                            Board Support Package
*
*                                                   Micrium
*                                    (c) Copyright 2005, Micrium, Weston, FL
*                                              All Rights Reserved
*
*
* File : BSP.C
* By   : Eric Shufro
*
* Rev  : NgMS @ 2 Aug, 2010
*        - Generic PIC24F
*
*********************************************************************************************************
*/

#include <includes.h>

/*
*********************************************************************************************************
*                                         MPLAB CONFIGURATION MACROS
*********************************************************************************************************
*/
_CONFIG1( JTAGEN_OFF & BKBUG_OFF & FWDTEN_OFF) 
_CONFIG2( FCKSM_CSDCMD & OSCIOFNC_ON & POSCMOD_NONE & FNOSC_FRC ) /* 8MHz FRC */


/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              VARIABLES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              PROTOTYPES
*********************************************************************************************************
*/

static  void  Tmr_TickInit(void);

/*
*********************************************************************************************************
*                                         BSP INITIALIZATION
*
* Description : This function should be called by your application code before you make use of any of the
*               functions found in this module.
*
* Arguments   : none
*********************************************************************************************************
*/

void  BSP_Init (void)
{
    RCON    &= ~SWDTEN;                                                 /* Ensure Watchdog disabled via IDE CONFIG bits and SW.     */

    Tmr_TickInit();                                                     /* Initialize the uC/OS-II tick interrupt                   */
}

/*
*********************************************************************************************************
*                                      BSP_CPU_ClkFrq()

* Description : This function determines the CPU clock frequency (Fcy)
* Returns     : The CPU frequency in (HZ)
*********************************************************************************************************
*/

CPU_INT32U  BSP_CPU_ClkFrq (void)
{
    CPU_INT08U  Clk_Selected;
    CPU_INT16U  FRC_Div;
    CPU_INT32U  CPU_Clk_Frq;


    FRC_Div       = ((CLKDIV & FRCDIV_MASK) >> 8);                      /* Get the FRC Oscillator Divider register value            */
    FRC_Div       = ((1 << FRC_Div) * 2);                               /* Compute the FRC Divider value                            */

    Clk_Selected  =  (OSCCON & COSC_MASK) >> 12;                        /* Determine which clock source is currently selected       */

    switch (Clk_Selected) {
        case 0:                                                         /* Fast Oscillator (FRC) Selected                           */
             CPU_Clk_Frq   =   CPU_FRC_OSC_FRQ;                         /* Return the frequency of the internal fast oscillator     */
             break;

        case 1:                                                         /* Fast Oscillator (FRC) with PLL Selected                  */
             CPU_Clk_Frq   =  (CPU_FRC_OSC_FRQ  * 4);                   /* Compute the PLL output frequency  = (FRC * 4)            */
            break;

        case 2:                                                         /* Primary External Oscillator Selected                     */
             CPU_Clk_Frq   =   CPU_PRIMARY_OSC_FRQ;                     /* Return the frequency of the primary external oscillator  */
             break;

        case 3:                                                         /* Primary External Oscillator with PLL Selected            */
             CPU_Clk_Frq   =  (CPU_PRIMARY_OSC_FRQ * 4);                /* Compute the PLL output frq as (CPU_PRIMARY_OSC_FRQ * 4)  */
             break;

        case 4:                                                         /* Secondary Oscillator Selected (SOCS)                     */
             CPU_Clk_Frq   =   CPU_SECONDARY_OSC_FRQ;                   /* Return the frq of the external secondary oscillator      */
             break;

        case 5:                                                         /* Low Power Oscillator (LPOSC) Selected                    */
             CPU_Clk_Frq   =   CPU_LOW_POWER_OSC_FRQ;                   /* Return the frq of the Low Power Oscillator               */
             break;

        case 6:
             CPU_Clk_Frq = 0;                                           /* Return 0 for the Reserved clock setting                  */
             break;

        case 7:                                                         /* Fast Oscillator (FRC) with FRCDIV Selected               */
             CPU_Clk_Frq   =   CPU_FRC_OSC_FRQ / FRC_Div;               /* Return the clock frequency of FRC / FRC_Div              */
             break;

        default:
             CPU_Clk_Frq = 0;                                           /* Return 0 if the clock source cannot be determined        */
             break;
    }
    
    CPU_Clk_Frq   /=  2;                                                /* Divide the final frq by 2, get the actual CPU Frq (Fcy)  */

    return (CPU_Clk_Frq);                                               /* Return the operating frequency                           */
}

/*
*********************************************************************************************************
*                                     DISABLE ALL INTERRUPTS
*
* Description : This function disables all interrupts from the interrupt controller.
*
* Arguments   : none
*********************************************************************************************************
*/

void  BSP_IntDisAll (void)
{
}

/*
*********************************************************************************************************
*                                       TICKER INITIALIZATION
*
* Description : This function is called to initialize uC/OS-II's tick source (typically a timer generating
*               interrupts every 1 to 100 mS).
*
* Arguments   : none
*
* Note(s)     : 1) The timer operates at a frequency of Fosc / 4
*               2) The timer resets to 0 after period register match interrupt is generated
*********************************************************************************************************
*/

static  void  Tmr_TickInit (void)
{
    CPU_INT32U  tmr_frq;
    CPU_INT16U  cnts;


    tmr_frq   =   BSP_CPU_ClkFrq();                                     /* Get the CPU Clock Frequency (Hz) (Fcy)                   */
    cnts      =   (tmr_frq / OS_TICKS_PER_SEC) - 1;                     /* Calaculate the number of timer ticks between interrupts  */

#if BSP_OS_TMR_SEL == 2
    T2CON     =   0;                                                    /* Use Internal Osc (Fcy), 16 bit mode, prescaler = 1  		*/
    TMR2      =   0;                                                    /* Start counting from 0 and clear the prescaler count      */
    PR2       =   cnts;                                                 /* Set the period register                                  */
    IPC1     &=  ~T2IP_MASK;                                            /* Clear all timer 2 interrupt priority bits                */
    IPC1     |=  (TIMER_INT_PRIO << 12);                                /* Set timer 2 to operate with an interrupt priority of 4   */
    IFS0     &=  ~T2IF;                                                 /* Clear the interrupt for timer 2                          */
    IEC0     |=   T2IE;                                                 /* Enable interrupts for timer 2                            */
    T2CON    |=   TON;                                                  /* Start the timer                                          */
#endif

#if BSP_OS_TMR_SEL == 4
    T4CON     =   0;                                                    /* Use Internal Osc (Fcy), 16 bit mode, prescaler = 1  		*/
    TMR4      =   0;                                                    /* Start counting from 0 and clear the prescaler count      */
    PR4       =   cnts;                                                 /* Set the period register                                  */
    IPC6     &=  ~T4IP_MASK;                                            /* Clear all timer 4 interrupt priority bits                */
    IPC6     |=  (TIMER_INT_PRIO << 12);                                /* Set timer 4 to operate with an interrupt priority of 4   */
    IFS1     &=  ~T4IF;                                                 /* Clear the interrupt for timer 4                          */
    IEC1     |=   T4IE;                                                 /* Enable interrupts for timer 4                            */
    T4CON    |=   TON;                                                  /* Start the timer                                          */
#endif
}

/*
*********************************************************************************************************
*                                     OS TICK INTERRUPT SERVICE ROUTINE
*
* Description : This function handles the timer interrupt that is used to generate TICKs for uC/OS-II.
*********************************************************************************************************
*/

void OS_Tick_ISR_Handler (void)
{
#if  BSP_OS_TMR_SEL == 2
    IFS0 &= ~T2IF;
#endif

#if  BSP_OS_TMR_SEL == 4
    IFS1 &= ~T41F;
#endif

    OSTimeTick();
}

