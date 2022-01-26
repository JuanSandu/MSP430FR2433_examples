//******************************************************************************
//!
//! ADC - Sample A2 input, 3.0V? (igual es 1.5) Ref, LED ON if A2 > 0.5V
//!
//!  MSP430FR2433
//!  Ejemplo para leer la tension en los pines P1.2 y P1.4
//!  La utilidad del ejemplo es la de ver como pasar de la lectura de un canal
//!  Como prueba adicional, se utiliza el timer A1 en lugar del A0.
//!
//******************************************************************************
#include "driverlib.h"
#include "Board.h"

#define TIMER_PERIOD 80
int16_t ADC_Result[2]; // Vector para guardar resultados
int16_t i=0, adc_cont=0;


void main (void)
{
    //Stop Watchdog Timer
    WDT_A_hold(WDT_A_BASE);

    //Set A7 as an input pin.
    //Set appropriate module function
    SYSCFG2=(ADCPCTL2|ADCPCTL4); // Se habilitan p1.2 y p1.4 como entrada analogica

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
        ADC_INPUT_A4,
        ADC_VREFPOS_AVCC,
        ADC_VREFNEG_AVSS);

    ADC_clearInterrupt(ADC_BASE,
        ADC_COMPLETED_INTERRUPT);

    //Enable Memory Buffer interrupt
    ADC_enableInterrupt(ADC_BASE,
        ADC_COMPLETED_INTERRUPT);


    // Configure TA1 to provide delay for reference settling ~75us
    Timer_A_initUpModeParam initUpModeParam = {0};
    initUpModeParam.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    initUpModeParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    initUpModeParam.timerPeriod = TIMER_PERIOD;
    initUpModeParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    initUpModeParam.captureCompareInterruptEnable_CCR0_CCIE =
        TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    initUpModeParam.timerClear = TIMER_A_DO_CLEAR;
    initUpModeParam.startTimer = true;
    Timer_A_initUpMode(TIMER_A1_BASE, &initUpModeParam);

    __bis_SR_register(CPUOFF + GIE);           // LPM0, TA0_ISR will force exit

    for (;;)
    {
        //Delay between conversions
        __delay_cycles(5000);

        //Enable and Start the conversion
        //in Single-Channel, Single Conversion Mode
        ADC_startConversion(ADC_BASE,
                 ADC_SEQOFCHANNELS);

        //LPM0, ADC_ISR will force exit
        __bis_SR_register(CPUOFF + GIE);
        //For debug only
        //__no_operation();


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
    switch (__even_in_range(ADCIV,ADCIV_ADCIFG)){
        case  ADCIV_NONE: break; //No interrupt
        case  ADCIV_ADCOVIFG: break; //conversion result overflow
        case  ADCIV_ADCTOVIFG: break; //conversion time overflow
        case  ADCIV_ADCHIIFG: break; //ADCHI
        case  ADCIV_ADCLOIFG: break; //ADCLO
        case  ADCIV_ADCINIFG: break; //ADCIN
        case  ADCIV_ADCIFG:        //ADCIFG0

            //Automatically clears ADCIFG0 by reading memory buffer
            //ADCMEM = A0 > 0.5V?
            // ADC_Result[i] = ADCMEM0; es lo mismo que ADC_Result[i]=ADC_getResults(ADC_BASE);


            if(adc_cont == 0)
            {
                ADC_Result[i] = ADCMEM0;
                // Lectura de Pin 1.4
                if (ADC_Result[i] < 500) {

                    //Turn LED2 off
                    GPIO_setOutputLowOnPin(
                        GPIO_PORT_LED2,
                        GPIO_PIN_LED2
                    );
                }
                else {
                    //Turn LED2 on
                    GPIO_setOutputHighOnPin(
                        GPIO_PORT_LED2,
                        GPIO_PIN_LED2
                    );
                }
                i++;
                adc_cont++;
            }
            else if (adc_cont==1)
            {
                // Lectura del Pin 1.3 no habilitada
                // se pasa el turno leyendo el buffer
                int16_t foo1 = ADCMEM0; // No useful data here
                adc_cont++;
            }
            else
            {
                ADC_Result[i] = ADCMEM0;
                // Lectura de Pin 1.2
                if (ADC_Result[i] < 500) {
                    //Turn LED1 off
                    GPIO_setOutputLowOnPin(
                        GPIO_PORT_LED1,
                        GPIO_PIN_LED1
                    );
                }
                else {
                    //Turn LED1 on
                    GPIO_setOutputHighOnPin(
                        GPIO_PORT_LED1,
                        GPIO_PIN_LED1
                    );

                }
                i--;
                adc_cont=0;
            }

            //Clear CPUOFF bit from 0(SR)
            //Breakpoint here and watch ADC_Result
            __bic_SR_register_on_exit(CPUOFF);
            break;
        default: break;
    }
}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(TIMER1_A0_VECTOR)))
#endif
void TA1_ISR (void)
{
      TA1CTL = 0;
      LPM0_EXIT;                                // Exit LPM0 on return
}
