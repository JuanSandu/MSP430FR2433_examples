//******************************************************************************
//!  TIMER_A, Toggle LED1, CCR0 Cont. Mode ISR, DCO SMCLK
//!
//!  Toggle LED1 using software and TA_0 ISR. Toggles every
//!  50000 SMCLK cycles. SMCLK provides clock source for TACLK.
//!  During the TA_0 ISR, LED1 is toggled and 50000 clock cycles are added to
//!  CCR0. TA_0 ISR is triggered every 50000 cycles. CPU is normally off and
//!  used only during TA_ISR.
//!  ACLK = n/a, MCLK = SMCLK = TACLK = default DCO ~1.048MHz
//!
//!           MSP430FR2xx_4xx Board
//!              ---------------
//!          /|\|               |
//!           | |               |
//!           --|RST            |
//!             |               |
//!             |               |-->LED1
//!
//! This example uses the following peripherals and I/O signals.  You must
//! review these and change as needed for your own board:
//! - TimerA peripheral
//! - GPIO peripheral
//!
//! This example uses the following interrupt handlers.  To use this example
//! in your own application you must add these interrupt handlers to your
//! vector table.
//! - Timer A0
//!
//*****************************************************************************
#include "driverlib.h"
#include "Board.h"

#define COMPARE_VALUE 50000

void main (void)
{
    //Stop Watchdog Timer
    WDT_A_hold(WDT_A_BASE);

    // GPIO CONFIGURATION
    //-------------------
    //Set LED1 as an output pin.
    GPIO_setAsOutputPin(
        GPIO_PORT_LED1,
        GPIO_PIN_LED1
        );

    /*
     * Disable the GPIO power-on default high-impedance mode to activate
     * previously configured port settings
     */
    PMM_unlockLPM5();

    /*
    // CLOCK CONFIGURATION
    //--------------------
    //Set DCO FLL reference = REFO
    CS_initClockSignal(CS_FLLREF,CS_REFOCLK_SELECT,CS_CLOCK_DIVIDER_1);
    // DCO to 16MHz. 16MHz/32.768kHz~488 -> El anterior entero: 487
    CS_initFLLSettle(16000,487);
    CS_clearAllOscFlagsWithTimeout(1000);

    //Set ACLK = REFOCLK with clock divider of 1
    CS_initClockSignal(CS_ACLK,CS_REFOCLK_SELECT,CS_CLOCK_DIVIDER_1);
    //Set SMCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_SMCLK,CS_DCOCLKDIV_SELECT,CS_CLOCK_DIVIDER_1);
    //Set MCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_MCLK,CS_DCOCLKDIV_SELECT,CS_CLOCK_DIVIDER_1);
    */

    //Start timer in continuous mode sourced by SMCLK
    Timer_A_initContinuousModeParam initContParam = {0};
    initContParam.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    initContParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_16;
    initContParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    initContParam.timerClear = TIMER_A_DO_CLEAR;
    initContParam.startTimer = false;
    Timer_A_initContinuousMode(TIMER_A1_BASE, &initContParam);

    //Initiaze compare mode
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0
        );

    Timer_A_initCompareModeParam initCompParam = {0};
    initCompParam.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;
    initCompParam.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    initCompParam.compareOutputMode = TIMER_A_OUTPUTMODE_OUTBITVALUE;
    initCompParam.compareValue = COMPARE_VALUE;
    Timer_A_initCompareMode(TIMER_A1_BASE, &initCompParam);


    Timer_A_startCounter( TIMER_A1_BASE,
            TIMER_A_CONTINUOUS_MODE
                );

    //Enter LPM0, enable interrupts
    __bis_SR_register(LPM0_bits + GIE);

    //For debugger
    __no_operation();
}

//******************************************************************************
//
//This is the TIMER1_A0 interrupt vector service routine.
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(TIMER1_A0_VECTOR)))
#endif
void TIMER1_A0_ISR (void)
{
    uint16_t compVal = Timer_A_getCaptureCompareCount(TIMER_A1_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_0)
            + COMPARE_VALUE;

    //Toggle LED1
    GPIO_toggleOutputOnPin(
        GPIO_PORT_LED1,
        GPIO_PIN_LED1
        );

    //Add Offset to CCR0
    Timer_A_setCompareValue(TIMER_A1_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0,
        compVal
        );
}
