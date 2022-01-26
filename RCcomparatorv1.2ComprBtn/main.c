#include <msp430.h> 
/* El principal inconveniente que encuentro en este programa es que no se como configurar como salida pwm otro
 * pin que no sea el 1.6; hay que mirar por ahi como hacerlo */
//Defino variables globales
int IncDec_PWM = 1; //Variable para

void P1init(void);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	//Configuro las entradas
    P1init();

    //Configuro el modo pwm
    TA0CCR0 |= 2000-1;		//El micro funciona a 1(?)MHz, por tanto pongo la salida a, mas o menos, 10KHz(?)
    TA0CCTL1 |= OUTMOD_7;	//Se pone en modo reset-set
    TA0CCR1 |= 1;		//El duty cycle de 1/2000, por lo que estara al 0.05% aprox.
    TA0CTL |= TASSEL_2 | MC_1;//Se pone el modo count_up y se selecciona el reloj interno

    // Ya no es necesaria la interrupción del timer A1.
    /*TA1CCR0 |= 1000-1; //Cuenta hasta este valor, por lo que genera una interrupcion cada 20000/n = x ms
    TA1CCTL0 |= CCIE;
    TA1CTL |= TASSEL_2 | MC_1;*/

    _BIS_SR(LPM0_bits + GIE); //Se pone el micro en low power mode y se activan las interrupciones
    while(1);

	return 0;
}

void P1init(void) {
	P1DIR = BIT6; //P1.6 como salida
	P1SEL = BIT6; //Seleccion del modo pwm de dichos pines
	P1OUT = BIT3; 	/* P1.3 pull up */
    P1REN = BIT3; // Pull-up resistor
    P1IFG=0;
    P1IES=BIT3;
	P1IE=BIT3;

} // P1init

#pragma vector = PORT1_VECTOR
__interrupt void P1_ISR(void) {
	if(P1IFG & BIT6) P1IFG &= ~BIT6;
	while(!(P1IFG & BIT3));
	TA0CCR1 += (80*IncDec_PWM); //El duty cycle se incrementa o decrementa en 0.05%*4=0.2%

	if( TA0CCR1 > 1900 || TA0CCR1 < 100 ) //Cuando la cuenta llega al máximo o al mínimo, se invierte la tendencia
	IncDec_PWM = -(80*IncDec_PWM);

	P1IFG &= ~(BIT3|BIT6); //Limpiar flag
}

