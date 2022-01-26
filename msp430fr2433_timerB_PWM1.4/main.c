//*******************************************************************************
//!  Timer_A3, PWM TA1.2, Up Mode, DCO SMCLK
//!
//!  Description: This program generates PWM outputs on P1.1 at 50Hz for controlling
//!  SG90 Servo. When pressing S1, angle grows, when pressing S2, angle drops
//!
//!  Tested On:   MSP430FR2433
//!            -------------------
//!        /|\|                   |
//!         | |                   |
//!         --|RST                |
//!           |                   |
//!           |         P1.1/TA0.1|--> CCR1 - SG90 PWM
//!           |                   |
//!
//! This example uses the following peripherals and I/O signals.  You must
//! review these and change as needed for your own board:
//! - Timer peripheral
//! - GPIO peripheral (switches)
//!
//! This example uses the following interrupt handlers.  To use this example
//! in your own application you must add these interrupt handlers to your
//! vector table.
//! - NONE
//******************************************************************************
#include "driverlib.h"
#include "Board.h"

#define TIMER_A_PERIOD 5000
uint16_t DUTY_CYCLE=405;
uint16_t estadoS1, estadoS2;
Timer_A_outputPWMParam param = {0};

void main (void)
{
    //Stop Watchdog Timer
    WDT_A_hold(WDT_A_BASE);

    // CLOCK CONFIGURATION
    //--------------------
    //Set DCO FLL reference = REFO
    CS_initClockSignal(CS_FLLREF,CS_REFOCLK_SELECT,CS_CLOCK_DIVIDER_1);
    // DCO to 16MHz. 16MHz/32.768kHz~488 -> El anterior entero: 487
    CS_initFLLSettle(16000,487);
    CS_clearAllOscFlagsWithTimeout(10000);

    //Set ACLK = REFOCLK with clock divider of 1
    CS_initClockSignal(CS_ACLK,CS_REFOCLK_SELECT,CS_CLOCK_DIVIDER_1);
    //Set SMCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_SMCLK,CS_DCOCLKDIV_SELECT,CS_CLOCK_DIVIDER_1);
    //Set MCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_MCLK,CS_DCOCLKDIV_SELECT,CS_CLOCK_DIVIDER_1);


     // Se activa el detector de faltas de CLK
     //SFR_enableInterrupt(SFR_OSCILLATOR_FAULT_INTERRUPT);

    //P1.1 as PWM output
    P1DIR |= BIT1;
    P1SEL0 &= ~BIT1;
    P1SEL1 |= BIT1;

    //Enable S1 internal resistance as pull-Up resistance
    GPIO_setAsInputPinWithPullUpResistor(
        GPIO_PORT_S1,
        GPIO_PIN_S1
        );
    //Enable S1 internal resistance as pull-Up resistance
    GPIO_setAsInputPinWithPullUpResistor(
        GPIO_PORT_S2,
        GPIO_PIN_S2
        );

    //S1 interrupt enabled
    GPIO_enableInterrupt(
        GPIO_PORT_S1,
        GPIO_PIN_S1
        );
    //S1 interrupt enabled
    GPIO_enableInterrupt(
        GPIO_PORT_S2,
        GPIO_PIN_S2
        );

    //S1 Hi/Lo edge
    GPIO_selectInterruptEdge(
        GPIO_PORT_S1,
        GPIO_PIN_S1,
        GPIO_HIGH_TO_LOW_TRANSITION
        );
    //S1 Hi/Lo edge
    GPIO_selectInterruptEdge(
        GPIO_PORT_S2,
        GPIO_PIN_S2,
        GPIO_HIGH_TO_LOW_TRANSITION
        );


    /*
     * Disable the GPIO power-on default high-impedance mode to activate
     * previously configured port settings
     */
    PMM_unlockLPM5();

    //Generate PWM - Timer runs in Up-Down mode
    param.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
    param.timerPeriod = TIMER_A_PERIOD;
    param.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    param.compareOutputMode =TIMER_A_OUTPUTMODE_RESET_SET;
    param.dutyCycle = DUTY_CYCLE;
    Timer_A_outputPWM(TIMER_A0_BASE, &param);

    //Enter LPM0
    __bis_SR_register(LPM0_bits + GIE);

    //For debugger
    __no_operation();
}


//******************************************************************************
//
//This is the PORT2_VECTOR interrupt vector service routine
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT2_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(PORT2_VECTOR)))
#endif
void P2_ISR (void)
{
    estadoS1 = GPIO_getInterruptStatus(GPIO_PORT_S1,GPIO_PIN_S1);
    GPIO_clearInterrupt(GPIO_PORT_S1,GPIO_PIN_S1);
    estadoS2 = GPIO_getInterruptStatus(GPIO_PORT_S2,GPIO_PIN_S2);
    GPIO_clearInterrupt(GPIO_PORT_S2,GPIO_PIN_S2);

    if (estadoS1==8){
        DUTY_CYCLE=DUTY_CYCLE-20;
        if (DUTY_CYCLE<150)
        {
            DUTY_CYCLE=150;
        }
        TA0CCR1=200;
        while(GPIO_getInputPinValue(GPIO_PORT_S1,GPIO_PIN_S1));
    }
    if (estadoS2==128){
        DUTY_CYCLE=DUTY_CYCLE+30;
        if (DUTY_CYCLE>620)
        {
            DUTY_CYCLE=620;
        }
        TA0CCR1=600;
        while(GPIO_getInputPinValue(GPIO_PORT_S2,GPIO_PIN_S2));
    }
}
