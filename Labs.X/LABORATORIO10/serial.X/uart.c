/*
 * File:   lab10.c
 * Dispositivo:	PIC16f887
 * Author: isaac
 * Compilador:	pic-as (v2.30), MPLABX V5.45
 * 
 * Programa: Transmición y recepción de datos via serial
 * Hardware: Pic 16f887, leds, resistencias, terminal serial.
 * 
 * Created on 04 de mayo de 2021, 8:16
 * Last modification on 04 de mayo de 2021, 19:19
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

#define _XTAL_FREQ 8000000 //__delay_ms(x)

//---------------------------variables------------------------------------------
                          
const char num_display[] = {97, 98, 99, 100, 101, 102, 103, 104, 105, 106};
                          
char espacio;
//---------------------------interrupciones-------------------------------------

void __interrupt()isr(void){
    
    if(PIR1bits.RCIF){  //chequeo si recibo datos
        PORTB = RCREG;  //lo paso al puerto B
    }

}


void main(void){

    ANSEL = 0X00;   //NO HAY PINES ANALOGOS
    ANSELH = 0x00;

    TRISA = 0x00; // PORTA todo salida
    TRISB = 0x00; // PORTB todo salida
    
    OSCCONbits.IRCF = 0b0111 ;  // config. de oscilador interno
    OSCCONbits.SCS = 1;         //reloj interno

                                //Confi. serial comunication
    TXSTAbits.SYNC = 0;     //asincrono
    TXSTAbits.BRGH = 1;     //high speed
    BAUDCTLbits.BRG16 = 1;  //uso los 16 bits
    
    SPBRG = 25;             
    SPBRGH = 0;             
    
    RCSTAbits.SPEN = 1;     //enciendo el modulo
    RCSTAbits.RX9 = 0;      //No trabajo a 9 bits
    RCSTAbits.CREN = 1;     //activo recepción
    TXSTAbits.TXEN = 1;     //activo transmision 
    
                            //confi. interrupciones
    PIR1bits.RCIF = 0;      //INTERRUPCION POR RECEPCIÓN DE DATOS
    PIE1bits.RCIE = 1; 
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    
    espacio = 0;
    PORTA = 0;
    PORTB = 0;


    //------------------------------loop principal----------------------------------
    while (1) {
        __delay_ms(500);
        
        if(PIR1bits.TXIF){
            TXREG = num_display[espacio];       //cada 500ms mando un dato
            espacio++;
            if(espacio == 10){
                espacio = 0;
            }
        }
        
    }
    return;     //end
}