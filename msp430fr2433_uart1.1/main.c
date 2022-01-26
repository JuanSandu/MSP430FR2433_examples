
//******************************************************************************
//!  EUSCI_A0 External Loopback test using EUSCI_A_UART_init API
//!
//!  Description: This demo sends some data (string) through UART
//!
//!  SMCLK = MCLK = BRCLK = DCOCLKDIV = ~1MHz, ACLK = 32.768kHz
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
char TXData[] = {"MSP430fr2433 dice: Funciona.\r\n"};
uint8_t check = 0;
int cont = 0;

void main(void)
{
    //Stop Watchdog Timer
    WDT_A_hold(WDT_A_BASE);

    //Set ACLK = REFOCLK with clock divider of 1
    CS_initClockSignal(CS_ACLK,CS_REFOCLK_SELECT,CS_CLOCK_DIVIDER_1);
    //Set SMCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_SMCLK,CS_DCOCLKDIV_SELECT,CS_CLOCK_DIVIDER_1);
    //Set MCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_MCLK,CS_DCOCLKDIV_SELECT,CS_CLOCK_DIVIDER_1);

    //Configure UART pins
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_UCA0TXD,
        GPIO_PIN_UCA0TXD,
        GPIO_FUNCTION_UCA0TXD
    );
    /*GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_UCA0RXD,
        GPIO_PIN_UCA0RXD,
        GPIO_FUNCTION_UCA0RXD
    );*/

    /*
     * Disable the GPIO power-on default high-impedance mode to activate
     * previously configured port settings
     */
    PMM_unlockLPM5();

    //Configure UART
    //SMCLK = 1MHz, Baudrate = 115200
    //UCBRx = 8, UCBRFx = 0, UCBRSx = 0xD6, UCOS16 = 0
    EUSCI_A_UART_initParam param = {0};
    param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    param.clockPrescalar = 8;
    param.firstModReg = 0;
    param.secondModReg = 0xD6;
    param.parity = EUSCI_A_UART_NO_PARITY;
    param.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
    param.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
    param.uartMode = EUSCI_A_UART_MODE;
    param.overSampling = EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;

    if (STATUS_FAIL == EUSCI_A_UART_init(EUSCI_A0_BASE, &param)) {
        return;
    }

    EUSCI_A_UART_enable(EUSCI_A0_BASE);

    EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE,
        EUSCI_A_UART_TRANSMIT_INTERRUPT);

    // Enable USCI_A0 TX interrupt
    EUSCI_A_UART_enableInterrupt(EUSCI_A0_BASE,
        EUSCI_A_UART_TRANSMIT_INTERRUPT);

    // Enable global interrupts
    __enable_interrupt();

    int MAX_long = sizeof(TXData);
    while (1)
    {
        // Load data onto buffer
        EUSCI_A_UART_transmitData(EUSCI_A0_BASE, TXData[i]);

        i++;
        if(i==MAX_long)
        {
            i=0; // Variable reset
        }
        while(check != 1);
        check = 0;
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
    // Se limpia la interrupcion cuando se sabe que se ha terminado de enviar un byte
    EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE,
            EUSCI_A_UART_TRANSMIT_INTERRUPT);
    check =1;
    //cont++;

}

