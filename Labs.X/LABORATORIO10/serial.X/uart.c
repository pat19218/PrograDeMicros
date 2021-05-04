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
#pragma config FCMEN = ON      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
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

//--------------------------funciones-------------------------------------------
void USART_Tx(char data);
char USART_Rx();
void USART_Cadena(char *str);

//---------------------------variables------------------------------------------

char valor;
char loop;
//---------------------------interrupciones-------------------------------------

void main(void){

    ANSEL = 0X00;   //NO HAY PINES ANALOGOS
    ANSELH = 0x00;

    TRISA = 0x00; // PORTA todo salida
    TRISB = 0x00; // PORTB todo salida
    
    PORTA = 0x00;
    PORTB = 0x00;
    
    OSCCONbits.IRCF = 0b111 ;  // config. de oscilador interno
    OSCCONbits.SCS = 1;         //reloj interno

                            //Confi. serial comunication
    TXSTAbits.SYNC = 0;     //asincrono
    TXSTAbits.BRGH = 1;     //high speed
    BAUDCTLbits.BRG16 = 1;  //uso los 16 bits
   
    SPBRG = 207;   //revisar tabla BAUD RATES FOR ASYNCHRONOUS MODES (CONTINUED)                      
    SPBRGH = 0;    //pagina 168 del datasheet del 2009         
    
    RCSTAbits.SPEN = 1;     //enciendo el modulo
    RCSTAbits.RX9 = 0;      //No trabajo a 9 bits
    RCSTAbits.CREN = 1;     //activo recepción
    TXSTAbits.TXEN = 1;     //activo transmision 
    


    //------------------------------loop principal----------------------------------
    while (1) {
        USART_Cadena(" Que accion desea ejecutar? ");
        TXREG = '\n';
        USART_Cadena(" 1) Desplegar cadena de caracteres ");
        TXREG = '\n';
        USART_Cadena(" 2) Cambiar PORTA ");
        TXREG = '\n';
        USART_Cadena(" 3) Cambiar PORTB ");
        TXREG = '\n';
        
        while(PIR1bits.RCIF == 0);
        
        if(PIR1bits.RCIF){//chequeo si recibo datos
            
            valor = USART_Rx();
            
            switch(valor){
                case ('1'):
                    USART_Cadena(" Hello fck wrld ");
                    break;
                        
                case ('2'):
                    loop = 1;
                    USART_Cadena(" Ingresa un caracter para el puerto A: ");
                    while(loop){
                        if(PIR1bits.RCIF){
                            PORTA = USART_Rx();  //lo paso al puerto A
                            loop = 0;
                        }
                    }
                    USART_Cadena(" Listo ");
                    break;
                        
                case ('3'):
                    loop = 1;
                    USART_Cadena(" Ingresa un caracter para el puerto B: ");
                    while(loop){
                        if(PIR1bits.RCIF){
                            PORTB = USART_Rx();  //lo paso al puerto A
                            loop = 0;
                        }
                    }                       
                    USART_Cadena(" Listo ");
                    break;
            }
        }
        __delay_ms(10);
    }
    
    return;     //end
}


    void USART_Tx(char data){
        while(TXSTAbits.TRMT == 0);
        TXREG = data;
    }

    char USART_Rx(){
        return RCREG; 
       }

    void USART_Cadena(char *str){
        while(*str != '\0'){
            USART_Tx(*str);
            str++;
        }
    }