/*
 * COMPROBACIÓN DEL COMPARADOR
 *
 * Programa para comprobar el funcionamiento del comparador CA+. Se coloca un
 * potenciómetro con una pata a la alimentación (a través de otra resistencia
 * para evitar cortos) y la pata central a tierra. Se cablea la primera pata
 * al pin P1.1, entrada al comparador.
 *
 * Cada vez que se quiera comparar, se pulsa el botón del pin P1.3 y la
 * interrupción despierta al micro, desarrollándose el resto del programa.
 *
 * Elaborado por Juan Sandubete López.
 *
 */
#include <msp430.h> 

#define LED1    BIT0
#define LED2    BIT6
#define BTN1    BIT3
#define VCTL    BIT3
#define AIN1	BIT1

//Contador de interrupciones
int n=0;

// Declaración de funciones
void Pinit(void);
void CAinit(void);
void conf_reloj(char VEL);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    conf_reloj(16); // SMLCK 1Mhz
    Pinit(); //Configuración de pin-out
    CAinit(); //Configuración del Comparador

    while(1)
    {
    	_BIS_SR(LPM0_bits + GIE); //Siestecita
    	CACTL1 |= CAON; //Encender comparador
    	__delay_cycles(1000); //Dejo estabilizarse la medida
    	if((CAOUT & CACTL2)) //Sal Comp 1 y registro 1
    	{
    		P1OUT|=LED1; //Encender verde si tensión mayor
    		P1OUT &= ~(LED2);
    	}
    	else if(!(CAOUT & CACTL2)) //Sal Com 0 y registro 0
    	{
    		P1OUT|=LED2; //Encender rojo si tensión menor
    		P1OUT &= ~(LED1);
    	}
    	CACTL1 &= ~CAON; // Apagar comparador
    	__delay_cycles(10); //Me aseguro de que se apague antes de LPM0
    }

	return 0;
}

#pragma vector = PORT1_VECTOR
__interrupt void P1_ISR(void) {
	P1IFG &= ~(BTN1); //Limpiar flag
	while(!(P1IN & BTN1)); //Esperar a que deje de pulsar
	CACTL1 &= ~(CAIFG);
	n++;
	LPM0_EXIT;
}

void Pinit(void) {
    P1OUT = (LED2|BTN1);    // default LED2 (green) on to indicate ready.
    P1DIR = LED1 + LED2; // output on P1.0 and P1.6 for LEDs,

    P1REN = BTN1; // Pull-up resistor
    P1IFG = 0;
    P1IES = BTN1;   // falling edge for pulled-up button
    P1IE = BTN1;    // enable interrupt for BTN1

} // Pin init = Pinit

void CAinit(void) {
    CACTL1 = CARSEL + CAREF_2;   // 0.25 Vcc ref on - pin.
    CACTL2 = P2CA4 + CAF;       // Input CA1 on + pin, filter output.
} // CAinit


void conf_reloj(char VEL){
	BCSCTL2 = SELM_0 | DIVM_0 | DIVS_0;
	switch(VEL){
	case 1:
		if (CALBC1_1MHZ != 0xFF) {
			DCOCTL = 0x00;
			BCSCTL1 = CALBC1_1MHZ;      /* Set DCO to 1MHz */
			DCOCTL = CALDCO_1MHZ;
		}
		break;
	case 8:

		if (CALBC1_8MHZ != 0xFF) {
			__delay_cycles(100000);
			DCOCTL = 0x00;
			BCSCTL1 = CALBC1_8MHZ;      /* Set DCO to 8MHz */
			DCOCTL = CALDCO_8MHZ;
		}
		break;
	case 12:
		if (CALBC1_12MHZ != 0xFF) {
			__delay_cycles(100000);
			DCOCTL = 0x00;
			BCSCTL1 = CALBC1_12MHZ;     /* Set DCO to 12MHz */
			DCOCTL = CALDCO_12MHZ;
		}
		break;
	case 16:
		if (CALBC1_16MHZ != 0xFF) {
			__delay_cycles(100000);
			DCOCTL = 0x00;
			BCSCTL1 = CALBC1_16MHZ;     /* Set DCO to 16MHz */
			DCOCTL = CALDCO_16MHZ;
		}
		break;
	default:
		if (CALBC1_1MHZ != 0xFF) {
			DCOCTL = 0x00;
			BCSCTL1 = CALBC1_1MHZ;      /* Set DCO to 1MHz */
			DCOCTL = CALDCO_1MHZ;
		}
		break;

	}
	BCSCTL1 |= XT2OFF | DIVA_0;
	BCSCTL3 = XT2S_0 | LFXT1S_2 | XCAP_1;

} // Configuracion de reloj
