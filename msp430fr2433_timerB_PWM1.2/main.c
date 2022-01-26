//!Timer_A3, PWM TA1.1-2, Up/Down Mode, DCO SMCLK
//!
//!Description: This program generates two PWM outputs on P2.0,P2.1 using
//!Timer1_A configured for up/down mode. The value in CCR0, 128, defines the
//!PWM period/2 and the values in CCR1 and CCR2 the PWM duty cycles. Using
//!~1.045MHz SMCLK as TACLK, the timer period is ~233us with a 75% duty cycle
//!on P2.0 and 25% on P2.1.
//!SMCLK = MCLK = TACLK = default DCO ~1.045MHz.
//!
//!Tested On: MSP430F5529
//!     -------------------
//! /|\|                   |
//!  | |                   |
//!  --|RST                |
//!    |                   |
//!    |         P2.0/TA1.1|--> CCR1 - 75% PWM
//!    |         P2.1/TA1.2|--> CCR2 - 25% PWM
//!
//! This example uses the following peripherals and I/O signals.  You must
//! review these and change as needed for your own board:
//! - Timer peripheral
//! - GPIO Port peripheral
//!
//! This example uses the following interrupt handlers.  To use this example
//! in your own application you must add these interrupt handlers to your
//! vector table.
//! - NONE
//!
//
//*****************************************************************************

#include "driverlib.h"
#include <msp430.h>

#define TIMER_PERIOD 500
#define DUTY_CYCLE1 250
#define DUTY_CYCLE2 96

void main(void)
{
    //Stop WDT
    WDT_A_hold(WDT_A_BASE);


    PMM_unlockLPM5();

    //P1.1 output
    //P1.1 options select
    P1DIR |= BIT1;
    P1SEL0 &= ~BIT1;
    P1SEL1 |= BIT1;

    //Start Timer
    Timer_A_initUpDownModeParam initUpDownParam = {0};
    initUpDownParam.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    initUpDownParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    initUpDownParam.timerPeriod = TIMER_PERIOD;
    initUpDownParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    initUpDownParam.captureCompareInterruptEnable_CCR0_CCIE =
        TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE;
    initUpDownParam.timerClear = TIMER_A_DO_CLEAR;
    initUpDownParam.startTimer = false;
    Timer_A_initUpDownMode(TIMER_A0_BASE, &initUpDownParam);

    Timer_A_startCounter(TIMER_A0_BASE,
                         TIMER_A_UPDOWN_MODE
                         );

    //Initialze compare registers to generate PWM1
    Timer_A_initCompareModeParam initComp1Param = {0};
    initComp1Param.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    initComp1Param.compareInterruptEnable =
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    initComp1Param.compareOutputMode = TIMER_A_OUTPUTMODE_TOGGLE_SET;
    initComp1Param.compareValue = DUTY_CYCLE1;
    Timer_A_initCompareMode(TIMER_A0_BASE, &initComp1Param);


    //Enter LPM0
    __bis_SR_register(LPM0_bits);

    //For debugger
    __no_operation();
}
