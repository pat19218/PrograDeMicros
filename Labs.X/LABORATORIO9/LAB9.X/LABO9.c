/*
 * File:   lab9.c
 * Dispositivo:	PIC16f887
 * Author: isaac
 * Compilador:	pic-as (v2.30), MPLABX V5.45
 * 
 * Programa: Controlar 2 servo con potenciometro
 * Hardware: Pic 16f887, potenciometro, servomotor
 * 
 * Created on 26 de abril de 2021, 13:00
 * Last modification on 26 de abril de 2021, 19:19
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
char    turno;
                          // 0     1    2     3     4
const char num_display[] = {0xFC, 0x60, 0xDA, 0xF2, 0x66,
                            0xB6, 0xBE, 0xE0, 0xFE, 0xF6};
                          //  5     6     7    8     9


//---------------------------interrupciones-------------------------------------

void __interrupt()isr(void){
    
    if(PIR1bits.ADIF == 1){
        
        if(turno == 1){
            CCPR1L = (ADRESH >> 1) + 124;
        }
        else if(turno == 0){
            CCPR2L = (ADRESH >> 1) + 124;
        }
                
        PIR1bits.ADIF = 0;
        
    }

}


void main(void){

    ANSEL = 0b00000011; // hay i/o analogicas AN0 y AN1
    ANSELH = 0x00;

    TRISA = 0b00000011; // PORTA solo 2 pines como entradas
    TRISC = 0x00; // PORTC todo salida
    TRISD = 0x00; // PORTC todo salida
    
                            //Confi. ADC
    ADCON1bits.ADFM = 0;    //Justificado a la izquierda
    ADCON1bits.VCFG0 = 0;   //voltaje de 0V-5V
    ADCON1bits.VCFG1 = 0;
    ADCON0bits.ADCS = 0b10; //Fosc/32
    ADCON0bits.CHS = 0;     //canal 0
    __delay_us(50);
    ADCON0bits.ADON = 1;    //activo el modulo
    
                            //Config. PWM
    TRISCbits.TRISC2 = 1;   //RC2/CCP1 como entrada
    TRISDbits.TRISD5 = 1;   //RC2/CCP1 como entrada
    
    PR2 = 255;              //Valor de pwm
    
    CCP1CONbits.P1M = 0;    //PWM mode
    CCP1CONbits.CCP1M = 0b1100; 
    CCPR1L = 0x0f;          //inicio de ciclo de trabajo
    //
    CCP2CONbits.CCP2M = 0;    //PWM mode
    CCP2CONbits.CCP2M = 0b1100; 
    CCPR1L = 0x0f;          //inicio de ciclo de trabajo
    CCPR2L = 0x0f;          //inicio de ciclo de trabajo
    
    CCP1CONbits.DC1B = 0;
    CCP2CONbits.DC2B0 = 0;
    CCP2CONbits.DC2B1 = 0;
    //
    PIR1bits.TMR2IF = 0;     //bajo la bandera
    T2CONbits.T2CKPS = 0b11; //pre-escaler 1:16
    T2CONbits.TMR2ON = 1;    //Enciendo el timmer 2
    while(PIR1bits.TMR2IF == 0);    //espero a completar el un ciclo tmr2
    PIR1bits.TMR2IF = 0;
    TRISCbits.TRISC2 = 0;           // Salida PWM
    TRISDbits.TRISD5 = 0;           // Salida PWM
    
    OSCCONbits.IRCF = 0b0111 ;  // config. de oscilador
    OSCCONbits.SCS = 1;         //reloj interno
    
    
                            //confi. interrupciones
    PIR1bits.ADIF = 0;
    PIE1bits.ADIE = 1; 
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    ADCON0bits.GO = 1;  

    turno = 1;

    //------------------------------loop principal----------------------------------
    while (1) {
        
        if(ADCON0bits.GO == 0){
            
            if(ADCON0bits.CHS == 0){
                turno = 0;
                ADCON0bits.CHS = 1;
            }
            else if(ADCON0bits.CHS == 1){
                turno = 1;
                ADCON0bits.CHS = 0;
            }
            __delay_us(50);     //con 6 micros segundos será suficiente se dejo
                                //en 50 por fallos de software en proteus
            ADCON0bits.GO = 1;
        }
    }
    return;     //end
}
