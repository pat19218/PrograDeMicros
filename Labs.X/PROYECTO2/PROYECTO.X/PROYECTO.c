/*
 * File:   PROYECTO.c
 * Dispositivo:	PIC16f887
 * Author: isaac
 *
 * Compilador:	pic-as (v2.30), MPLABX V5.45
 * 
 * Programa: Dispensador de pastillas con sistema de ventilacion y usuario
 * Hardware: Pic 16f887, transistores, resistencias, leds, button
 * 
 * Created on 14 de mayo de 2021, 21:51
 * Last modification on 14 de mayo de 2021, 23:00
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
#define _XTAL_FREQ 4000000 //__delay_ms(x)

//---------------------------variables------------------------------------------


//---------------------------interrupciones-------------------------------------

void __interrupt()isr(void) {

  

}

//----------------------configuracion microprocesador---------------------------

void main(void) {
    ANSEL = 0x00; // NO HAY ANALOGICOS
    ANSELH = 0x00;
    
    TRISA = 0x00;
    TRISB = 0xff;
    TRISC = 0x00;
    TRISD = 0x00; // PORTD todo salida
    TRISE = 0x00; // PORTE todo salida
    TRISE = 0b0011; // primeros 2 pines como entrada analogicas
    
    ADCON1bits.ADFM = 0;    //Justificado a la izquierda
    ADCON1bits.VCFG0 = 0;   //voltaje de 0V-5V
    ADCON1bits.VCFG1 = 0;
    ADCON0bits.ADCS0 = 1;   //Fosc/8
    ADCON0bits.ADCS1 = 0;
    ADCON0bits.CHS = 5; //canal 5
    __delay_us(100);
    ADCON0bits.ADON = 1;    //activo el modulo
    
    OPTION_REGbits.nRBPU = 0; //config. de PULL-UP
    WPUBbits.WPUB0 = 1;
    WPUBbits.WPUB1 = 1;
    WPUBbits.WPUB2 = 1;
    
    
    OSCCONbits.IRCF = 0b110; //Config. de oscilacion 4MHz
    OSCCONbits.SCS = 1;      //reloj interno

                             //Config. timmer0
    OPTION_REGbits.T0CS = 0;    //Uso reloj interno
    OPTION_REGbits.PSA = 0;     //Uso pre-escaler
    OPTION_REGbits.PS = 0b111;  //PS = 111 / 1:256
    TMR0 = 78;                  //Reinicio del timmer

    ADCON0bits.GO = 1;  //para el ADC
    
    INTCONbits.GIE = 1; //habilito interrupciones
    INTCONbits.T0IE = 0; //desactivo interrupciones por timmer 0
    INTCONbits.T0IF = 0; //bajo la bandera
    
    PORTB = 0; // Estado inicial de los pines
    PORTD = 0; // Estado inicial de los pines
    PORTE = 0; // Estado inicial de los pines

    //------------------------------loop principal----------------------------------
    while (1) {
       if (T0IF == 1) { //INTERRUPCION POR TIMMER0
            if(!RB0){
                RD0 = 1;
                __delay_ms(2);
                RD0 = 0;
                TMR0 = 70;  //POSICION 0°
                INTCONbits.T0IF = 0; //bajo la bandera
            }
            else if(!RB1){
                RD1 = 1;
                __delay_ms(2);
                RD1 = 0;
                TMR0 = 70;  //POSICION 0°
                INTCONbits.T0IF = 0; //bajo la bandera                
            }
            else if(!RB2){
                RD2 = 1;
                __delay_ms(2);
                RD2 = 0;
                TMR0 = 70;  //POSICION 0°
                INTCONbits.T0IF = 0; //bajo la bandera                
            }
            else{
                RD0 = 1;
                RD1 = 1;
                RD2 = 1;
                __delay_ms(1);
                RD1 = 0;
                RD2 = 0;
                RD0 = 0;
                TMR0 = 74;  //POSICION 180°
                INTCONbits.T0IF = 0; //bajo la bandera
            }            
        }
       
       if(ADCON0bits.GO == 0){
            
            if(ADCON0bits.CHS == 6){
                PORTA = ADRESH;
                ADCON0bits.CHS = 5;
            }
            else if(ADCON0bits.CHS == 5){
                PORTC = ADRESH;
                ADCON0bits.CHS = 6;
            }
            __delay_us(50);     //con 2 micros segundos será suficiente se dejo
                                //en 50 por fallos de software en proteus
            ADCON0bits.GO = 1;
        }
       
    }    
    return;
}
/*
 * Temporizacion = 20ms = 4/Fosc * Prescaler * x 
 * 20ms = 4/4MHz * 256 * x
 * x = 78
 * 
 * 18ms: 0°
 * 18ms = 4/Fosc * Prescaler * x
 * x = 70
 * 
 * 19ms: 180°
 * 19ms = 4/Fosc * Prescaler * x
 * x = 74
 * 
 * 18,5ms: 90°
 * 18,5ms = 4/Fosc * Prescaler * x
 * x = 72
 */

