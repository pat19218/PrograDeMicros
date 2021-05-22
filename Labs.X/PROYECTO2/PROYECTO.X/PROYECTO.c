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
 * Last modification on 21 de mayo de 2021, 23:00
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
char hora, minuto, escala;
char valor;
char ventana;

//--------------------------funciones-------------------------------------------
void USART_Tx(char data);
char USART_Rx();
void USART_Cadena(char *str);

//---------------------------interrupciones-------------------------------------

void __interrupt()isr(void) {

    

}

//----------------------configuracion microprocesador---------------------------

void main(void) {
    ANSEL = 0x00;       // NO HAY ANALOGICOS
    ANSELH = 0x00;
    
    TRISA = 0x00;
    TRISB = 0xff;
    TRISD = 0x00;       // PORTD todo salida
    TRISE = 0b0011;     // primeros 2 pines como entrada analogicas
    
                            //Confi. ADC
    ADCON1bits.ADFM = 0;    //Justificado a la izquierda
    ADCON1bits.VCFG0 = 0;   //voltaje de 0V-5V
    ADCON1bits.VCFG1 = 0;
    ADCON0bits.ADCS0 = 1;   //Fosc/8
    ADCON0bits.ADCS1 = 0;
    ADCON0bits.CHS = 5;     //canal 5
    __delay_us(100);
    ADCON0bits.ADON = 1;    //activo el modulo
    
    OPTION_REGbits.nRBPU = 0; //config. de PULL-UP
    WPUBbits.WPUB0 = 1;
    WPUBbits.WPUB1 = 1;
    WPUBbits.WPUB2 = 1;
    
    
    OSCCONbits.IRCF = 0b110; //Config. de oscilacion 4MHz
    OSCCONbits.SCS = 1;      //reloj interno

                                //Config. timmer0 para PWM
    OPTION_REGbits.T0CS = 0;    //Uso reloj interno
    OPTION_REGbits.PSA = 0;     //Uso pre-escaler
    OPTION_REGbits.PS = 0b111;  //PS = 111 / 1:256
    TMR0 = 78;                  //Reinicio del timmer

                            //Confi. serial comunication
    TXSTAbits.SYNC = 0;     //asincrono
    TXSTAbits.BRGH = 1;     //high speed
    BAUDCTLbits.BRG16 = 1;  //uso los 16 bits
   
    SPBRG = 103;   //revisar tabla BAUD RATES FOR ASYNCHRONOUS MODES (CONTINUED)                      
    SPBRGH = 0;    //pagina 168 del datasheet del 2009         
    
    RCSTAbits.SPEN = 1;     //enciendo el modulo
    RCSTAbits.RX9 = 0;      //No trabajo a 9 bits
    RCSTAbits.CREN = 1;     //activo recepción
    TXSTAbits.TXEN = 1;     //activo transmision 

    INTCONbits.GIE = 1;     //habilito interrupciones
    INTCONbits.T0IE = 0;    //desactivo interrupciones por timmer 0
    INTCONbits.T0IF = 0;    //bajo la bandera
//    PIE1bits.RCIE = 1;    //interrupcion por serialcomunication recepcion
//    PIR1bits.RCIF = 0;    //bajo la bandera
    
    ADCON0bits.GO = 1;  //para el ADC
    
    PORTA = 0; // Estado inicial de los pines
    PORTB = 0; // Estado inicial de los pines
    PORTC = 0; // Estado inicial de los pines
    PORTD = 0; // Estado inicial de los pines
    PORTE = 0; // Estado inicial de los pines
    
    ventana = 1;// Estado inicial

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
                escala = ADRESH;
                if (escala >= 0 && escala <=42 ){
                    minuto = 0;
                }else if(escala >= 43 && escala <= 84 ){
                    minuto = 15;
                }else if(escala >= 85 && escala <= 126 ){
                    minuto = 30;
                }else if(escala >= 127 && escala <= 168 ){
                    minuto = 45;
                }else if(escala >= 210 && escala <= 255 ){
                    minuto = 60;
                }
       
                ADCON0bits.CHS = 5;
            }
            else if(ADCON0bits.CHS == 5){
                escala = ADRESH;
                if (escala >= 0 && escala <=26 ){
                    hora = 0;
                }else if(escala >= 27 && escala <=52 ){
                    hora = 1;
                }else if(escala >= 52 && escala <=78 ){
                    hora = 2;
                }else if(escala >= 79 && escala <=104 ){
                    hora = 3;
                }else if(escala >= 105 && escala <=130 ){
                    hora = 4;
                }else if(escala >= 131 && escala <=156 ){
                    hora = 5;
                }else if(escala >= 157 && escala <=182 ){
                    hora = 6;
                }else if(escala >= 183 && escala <=208 ){
                    hora = 7;
                }else if(escala >= 209 && escala <=234 ){
                    hora = 8;
                }else if(escala >= 235 && escala <=255 ){
                    hora = 9;
                }
                
                ADCON0bits.CHS = 6;
            }
            __delay_us(50);     //con 2 micros segundos será suficiente se dejo
                                //en 50 por fallos de software en proteus
            ADCON0bits.GO = 1;
        }
       
       PORTA = hora;
       
       if(ventana == 1){
           
           USART_Cadena("\r Que accion desea ejecutar? \r");
           USART_Cadena("1) ver datos de hora \r");
           USART_Cadena("2) ver datos de minuto \r");
           USART_Cadena("3) ver datos de ambos \r\r");
           ventana = 0;
        }
       
       if (PIR1bits.RCIF == 1){
           valor = USART_Rx();
           ventana = 1;
       }
              
       switch(valor){
            case ('1'):
                USART_Cadena(" Esperar  ");
                while(TXSTAbits.TRMT == 0);
                if (hora == 0){
                    TXREG = 48;                    
                }else if(hora == 1){
                    TXREG = 49;                    
                }else if(hora == 2){
                    TXREG = 50;                    
                }else if(hora == 3){
                    TXREG = 51;
                }else if(hora == 4){
                    TXREG = 52;
                }else if(hora == 5){
                    TXREG = 53;
                }else if(hora == 6){
                    TXREG = 54;
                }else if(hora == 7){
                    TXREG = 55;
                }else if(hora == 8){
                    TXREG = 56;
                }else if(hora == 9){
                    TXREG = 57;
                }
                
                USART_Cadena(" Horas ");
                
                USART_Cadena(" con  ");
                while(TXSTAbits.TRMT == 0);
                if(minuto == 0) {
                    USART_Cadena("0");
                }else if (minuto == 15){
                    USART_Cadena("15");
                }else if (minuto == 30){
                    USART_Cadena("30");
                }else if (minuto == 45){
                    USART_Cadena("45");
                }else if (minuto == 60){
                    USART_Cadena("60");
                }
                USART_Cadena(" mins. para el medicamento  \r\r");
                ventana = 1;
                break;
                        
            case ('2'):
                ventana = 1;
                break;
                        
            case ('3'):
                ventana = 1;
                break;
        }
       valor = 0;
       
    }    
    return;
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

