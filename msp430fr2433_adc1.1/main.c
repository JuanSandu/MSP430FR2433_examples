//******************************************************************************
//!
//! ADC - Sample A2 input, 3.0V? (igual es 1.5) Ref, LED ON if A2 > 0.5V
//!
//!  MSP430FR2433
//!  Ejemplo para leer la tension en el pin 1.2
//!  Si pones un sensor, un potenciometro o algo para variar la tension, cuando
//!  pasa del limite puesto, se encienden los dos leds
//!
//******************************************************************************
#include "driverlib.h"
#include "Board.h"

#define TIMER_PERIOD 80
int16_t adcvalue=0;
int16_t buff;


void main (void)
{
    //Stop Watchdog Timer
    WDT_A_hold(WDT_A_BASE);

    //Set A7 as an input pin.
    //Set appropriate module function
    SYSCFG2=ADCPCTL2; // Se habilita el pin 1.2 como entrada analogica

    //Set LED1 as an output pin.
    GPIO_setAsOutputPin(
            GPIO_PORT_LED1,
            GPIO_PIN_LED1);

    //Set LED2 as an output pin.
    GPIO_setAsOutputPin(
            GPIO_PORT_LED2,
            GPIO_PIN_LED2);

    /*
     * Disable the GPIO power-on default high-impedance mode to activate
     * previously configured port settings
     */
    PMM_unlockLPM5();

    //Initialize the ADC Module
    /*
     * Base Address for the ADC Module
     * Use internal ADC bit as sample/hold signal to start conversion
     * USE MODOSC 5MHZ Digital Oscillator as clock source
     * Use default clock divider of 1
     */
    ADC_init(ADC_BASE,
        ADC_SAMPLEHOLDSOURCE_SC,
        ADC_CLOCKSOURCE_ADCOSC,
        ADC_CLOCKDIVIDER_1);

    ADC_enable(ADC_BASE);

    /*
     * Base Address for the ADC Module
     * Sample/hold for 16 clock cycles
     * Do not enable Multiple Sampling
     */
    ADC_setupSamplingTimer(ADC_BASE,
        ADC_CYCLEHOLD_16_CYCLES,
        ADC_MULTIPLESAMPLESDISABLE);

    //Configure Memory Buffer
    /*
     * Base Address for the ADC Module
     * Use input A2
     * Use positive reference of Internally generated Vref
     * Use negative reference of AVss
     */
    ADC_configureMemory(ADC_BASE,
        ADC_INPUT_A2,
        ADC_VREFPOS_AVCC,
        ADC_VREFNEG_AVSS);

    ADC_clearInterrupt(ADC_BASE,
        ADC_COMPLETED_INTERRUPT);

    //Enable Memory Buffer interrupt
    ADC_enableInterrupt(ADC_BASE,
        ADC_COMPLETED_INTERRUPT);


    // Configure TA0 to provide delay for reference settling ~75us
    Timer_A_initUpModeParam initUpModeParam = {0};
    initUpModeParam.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    initUpModeParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    initUpModeParam.timerPeriod = TIMER_PERIOD;
    initUpModeParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    initUpModeParam.captureCompareInterruptEnable_CCR0_CCIE =
        TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    initUpModeParam.timerClear = TIMER_A_DO_CLEAR;
    initUpModeParam.startTimer = true;
    Timer_A_initUpMode(TIMER_A0_BASE, &initUpModeParam);

    __bis_SR_register(CPUOFF + GIE);           // LPM0, TA0_ISR will force exit

    for (;;)
    {
        //Delay between conversions
        __delay_cycles(5000);

        //Enable and Start the conversion
        //in Single-Channel, Single Conversion Mode
        ADC_startConversion(ADC_BASE,
                ADC_SINGLECHANNEL);

        //LPM0, ADC_ISR will force exit
        __bis_SR_register(CPUOFF + GIE);
        //For debug only
        __no_operation();


    }
}

//ADC interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(ADC_VECTOR)))
#endif
void ADC_ISR (void)
{
    switch (__even_in_range(ADCIV,12)){
        case  0: break; //No interrupt
        case  2: break; //conversion result overflow
        case  4: break; //conversion time overflow
        case  6: break; //ADCHI
        case  8: break; //ADCLO
        case 10: break; //ADCIN
        case 12:        //ADCIFG0

            //Automatically clears ADCIFG0 by reading memory buffer
            //ADCMEM = A0 > 0.5V?
            // buff = ADCMEM0; es lo mismo que adcvalue=ADC_getResults(ADC_BASE);
            adcvalue=ADC_getResults(ADC_BASE);
            if (adcvalue < 0x155) {
                //Turn LED1 off
                GPIO_setOutputLowOnPin(
                    GPIO_PORT_LED1,
                    GPIO_PIN_LED1
                );

                //Turn LED2 off
                GPIO_setOutputLowOnPin(
                    GPIO_PORT_LED2,
                    GPIO_PIN_LED2
                );
            }
            else {
                //Turn LED1 on
                GPIO_setOutputHighOnPin(
                    GPIO_PORT_LED1,
                    GPIO_PIN_LED1
                );

                //Turn LED2 on
                GPIO_setOutputHighOnPin(
                    GPIO_PORT_LED2,
                    GPIO_PIN_LED2
                );
            }

            //Clear CPUOFF bit from 0(SR)
            //Breakpoint here and watch ADC_Result
            __bic_SR_register_on_exit(CPUOFF);
            break;
        default: break;
    }
}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(TIMER0_A0_VECTOR)))
#endif
void TA0_ISR (void)
{
      TA0CTL = 0;
      LPM0_EXIT;                                // Exit LPM0 on return
}
