/*
 * COMPROBACIÓN DE SINCRONIZACIÓN COMP-TIMER
 *
 * Programa para comprobar el funcionamiento del comparador CA+ en conjunto
 * con el timer. Se trata de medir el tiempo que pasa entre que la entrada del
 * comparador es mayor que la referencia y que es menor que ésta.
 *
 * Se coloca un potenciómetro con una pata a la alimentación (a través de otra
 * resistencia para evitar cortos) y la pata central a tierra. Se cablea la
 * primera pata al pin P1.1, entrada al comparador.
 *
 * Cada vez que se quiera comparar, se pulsa el botón del pin P1.3 y la
 * interrupción despierta al micro, desarrollándose el resto del programa.
 * Mientras se hace la medida de tiempo, el timer sigue activándose, incrementando
 * la variable de conteo de overflow. Finalmente, cuando acaba la medida, la
 * interrupción del comparador se encarga de despertar al micro de la hibernación
 * y terminar de desarrollar las acciones necesarias.
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

// Variables globales
int n=0; //Contador de interrupciones
int cont=0; //Contador de overflows
unsigned int t=0; // Variable de tiempo

// Declaración de funciones
void Pinit(void);
void CAinit(void);
void conf_reloj(char VEL);
void TAinit(void);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    conf_reloj(1); // SMLCK 1Mhz
    Pinit(); //Configuración de pin-out
    CAinit(); //Configuración del Comparador
    TAinit(); //Conf Timer

    while(1)
    {
    	TA1CCTL0 &= (~CCIE); //Deshabilitar interrupción si no está midiendo
    	_BIS_SR(LPM0_bits + GIE); //Siestecita

    	TA1R=0; //Resetear contador
    	TA1CCTL0 |= CCIE; //Habilitar interrupción previa medición
    	TA1CTL |= MC_2; //Iniciar timer
    	CACTL1 |= CAON; //Encender comparador
    	__delay_cycles(100); //Dejo estabilizarse la medida

    	P1OUT &= ~(LED1);
    	P1OUT|=LED2; //Encender rojo mientras mide

    	_BIS_SR(LPM0_bits + GIE); //Siestecita mientras mide

    	TA1CCTL0 &= (~CCIE); //Deeshabilitar antes de medir
    	TA1CTL &= ~MC_2; //Apagar timer
    	t=TA1R; //Guardar valor contado

    	P1OUT &= ~(LED2);
   		P1OUT|=LED1; //Encender verde si termina de medir

    	CACTL1 &= ~CAON; // Apagar comparador
    	__delay_cycles(100); //Me aseguro de que se apague antes de LPM0
    }

	return 0;
}

#pragma vector = PORT1_VECTOR
__interrupt void P1_ISR(void) {
	P1IFG &= ~(BTN1); //Limpiar flag
	while(!(P1IN & BTN1)); //Esperar a que deje de pulsar
	n++;
	LPM0_EXIT;
}

#pragma vector = COMPARATORA_VECTOR
__interrupt void AC_ISR(void){
	CACTL1 &= ~(CAIFG); //Limpiar flag
	LPM0_EXIT; //Salir de hibernación
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TIM0_ISR(void){
	TA1CCTL0 &= (~CCIFG); //Limpiar flag
	cont++; //El contador ha dado una vuelta
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
    CACTL1 = CARSEL + CAREF_2 + CAIE;   // 0.25 Vcc ref on - pin. Habilitar int.
    CACTL2 = P2CA4 + CAF;       // Input CA1 on + pin, filter output.
} // CAinit

void TAinit(void) {
	TA1CCTL0 = CM_0 | CCIS_0 | OUTMOD_0 | CCIE; //Sin captura, CCIxA, solo salida, int enable
	TA1CCR0 = 9999; //1Mhz/10000=100Hz -> Int cada 10ms
	TA1CTL = TASSEL_2 | ID_0 | MC_0; //SMCLK, sin div, parado
} // TAinit

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
