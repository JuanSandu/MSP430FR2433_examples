
//******************************************************************************
//!  EUSCI_A0 External Loopback test using EUSCI_A_UART_init API
//!
//!  Description: This demo sends some data (string) through UART
//!  The main difference with uart1.1 is the clock configuration
//!
//!  SMCLK = MCLK = BRCLK = DCOCLKDIV = ~16MHz, ACLK = 32.768kHz
//!
//!
//!           MSP430FR2xx_4xx Board
//!             -----------------
//!       RST -|          UCA0TXD|----|
//!            |                 |    |
//!            |                 |    |
//!            |          UCA0RXD|----|
//!            |                 |
//!
//! This example uses the following peripherals and I/O signals. You must
//! review these and change as needed for your own board:
//! - UART peripheral
//! - GPIO Port peripheral (for UART pins)
//! - UCA0TXD
//!
//******************************************************************************
#include "driverlib.h"
#include "Board.h"
#include "string.h"

volatile unsigned int i=0;
uint8_t RXData = 0;
char TXData[] = {"MSP430fr2433 dice: Funciona."};
char TXRespuesta[] = {"MSP430fr2433 dice: Se ha recibido "};
char TXRetorno[]={"\r\n"};
uint8_t check = 0;
int cont = 0;
uint32_t clockValue = 0;

void main(void)
{
    //Stop Watchdog Timer
    WDT_A_hold(WDT_A_BASE);

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


    // Se activa el detector de faltas de CLK
    SFR_enableInterrupt(SFR_OSCILLATOR_FAULT_INTERRUPT);

    //Configure UART pins
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_UCA0TXD,
        GPIO_PIN_UCA0TXD,
        GPIO_FUNCTION_UCA0TXD
    );
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_UCA0RXD,
        GPIO_PIN_UCA0RXD,
        GPIO_FUNCTION_UCA0RXD
    );

    // Se comprueba si la frecuencia es realmente la esperada
    clockValue = CS_getMCLK();

    /*
     * Disable the GPIO power-on default high-impedance mode to activate
     * previously configured port settings
     */
    PMM_unlockLPM5();

    // UART CONFIGURATION
    //-------------------
    //SMCLK = 16MHz, Baudrate = 115200
    //UCBRx = 8, UCBRFx = 10, UCBRSx = 0xFB, UCOS16 = 1
    EUSCI_A_UART_initParam param = {0};
    param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    param.clockPrescalar = 8;
    param.firstModReg = 10;
    param.secondModReg = 0xFB;
    param.parity = EUSCI_A_UART_NO_PARITY;
    param.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
    param.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
    param.uartMode = EUSCI_A_UART_MODE;
    param.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;

    if (STATUS_FAIL == EUSCI_A_UART_init(EUSCI_A0_BASE, &param)) {
        return;
    }

    EUSCI_A_UART_enable(EUSCI_A0_BASE);

    EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE,
        EUSCI_A_UART_TRANSMIT_INTERRUPT);

    // Enable USCI_A0 TX interrupt
    EUSCI_A_UART_enableInterrupt(EUSCI_A0_BASE,
        EUSCI_A_UART_TRANSMIT_INTERRUPT);

    EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE,
        EUSCI_A_UART_RECEIVE_INTERRUPT);

    // Enable USCI_A0 RX interrupt
    EUSCI_A_UART_enableInterrupt(EUSCI_A0_BASE,
        EUSCI_A_UART_RECEIVE_INTERRUPT);

    // Enable global interrupts
    __enable_interrupt();

    int MAX_long = 0;
    while (1)
    {
        if(RXData)
        {
            MAX_long = sizeof(TXData);
            for(i=0;i<MAX_long;i++)
            {
                EUSCI_A_UART_transmitData(EUSCI_A0_BASE, TXData[i]);
                while(check != 1);
                check=0;
            }

            MAX_long = sizeof(TXRetorno);
            // Fin de la linea
            for(i=0;i<MAX_long;i++)
            {
                EUSCI_A_UART_transmitData(EUSCI_A0_BASE, TXRetorno[i]);
                while(check != 1);
                check=0;
            }

            // Respuesta
            MAX_long = sizeof(TXRespuesta);
            for(i=0;i<MAX_long;i++)
            {
                EUSCI_A_UART_transmitData(EUSCI_A0_BASE, TXRespuesta[i]);
                while(check != 1);
                check=0;
            }

            // Mensaje recibido
            EUSCI_A_UART_transmitData(EUSCI_A0_BASE, RXData);
            while(check != 1);
            check=0;

            MAX_long = sizeof(TXRetorno);
            // Fin de la linea
            for(i=0;i<MAX_long;i++)
            {
                EUSCI_A_UART_transmitData(EUSCI_A0_BASE, TXRetorno[i]);
                while(check != 1);
                check=0;
            }

            RXData = 0; // Se pone a cero de nuevo hasta el siguiente mensaje
        }
    }
}
//******************************************************************************
//
//This is the USCI_A0 interrupt vector service routine.
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_A0_VECTOR)))
#endif
void EUSCI_A0_ISR(void)
{
    switch(__even_in_range(UCA0IV,USCI_UART_UCTXCPTIFG))
    {
        case USCI_NONE: break;
        case USCI_UART_UCRXIFG:
            RXData = EUSCI_A_UART_receiveData(EUSCI_A0_BASE);
            __delay_cycles(10000);
            EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE,
                   EUSCI_A_UART_RECEIVE_INTERRUPT);
            break;
       case USCI_UART_UCTXIFG:
           // Se limpia la interrupcion cuando se sabe que se ha terminado de enviar un byte
           EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE,
                   EUSCI_A_UART_TRANSMIT_INTERRUPT);
           check=1; // Se ha terminado de enviar y se puede enviar el siguiente caracter
           break;
       case USCI_UART_UCSTTIFG: break;
       case USCI_UART_UCTXCPTIFG: break;
    }
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=UNMI_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(UNMI_VECTOR)))
#endif
void NMI_ISR(void)
{
    // If it still can't clear the oscillator fault flags after the timeout,
    // trap and wait here.
    CS_clearAllOscFlagsWithTimeout(1000);
}
