/*
 * File:   lab7.c
 * Dispositivo:	PIC16f887
 * Author: isaac
 * Compilador:	pic-as (v2.30), MPLABX V5.45
 * 
 * Programa: Contador de tiempo a cada 5ms, contador que incrementa o decrementa
 *           por push buttons y el valor de este contador se ve en decimal
 * Hardware: Pic 16f887, transistores, resistencias, leds, displays 7 seg,button
 * 
 * Created on 13 de abril de 2021, 8:45
 * Last modification on 13 de abril de 2021, 17:50
 */

// PIC16F887 Configuration Bit Settings
// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (RB3/PGM pin has PGM function, low voltage programming enabled)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

//-------------------------Librerias--------------------------------------------
#include <xc.h>
#include <stdint.h>

//--------------------------directivas del compilador---------------------------
#define _tmr0_value 100;

//---------------------------variables------------------------------------------
char centena, decena, unidad;
                          // 0     1    2     3     4
const char num_display[] = {0xFC, 0x60, 0xDA, 0xF2, 0x66,
                            0xB6, 0xBE, 0xE0, 0xFE, 0xF6};
                          //  5     6     7    8     9
char dividendo, divisor;
char turno;

//---------------------------interrupciones-------------------------------------

void __interrupt()isr(void) {

    if (T0IF == 1) { //INTERRUPCION POR TIMMER0
        PORTD++;
        PORTB = 0;                          //turn off, displays
        if (turno == 3) {
            PORTC = (num_display[centena]);
            RB5 = 1;                        //turn on only this display
        } 
        else if (turno == 2) {
            PORTC = (num_display[decena]);
            RB6 = 1;                        //turn on only this display
        }
        else if (turno == 1) {
            PORTC = (num_display[unidad]);
            RB7 = 1;                        //turn on only this display
        }
        
        turno--;
        if (turno == 0){
            turno = 3;                      //start again
        }

        INTCONbits.T0IF = 0; //bajo la bandera
        TMR0 = _tmr0_value; //reinicio timer
    } 
    else if (RBIF == 1) { //INTERRUPCION POR CAMBIO DE ESTADO
        if (RB0 == 0) {
            PORTA++;        //incremento puerto
            RBIF = 0;
        } else if (RB1 == 0) {
            PORTA--;        //decremento puerto
            RBIF = 0;
        }

    }

}

//----------------------configuracion microprocesador---------------------------

void main(void){

    ANSEL = 0x00; //No hay i/o analogicas
    ANSELH = 0x00;

    TRISA = 0x00; // PORTA todo salida
    TRISB = 0b00000011; // PORTB todo salida a excepcion de los primeros 2 bits
    TRISC = 0x00; // PORTC todo salida
    TRISD = 0x00; // PORTD todo salida

    OPTION_REGbits.nRBPU = 0; //config. de PULL-UP
    WPUBbits.WPUB0 = 1;
    WPUBbits.WPUB1 = 1;

    OSCCONbits.IRCF2 = 1; //Config. de oscilacion
    OSCCONbits.IRCF1 = 1; //Pre-escaler a 8MHz (111))
    OSCCONbits.IRCF0 = 1;
    OSCCONbits.SCS = 1; //reloj interno

    OPTION_REGbits.T0CS = 0; //Uso reloj interno
    OPTION_REGbits.PSA = 0; //Uso pre-escaler
    OPTION_REGbits.PS2 = 1;
    OPTION_REGbits.PS1 = 0;
    OPTION_REGbits.PS0 = 1; //PS = 101 / 1:64
    TMR0 = 100; //Reinicio del timmer

    IOCBbits.IOCB0 = 1; // pines en lo que habra interrupcion por cambio 
    IOCBbits.IOCB1 = 1; //de estado
    RBIF = 0;

    INTCONbits.GIE = 1; //habilito interrupciones
    INTCONbits.T0IE = 1; //activo interrupciones por timmer 0
    INTCONbits.T0IF = 0; //bajo la bandera
    INTCONbits.RBIE = 1; //activo interrupciones por cambio de estado
    INTCONbits.RBIF = 0; //bajo la bandera

    PORTA = 0; // Estado inicial de los pines
    PORTB = 0; // Estado inicial de los pines
    PORTC = 0; // Estado inicial de los pines
    PORTD = 0; // Estado inicial de los pines

    centena = 0; //valor inicial de las variables
    decena = 0;
    unidad = 0;
    turno = 3;

    //------------------------------loop principal----------------------------------
    while (1) {

        dividendo = PORTA;                          //copy el valor
        centena = dividendo / 100;                  //saco centenas
        decena = (dividendo - (100 * centena))/10;  //saco decenas
        unidad = dividendo - (100 * centena) - (decena * 10);   //saco unidades

    }
    return;     //end
}
