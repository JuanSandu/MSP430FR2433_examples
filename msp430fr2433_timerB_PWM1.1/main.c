//*******************************************************************************
//!  Timer_A3, PWM TA1.2, Up Mode, DCO SMCLK
//!
//!  Description: This program generates PWM outputs on P2.2 using
//!  Timer1_A configured for up mode. The value , TIMER_PERIOD, defines the PWM
//!  period and the value DUTY_CYCLE the PWM duty cycle. Using ~1.048MHz
//!  SMCLK as TACLK, the timer period is ~480us with a 75% duty cycle on P4.0
//!  ACLK = n/a, SMCLK = MCLK = TACLK = default DCO ~1.048MHz.
//!
//!  Tested On:   MSP430FR4133
//!            -------------------
//!        /|\|                   |
//!         | |                   |
//!         --|RST                |
//!           |                   |
//!           |         P4.0/TA1.1|--> CCR1 - 75% PWM
//!           |                   |
//!
//! This example uses the following peripherals and I/O signals.  You must
//! review these and change as needed for your own board:
//! - Timer peripheral
//! - GPIO peripheral
//!
//! This example uses the following interrupt handlers.  To use this example
//! in your own application you must add these interrupt handlers to your
//! vector table.
//! - NONE
//******************************************************************************
#include "driverlib.h"

#define TIMER_A_PERIOD 625
#define DUTY_CYCLE  60


void main (void)
{
    //Stop WDT
    WDT_A_hold(WDT_A_BASE);

    //P1.0 as PWM output
    P1DIR |= BIT1;
    P1SEL0 &= ~BIT1;
    P1SEL1 |= BIT1;

    /*
     * Disable the GPIO power-on default high-impedance mode to activate
     * previously configured port settings
     */
    PMM_unlockLPM5();

    //Generate PWM - Timer runs in Up-Down mode
    Timer_A_outputPWMParam param = {0};
    param.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_32;
    param.timerPeriod = TIMER_A_PERIOD;
    param.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    param.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    param.dutyCycle = DUTY_CYCLE;
    Timer_A_outputPWM(TIMER_A0_BASE, &param);

    //Enter LPM0
    __bis_SR_register(LPM0_bits);

    //For debugger
    __no_operation();
}
