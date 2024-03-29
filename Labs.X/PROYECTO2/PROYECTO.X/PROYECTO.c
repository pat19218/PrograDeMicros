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
 * Last modification on 01 de junio de 2021, 23:00
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
#pragma config LVP = OFF         // Low Voltage Programming Enable bit (RB3/PGM pin has PGM function, low voltage programming enabled)

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
#define medicina1 0x10     //espacios de la eeprom a usar
#define medicina2 0x11
#define medicina3 0x12

//---------------------------variables------------------------------------------
char hora, minuto, escala;
char valor;
char ventana;
char min_user1, min_user2, min_user3;
char hora_user1, hora_user2, hora_user3;
char hora_nueva;
char me_1_us1_h, me_2_us1_h, me_3_us1_h;
char me_1_us2_h, me_2_us2_h, me_3_us2_h;
char me_1_us3_h, me_2_us3_h, me_3_us3_h;
char me_1_us1_m, me_2_us1_m, me_3_us1_m;
char me_1_us2_m, me_2_us2_m, me_3_us2_m;
char me_1_us3_m, me_2_us3_m, me_3_us3_m;
char contm, conth;
char mins, horas;
char activa1, activa2, activa3;
char me_1_us1_th, me_2_us1_th, me_3_us1_th;
char me_1_us2_th, me_2_us2_th, me_3_us2_th;
char me_1_us3_th, me_2_us3_th, me_3_us3_th;
char me_1_us1_tm, me_2_us1_tm, me_3_us1_tm;
char me_1_us2_tm, me_2_us2_tm, me_3_us2_tm;
char me_1_us3_tm, me_2_us3_tm, me_3_us3_tm;
char prueba;
char RB0_old, RB1_old, RB2_old, RB5_old, RB6_old, RB7_old;
char medicamento1, medicamento2, medicamento3;


//--------------------------funciones-------------------------------------------
void USART_Tx(char data);
char USART_Rx();
void USART_Cadena(char *str);
void SendCharH(char in);
void SendCharM(char in);
char CharToNumH(char in, char hora_user);
char CharToNumM(char in, char her);
void WriteToEEPROM(uint8_t dato, uint8_t addr);
uint8_t ReadToEEPROM(uint8_t addr);

//---------------------------interrupciones-------------------------------------

void __interrupt()isr(void) {
    if (TMR1IF == 1){     //timmer1
        contm++;
        conth++;
        if(contm == 2){
            //PORTA = ~PORTA;      // Toggle PORTB bit1 LED
            me_1_us1_tm++;         //mido minutos de cada medicamento
            me_1_us2_tm++; 
            me_1_us3_tm++; 
            me_2_us1_tm++; 
            me_2_us2_tm++; 
            me_2_us3_tm++; 
            me_3_us1_tm++; 
            me_3_us2_tm++; 
            me_3_us3_tm++;
            contm = 0;
            activa1 = 0;
            activa2 = 0;
            activa3 = 0;
            prueba = 1;
        }
        if(conth == 120){
            me_1_us1_th++;          //mido horas de cada medicamento
            me_1_us2_th++; 
            me_1_us3_th++; 
            me_2_us1_th++; 
            me_2_us2_th++; 
            me_2_us3_th++; 
            me_3_us1_th++; 
            me_3_us2_th++; 
            me_3_us3_th++;
            conth = 0;
            //PORTA = ~PORTA;
        }
                    
        TMR1IF = 0;           // interrupt must be cleared by software
        TMR1IE =1;        // reenable the interrupt
        TMR1H = 0;             // preset for timer1 MSB register
        TMR1L = 0;             // preset for timer1 LSB register
    }
}

//----------------------configuracion microprocesador---------------------------

void main(void) {
    ANSEL = 0b01100000;       // 2 pines ANALOGICOS
    ANSELH = 0x00;
    
    TRISA = 0b10000000;
    TRISB = 0xff;       // PORTB todo entrada
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
    WPUBbits.WPUB5 = 1;
    WPUBbits.WPUB6 = 1;
    WPUBbits.WPUB7 = 1;
    
    OSCCONbits.IRCF = 0b110; //Config. de oscilacion 4MHz
    OSCCONbits.SCS = 1;      //reloj interno

                                //Config. timmer0 para PWM
    OPTION_REGbits.T0CS = 0;    //Uso reloj interno
    OPTION_REGbits.PSA = 0;     //Uso pre-escaler
    OPTION_REGbits.PS = 0b111;  //PS = 111 / 1:256
    TMR0 = 78;                  //Reinicio del timmer
    
                                //config. timmer1 para contar tiempor
    T1CONbits.T1CKPS1 = 1;   // bits 5-4  Prescaler Rate Select bits
    T1CONbits.T1CKPS0 = 1;   // bit 4
    T1CONbits.T1OSCEN = 1;   // bit 3 Timer1 Oscillator Enable Control bit 1 = on
    T1CONbits.T1SYNC = 1;    // bit 2 Timer1 External Clock Input Synchronization Control bit...1 = Do not synchronize external clock input
    T1CONbits.TMR1CS = 0;    // bit 1 Timer1 Clock Source Select bit...0 = Internal clock (FOSC/4)
    T1CONbits.TMR1ON = 1;    // bit 0 enables timer
    TMR1H = 0;             // preset for timer1 MSB register
    TMR1L = 0;             // preset for timer1 LSB register
    

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
    PIR1bits.TMR1IF = 0;    // bandera timmer1
    PIE1bits.TMR1IE  = 1;   // enable Timer1 interrupts
    INTCONbits.PEIE = 1;    // 

//    PIE1bits.RCIE = 1;    //interrupcion por serialcomunication recepcion
//    PIR1bits.RCIF = 0;    //bajo la bandera
    
    ADCON0bits.GO = 1;  //para el ADC
    
    medicamento1 = ReadToEEPROM(medicina1); // Si inicialmente muestra el valor
    /*if(medicamento1 == 16){                 //de su ubicacion la inicion en 0
        medicamento1 = 0;
    }*/
    medicamento2 = ReadToEEPROM(medicina2);
    /*if(medicamento2 == 17){
        medicamento2 = 0;
    }*/
    medicamento3 = ReadToEEPROM(medicina3);
    /*if(medicamento3 == 18){
        medicamento3 = 0;
    }*/
    
    PORTA = 0; // Estado inicial de los pines
    PORTB = 0; // Estado inicial de los pines
    PORTC = 0; // Estado inicial de los pines
    PORTD = 0; // Estado inicial de los pines
    PORTE = 0; // Estado inicial de los pines
    
    activa1 = 0;        // variables que activaran los servos
    activa2 = 0;
    activa3 = 0;
    
    RB0_old = 1;
    RB1_old = 1;
    RB2_old = 1;
    RB5_old = 1;
    RB6_old = 1;
    RB7_old = 1;
    
    me_1_us1_h = 24; 
    me_2_us1_h = 24;
    me_3_us1_h = 24;
    me_1_us2_h = 24;
    me_2_us2_h = 24;
    me_3_us2_h = 24;
    me_1_us3_h = 24;
    me_2_us3_h = 24;
    me_3_us3_h = 24;
    me_1_us1_m = 55;
    me_2_us1_m = 55;
    me_3_us1_m = 55;
    me_1_us2_m = 55; 
    me_2_us2_m = 55;
    me_3_us2_m = 55;
    me_1_us3_m = 55; 
    me_2_us3_m = 55;
    me_3_us3_m = 55;
    
    me_1_us1_tm = 0;         //inicio minutos de cada medicamento
    me_1_us2_tm = 0; 
    me_1_us3_tm = 0; 
    me_2_us1_tm = 0; 
    me_2_us2_tm = 0; 
    me_2_us3_tm = 0; 
    me_3_us1_tm = 0; 
    me_3_us2_tm = 0; 
    me_3_us3_tm = 0;
    
    me_1_us1_th = 0;          //inicio horas de cada medicamento
    me_1_us2_th = 0; 
    me_1_us3_th = 0; 
    me_2_us1_th = 0; 
    me_2_us2_th = 0; 
    me_2_us3_th = 0; 
    me_3_us1_th = 0; 
    me_3_us2_th = 0; 
    me_3_us3_th = 0;

    ventana = 1;// Estado inicial
    
//------------------------------loop principal----------------------------------
    while (1) {
        if(prueba){
            if (me_1_us1_th == me_1_us1_h  || me_1_us1_h == 0){      //chequeo si algun usuario del 
                if(me_1_us1_tm == me_1_us1_m){  //medicamento 1 tiene que dispensar
                    if(activa1 == 0){
                        activa1 = 1;
                        medicamento1++;
                        WriteToEEPROM(medicamento1, medicina1);
                    }
                    me_1_us1_th = 0;
                    me_1_us1_tm = 0;
                }
            }
            if (me_1_us2_th == me_1_us2_h || me_1_us2_h  == 0){
                if(me_1_us2_tm == me_1_us2_m){
                    if(activa1 == 0){
                        activa1 = 1;
                        medicamento1++;
                        WriteToEEPROM(medicamento1, medicina1);
                    }
                    me_1_us2_th = 0;
                    me_1_us2_tm = 0;
                }
            }
            if (me_1_us3_th == me_1_us3_h || me_1_us3_h == 0){
                if(me_1_us3_tm == me_1_us3_m){
                    if(activa1 == 0){
                        activa1 = 1;
                        medicamento1++;
                        WriteToEEPROM(medicamento1, medicina1);
                    }
                    me_1_us3_th = 0;
                    me_1_us3_tm = 0;
                }
            }
            if (me_2_us1_th == me_2_us1_h || me_2_us1_h == 0){//chequeo si algun usuario del 
                if(me_2_us1_tm == me_2_us1_m){  //medicamento 2 tiene que dispensar
                    if(activa2 == 0){
                        activa2 = 1;
                        medicamento2++;
                        WriteToEEPROM(medicamento2, medicina2);
                    }
                    me_2_us1_th = 0;
                    me_2_us1_tm = 0;
                }
            }
            if (me_2_us2_th == me_2_us2_h || me_2_us2_h == 0){
                if(me_2_us2_tm == me_2_us2_m){
                    if(activa2 == 0){
                        activa2 = 1;
                        medicamento2++;
                        WriteToEEPROM(medicamento2, medicina2);
                    }
                    me_2_us2_th = 0;
                    me_2_us2_tm = 0;
                }
            }
            if (me_2_us3_th == me_2_us3_h || me_2_us3_h == 0){
                if(me_2_us3_tm == me_2_us3_m){
                    if(activa2 == 0){
                        activa2 = 1;
                        medicamento2++;
                       WriteToEEPROM(medicamento2, medicina2);
                    }
                    me_2_us3_th = 0;
                    me_2_us3_tm = 0;
                }
            }
            if (me_3_us1_th == me_3_us1_h || me_3_us1_h == 0){      //chequeo si algun usuario del 
                if(me_3_us1_tm == me_3_us1_m){  //medicamento 3 tiene que dispensar
                    if(activa3 == 0){
                        activa3 = 1;
                        medicamento3++;
                        WriteToEEPROM(medicamento3, medicina3);
                    }
                    me_3_us1_th = 0;
                    me_3_us1_tm = 0;
                }
            }
            if (me_3_us2_th == me_3_us2_h || me_3_us2_h == 0){
                if(me_3_us2_tm == me_3_us2_m){
                    if(activa3 == 0){
                        activa3 = 1;
                        medicamento3++;
                        WriteToEEPROM(medicamento3, medicina3);
                    }
                    me_3_us2_th = 0;
                    me_3_us2_tm = 0;
                }
            }
            if (me_3_us3_th == me_3_us3_h || me_3_us3_h == 0){
                if(me_3_us3_tm == me_3_us3_m){
                    if(activa3 == 0){
                        activa3 = 1;
                        medicamento3++;
                        WriteToEEPROM(medicamento3, medicina3);
                    }
                    me_3_us3_th = 0;
                    me_3_us3_tm = 0;
                }
            }
            prueba = 0;
        }
        if (T0IF == 1) { //INTERRUPCION POR TIMMER0 PWM
            if(RB0==0 || activa1==1){
                RD0 = 1;
                RA0 = ~RA0;
                __delay_ms(2);
                RD0 = 0;
                TMR0 = 70;  //POSICION 0°
                INTCONbits.T0IF = 0; //bajo la bandera
                
            }else{
                RD0 = 1;
                __delay_ms(1);
                RD0 = 0;
                TMR0 = 74;  //POSICION 180°
                INTCONbits.T0IF = 0; //bajo la bandera
                
            }
            if(RB1==0 || activa2==1){
                RD1 = 1;
                __delay_ms(2);
                RD1 = 0;
                TMR0 = 70;  //POSICION 0°
                INTCONbits.T0IF = 0; //bajo la bandera                
            }else{
                RD1 = 1;
                __delay_ms(1);
                RD1 = 0;
                TMR0 = 74;  //POSICION 180°
                INTCONbits.T0IF = 0; //bajo la bandera
            }
            if(RB2==0 || activa3==1){
                RD2 = 1;
                __delay_ms(2);
                RD2 = 0;
                TMR0 = 70;  //POSICION 0°
                INTCONbits.T0IF = 0; //bajo la bandera                
            }else{
                RD2 = 1;
                __delay_ms(1);
                RD2 = 0;
                TMR0 = 74;  //POSICION 180°
                INTCONbits.T0IF = 0; //bajo la bandera
            }
            T0IF = 0;
        }
        if (RB0 == 1 && RB0_old == 0) { // Escribo en la EEPROM si dispenso de
            medicamento1++;             // forma manual
            WriteToEEPROM(medicamento1, medicina1);
        }RB0_old = RB0;
        if (RB1 == 1 && RB1_old == 0){
            medicamento2++;
            WriteToEEPROM(medicamento2, medicina2);
        }RB1_old = RB1;
        if (RB2 == 1 && RB2_old == 0){
            medicamento3++;
            WriteToEEPROM(medicamento3, medicina3);
        }RB2_old = RB2;
       if(!RA7){            //reset todos los tiempos
           me_1_us1_tm = 0; 
           me_1_us2_tm = 0; 
           me_1_us3_tm = 0; 
           me_2_us1_tm = 0; 
           me_2_us2_tm = 0; 
           me_2_us3_tm = 0; 
           me_3_us1_tm = 0; 
           me_3_us2_tm = 0; 
           me_3_us3_tm = 0; 
           me_1_us1_th = 0; 
           me_1_us2_th = 0; 
           me_1_us3_th = 0; 
           me_2_us1_th = 0; 
           me_2_us2_th = 0; 
           me_2_us3_th = 0; 
           me_3_us1_th = 0; 
           me_3_us2_th = 0; 
           me_3_us3_th = 0; 
           //PORTA = 0;
       }
       if(ADCON0bits.GO == 0){
            
            if(ADCON0bits.CHS == 6){            //leo el canal para escalar mins
                escala = ADRESH;
                if ((escala >= 0) && (escala <=42)){
                    minuto = 0;
                }else if((escala >= 43 )&& (escala <= 84 )){
                    minuto = 15;
                }else if((escala >= 85) && (escala <= 126 )){
                    minuto = 30;
                }else if((escala >= 127) && (escala <= 168) ){
                    minuto = 45;
                }else if(escala >= 210 && escala <= 255 ){
                    minuto = 55;
                }
       
                ADCON0bits.CHS = 5;
            }
            else if(ADCON0bits.CHS == 5){      //leo el canal para escalar horas
                escala = ADRESH;
                if (escala >= 0 && escala <=10 ){
                    hora = 0;
                }else if(escala >= 11 && escala <=20 ){
                    hora = 1;
                }else if(escala >= 21 && escala <=30 ){
                    hora = 2;
                }else if(escala >= 31 && escala <=40 ){
                    hora = 3;
                }else if(escala >= 41 && escala <=50 ){
                    hora = 4;
                }else if(escala >= 51 && escala <=60 ){
                    hora = 5;
                }else if(escala >= 61 && escala <=70 ){
                    hora = 6;
                }else if(escala >= 71 && escala <=80 ){
                    hora = 7;
                }else if(escala >= 81 && escala <=90 ){
                    hora = 8;
                }else if(escala >= 91 && escala <=100 ){
                    hora = 9;
                }else if(escala >= 101&& escala <=110 ){
                    hora = 10;
                }else if(escala >= 111 && escala <=120 ){
                    hora = 11;
                }else if(escala >= 121 && escala <=130 ){
                    hora = 12;
                }else if(escala >= 131 && escala <=140 ){
                    hora = 13;
                }else if(escala >= 141 && escala <=150 ){
                    hora = 14;
                }else if(escala >= 151 && escala <=160 ){
                    hora = 15;
                }else if(escala >= 161 && escala <=170 ){
                    hora = 16;
                }else if(escala >= 171 && escala <=180 ){
                    hora = 17;
                }else if(escala >= 181 && escala <=190 ){
                    hora = 18;
                }else if(escala >= 191 && escala <=200 ){
                    hora = 19;
                }else if(escala >= 201&& escala <=210 ){
                    hora = 20;
                }else if(escala >= 211 && escala <=220 ){
                    hora = 21;
                }else if(escala >= 221 && escala <=230 ){
                    hora = 22;
                }else if(escala >= 231 && escala <=240 ){
                    hora = 23;
                }else if(escala >= 241 && escala <=255 ){
                    hora = 24;
                }
                ADCON0bits.CHS = 6;
            }
            __delay_us(50);     //con 2 micros segundos será suficiente se dejo
                                //en 50 por fallos de software en proteus
            ADCON0bits.GO = 1;
        }
              
       if(ventana == 1){        //muestro menú solo una vez
           
           USART_Cadena("\r Que accion desea ejecutar? \r \n");
           USART_Cadena("1) ver datos de usuario uno \r \n");
           USART_Cadena("2) ver datos de usuario dos\r \n");
           USART_Cadena("3) ver datos de usuario tres \r \n");
           USART_Cadena("4) Modificar horario usuario uno \r \n");
           USART_Cadena("5) Modificar horario usuario dos\r \n");
           USART_Cadena("6) Modificar horario usuario tres \r \n");
           USART_Cadena("7) Mostrar cantidad de medicamento dispensado \r\r\n\n");
           ventana = 0;
        }
       
       if (PIR1bits.RCIF == 1){ //compruebo si se introdujo un dato
           valor = USART_Rx();
           ventana = 1;
       }
              
       switch(valor){       //verifico que caracter se introdujo
            case ('1'):
                USART_Cadena(" Esperar  ");
                SendCharH(me_1_us1_h);      //convierto el número en caracter
                USART_Cadena(" Horas ");    //Entrego los minutos
                USART_Cadena(" con  ");
                SendCharM(me_1_us1_m);      //datos medicamento 1
                USART_Cadena(" mins. para el medicamento 1 \r\n");
                
                USART_Cadena(" Esperar  ");
                SendCharH(me_2_us1_h);
                USART_Cadena(" Horas ");    //Entrego los minutos
                USART_Cadena(" con  ");
                SendCharM(me_2_us1_m);
                USART_Cadena(" mins. para el medicamento 2 \r\n");
                
                USART_Cadena(" Esperar  ");
                SendCharH(me_3_us1_h);
                USART_Cadena(" Horas ");    //Entrego los minutos
                USART_Cadena(" con  ");
                SendCharM(me_3_us1_m);
                USART_Cadena(" mins. para el medicamento 3 \r\r\n\n");
                ventana = 1;
                break;
             
            case ('2'):
                USART_Cadena(" Esperar  ");
                SendCharH(me_1_us2_h);      //convierto el número en caracter
                USART_Cadena(" Horas ");    //Entrego los minutos
                USART_Cadena(" con  ");
                SendCharM(me_1_us2_m);      //datos medicamento 1
                USART_Cadena(" mins. para el medicamento 1 \r\n");
                
                USART_Cadena(" Esperar  ");
                SendCharH(me_2_us2_h);
                USART_Cadena(" Horas ");    //Entrego los minutos
                USART_Cadena(" con  ");
                SendCharM(me_2_us2_m);
                USART_Cadena(" mins. para el medicamento 2 \r\n");
                
                USART_Cadena(" Esperar  ");
                SendCharH(me_3_us2_h);
                USART_Cadena(" Horas ");    //Entrego los minutos
                USART_Cadena(" con  ");
                SendCharM(me_3_us2_m);
                USART_Cadena(" mins. para el medicamento 3 \r\r\n\n");
                ventana = 1;
                break;
           
            case ('3'):
                USART_Cadena(" Esperar  ");
                SendCharH(me_1_us3_h);      //convierto el número en caracter
                USART_Cadena(" Horas ");    //Entrego los minutos
                USART_Cadena(" con  ");
                SendCharM(me_1_us3_m);      //datos medicamento 1
                USART_Cadena(" mins. para el medicamento 1 \r\n");
                
                USART_Cadena(" Esperar  ");
                SendCharH(me_2_us3_h);
                USART_Cadena(" Horas ");    //Entrego los minutos
                USART_Cadena(" con  ");
                SendCharM(me_2_us3_m);
                USART_Cadena(" mins. para el medicamento 2 \r\n");
                
                USART_Cadena(" Esperar  ");
                SendCharH(me_3_us3_h);
                USART_Cadena(" Horas ");    //Entrego los minutos
                USART_Cadena(" con  ");
                SendCharM(me_3_us3_m);
                USART_Cadena(" mins. para el medicamento 3 \r\r\n\n");
                ventana = 1;
                break;
            
            case ('4'):
                USART_Cadena(" Ingresá la letra de la nueva hora  \n\r");
                USART_Cadena(" a.0  b.1   c.2   d.3   e.4   f.5   g.6   h.7  i.8\r\n");
                USART_Cadena(" j.9  k.10  l.11  m.12  n.13  o.14  p.15  q.16\r\n");
                USART_Cadena("r.17  s.18  t.19  u.20  v.21  w.22  x.23  y.24\r\n");
                while(1){
                    if (PIR1bits.RCIF == 1){ //compruebo si se introdujo un dato
                        USART_Cadena(" \r\n");
                        hora_nueva = USART_Rx();
                        USART_Cadena(" \r\n");
                        hora_user1 = CharToNumH(hora_nueva, hora_user1);
                        break;
                    }
                }
                USART_Cadena(" Ingresá la letra de los minutos nuevos  \r\n");
                USART_Cadena(" a.0  b.15   c.30   d.45   e.55   \r\n");
                while(1){
                    if (PIR1bits.RCIF == 1){ //compruebo si se introdujo un dato
                        USART_Cadena(" \r");
                        hora_nueva = USART_Rx();
                        USART_Cadena(" \r");
                        min_user1 = CharToNumM(hora_nueva, min_user1);
                        break;
                    }
                }
                USART_Cadena(" Actualmente se cuenta con 3 medicamentos\r\n");
                USART_Cadena(" presione el número de medicamento a guardar \r\n");
                USART_Cadena(" 1.AFC    2.FFT   3.DHL \r\n");
                while(1){
                    if (PIR1bits.RCIF == 1){ //compruebo si se introdujo un dato
                        hora_nueva = USART_Rx();
                        if(hora_nueva == '1'){
                            me_1_us1_h = hora_user1;
                            me_1_us1_m = min_user1;
                        }else if(hora_nueva == '2'){
                            me_2_us1_h = hora_user1;
                            me_2_us1_m = min_user1;
                        }else if(hora_nueva == '3'){
                            me_3_us1_h = hora_user1;
                            me_3_us1_m = min_user1;
                        }
                        break;
                    }
                }                             
                ventana = 1;
                break;
                        
            case ('5'):
                USART_Cadena(" Ingresá la letra de la nueva hora  \r\n");
                USART_Cadena(" a.0  b.1   c.2   d.3   e.4   f.5   g.6   h.7  i.8\r\n");
                USART_Cadena(" j.9  k.10  l.11  m.12  n.13  o.14  p.15  q.16\r\n");
                USART_Cadena("r.17  s.18  t.19  u.20  v.21  w.22  x.23  y.24\r\n");
                while(1){
                    if (PIR1bits.RCIF == 1){ //compruebo si se introdujo un dato
                        USART_Cadena(" \r\n");
                        hora_nueva = USART_Rx();
                        USART_Cadena(" \r\n");
                        hora_user2 = CharToNumH(hora_nueva, hora_user2);
                        break;
                    }
                }
                USART_Cadena(" Ingresá la letra de los minutos nuevos  \r\n");
                USART_Cadena(" a.0  b.15   c.30   d.45   e.55   \r\n");
                while(1){
                    if (PIR1bits.RCIF == 1){ //compruebo si se introdujo un dato
                        USART_Cadena(" \r\n");
                        hora_nueva = USART_Rx();
                        USART_Cadena(" \r\n");
                        min_user2 = CharToNumM(hora_nueva, min_user2);
                        break;
                    }
                }
                USART_Cadena(" Actualmente se cuenta con 3 medicamentos\r\n");
                USART_Cadena(" presione el número de medicamento a guardar \r\n");
                USART_Cadena(" 1.AFC    2.FFT   3.DHL \r\n");
                while(1){
                    if (PIR1bits.RCIF == 1){ //compruebo si se introdujo un dato
                        hora_nueva = USART_Rx();
                        if(hora_nueva == '1'){
                            me_1_us2_h = hora_user2;
                            me_1_us2_m = min_user2;
                        }else if(hora_nueva == '2'){
                            me_2_us2_h = hora_user2;
                            me_2_us2_m = min_user2;
                        }else if(hora_nueva == '3'){
                            me_3_us2_h = hora_user2;
                            me_3_us2_m = min_user2;
                        }
                        break;
                    }
                }                             
                ventana = 1;
                break;
            case ('6'):
                USART_Cadena(" Ingresá la letra de la nueva hora  \r\n");
                USART_Cadena(" a.0  b.1   c.2   d.3   e.4   f.5   g.6  h.7  i.8\r\n");
                USART_Cadena(" j.9  k.10  l.11  m.12  n.13  o.14  p.15  q.16\r\n");
                USART_Cadena("r.17  s.18  t.19  u.20  v.21  w.22  x.23  y.24\r\n");
                while(1){
                    if (PIR1bits.RCIF == 1){ //compruebo si se introdujo un dato
                        USART_Cadena(" \r\n");
                        hora_nueva = USART_Rx();
                        USART_Cadena(" \r\n");
                        hora_user3 = CharToNumH(hora_nueva, hora_user3);
                        break;
                    }
                }
                USART_Cadena(" Ingresá la letra de los minutos nuevos  \r\n");
                USART_Cadena(" a.0  b.15   c.30   d.45   e.55   \r\n");
                while(1){
                    if (PIR1bits.RCIF == 1){ //compruebo si se introdujo un dato
                        USART_Cadena(" \r\n");
                        hora_nueva = USART_Rx();
                        USART_Cadena(" \r\n");
                        min_user3 = CharToNumM(hora_nueva, min_user3);
                        break;
                    }
                }
                USART_Cadena(" Actualmente se cuenta con 3 medicamentos\r\n");
                USART_Cadena(" presione el número de medicamento a guardar \r\n");
                USART_Cadena(" 1.AFC    2.FFT   3.DHL \r\n");
                while(1){
                    if (PIR1bits.RCIF == 1){ //compruebo si se introdujo un dato
                        hora_nueva = USART_Rx();
                        if(hora_nueva == '1'){
                            me_1_us3_h = hora_user3;
                            me_1_us3_m = min_user3;
                        }else if(hora_nueva == '2'){
                            me_2_us3_h = hora_user3;
                            me_2_us3_m = min_user3;
                        }else if(hora_nueva == '3'){
                            me_3_us3_h = hora_user3;
                            me_3_us3_m = min_user3;
                        }
                        break;
                    }
                }                             
                ventana = 1;
                break;    
           case('7'):   //información de cantidad de medicamentos dispensados
               USART_Cadena("El medicamento 1 se ha dispensado ");
               SendCharH(medicamento1);
               USART_Cadena(" veces \r\n");
               USART_Cadena("El medicamento 2 se ha dispensado ");
               SendCharH(medicamento2);
               USART_Cadena(" veces \r\n");
               USART_Cadena("El medicamento 3 se ha dispensado ");
               SendCharH(medicamento3);
               USART_Cadena(" veces \r \r \n\n");
               break;
            }
        
       valor = '0';
       
       if (RB7==0 && RB3==1 && RB4==0){ //Guardo nuevo horario de forma manual
           me_3_us3_m = minuto;         //para medicamento uno
           me_3_us3_h = hora;
       }else if (RB7==0 && RB3==0 && RB4==1){
           me_2_us3_m = minuto;
           me_2_us3_h = hora;
       }else if (RB7==0 && RB3==0 && RB4==0){
           me_1_us3_m = minuto;
           me_1_us3_h = hora;
       }
       
       if (RB6==0 && RB3==1 && RB4==0){ //Guardo nuevo horario de forma manual
           me_3_us2_m = minuto;         //para medicamento dos
           me_3_us2_h = hora;
       }else if  (RB6==0 && RB3==0 && RB4==1){
           me_2_us2_m = minuto;
           me_2_us2_h = hora;
       }else if (RB6==0 && RB3==0 && RB4==0){
           me_1_us2_m = minuto;
           me_1_us2_h = hora;
       }
       
       if (RB5==0 && RB3==0 && RB4==1){ //Guardo nuevo horario de forma manual
           me_3_us1_m = minuto;         //para medicamento tres
           me_3_us1_h = hora;
       }else if  (RB5==0 && RB3==1 && RB4==0){
           me_2_us1_m = minuto;
           me_2_us1_h = hora;
       }else if (RB5==0 && RB3==0 && RB4==0){
           me_1_us1_m = minuto;
           me_1_us1_h = hora;
       }
    }    
    return;
}



void USART_Tx(char data){       //envio de un caracter
    while(TXSTAbits.TRMT == 0);
    TXREG = data;
}

char USART_Rx(){                //Lectura de comunicacion serial
    return RCREG; 
   }

void USART_Cadena(char *str){   //Envio de cadena de caracteres
    while(*str != '\0'){
        USART_Tx(*str);
        str++;
    }
}

void SendCharH(char in){    //Conversion de numero a caracter
    switch(in){       //acorde al caracter se entrega la hora
        case(0):
            while(TXSTAbits.TRMT == 0);
            TXREG = 48;
        break;
                    
        case(1):
            while(TXSTAbits.TRMT == 0);
            TXREG = 49;   
        break;
                    
        case(2):
            while(TXSTAbits.TRMT == 0);
            TXREG = 50;   
        break;

        case(3):
            while(TXSTAbits.TRMT == 0);
            TXREG = 51;   
        break;

        case(4):
            while(TXSTAbits.TRMT == 0);
            TXREG = 52;
        break;

        case(5):
            while(TXSTAbits.TRMT == 0);
            TXREG = 53;
        break;

        case(6):
            while(TXSTAbits.TRMT == 0);
            TXREG = 54;
        break;

        case(7):
            while(TXSTAbits.TRMT == 0);
            TXREG = 55;
        break;

        case(8):
            while(TXSTAbits.TRMT == 0);
            TXREG = 56;
        break;

        case(9):
            while(TXSTAbits.TRMT == 0);
            TXREG = 57;
        break;

        case(10):
            while(TXSTAbits.TRMT == 0);
            TXREG = 49;
            while(TXSTAbits.TRMT == 0);
            TXREG = 48;
        break;

        case(11):
            while(TXSTAbits.TRMT == 0);
            TXREG = 49;
            while(TXSTAbits.TRMT == 0);
            TXREG = 49;
        break;

        case(12):
            while(TXSTAbits.TRMT == 0);
            TXREG = 49;
            while(TXSTAbits.TRMT == 0);
            TXREG = 50;
        break;

        case(13):
            while(TXSTAbits.TRMT == 0);
            TXREG = 49;
            while(TXSTAbits.TRMT == 0);
            TXREG = 51;
        break;

        case(14):
            while(TXSTAbits.TRMT == 0);
            TXREG = 49;
            while(TXSTAbits.TRMT == 0);
            TXREG = 52;
        break;

        case(15):
            while(TXSTAbits.TRMT == 0);
            TXREG = 49;
            while(TXSTAbits.TRMT == 0);
            TXREG = 53;
        break;

        case(16):
            while(TXSTAbits.TRMT == 0);
            TXREG = 49;
            while(TXSTAbits.TRMT == 0);
            TXREG = 54;
        break;

        case(17):
            while(TXSTAbits.TRMT == 0);
            TXREG = 49;
            while(TXSTAbits.TRMT == 0);
            TXREG = 55;
        break;

        case(18):
            while(TXSTAbits.TRMT == 0);
            TXREG = 49;
            while(TXSTAbits.TRMT == 0);
            TXREG = 56;
        break;

        case(19):
            while(TXSTAbits.TRMT == 0);
            TXREG = 49;
            while(TXSTAbits.TRMT == 0);
            TXREG = 57;
        break;

        case(20):
            while(TXSTAbits.TRMT == 0);
            TXREG = 50;
            while(TXSTAbits.TRMT == 0);
            TXREG = 48;
        break;

        case(21):
            while(TXSTAbits.TRMT == 0);
            TXREG = 50;
            while(TXSTAbits.TRMT == 0);
            TXREG = 49;
        break;

        case(22):
            while(TXSTAbits.TRMT == 0);
            TXREG = 50;
            while(TXSTAbits.TRMT == 0);
            TXREG = 50;
        break;

        case(23):
            while(TXSTAbits.TRMT == 0);
            TXREG = 50;
            while(TXSTAbits.TRMT == 0);
            TXREG = 51;
        break;

        case(24):
            while(TXSTAbits.TRMT == 0);
            TXREG = 50;
            while(TXSTAbits.TRMT == 0);
            TXREG = 52;
        break;
    }
}

void SendCharM(char in){    //Conversion de numero a caracter
    while(TXSTAbits.TRMT == 0);
        if(in == 0) {
            USART_Cadena("0");
        }else if (in == 15){
            USART_Cadena("15");
        }else if (in == 30){
            USART_Cadena("30");
        }else if (in == 45){
            USART_Cadena("45");
        }else if (in == 55){
            USART_Cadena("55");
        }
}

char CharToNumH(char in, char hora_user){   //conversion de seleccion a numero
    switch(in){
        case('a'):
            hora_user = 0;
            break;
        case('b'):
            hora_user = 1;
            break;
        case('c'):
            hora_user = 2;
            break;
        case('d'):
            hora_user = 3;
            break;
        case('e'):
            hora_user = 4;
            break;
        case('f'):
            hora_user = 5;
            break;
        case('g'):
            hora_user = 6;
            break;
        case('h'):
            hora_user = 7;
            break;
        case('i'):
            hora_user = 8;
            break;
        case('j'):
            hora_user = 9;
            break;
        case('k'):
            hora_user = 10;
            break;
        case('l'):
            hora_user = 11;
            break;
        case('m'):
            hora_user = 12;
            break;
        case('n'):
            hora_user = 13;
            break;
        case('o'):
            hora_user = 14;
            break;
        case('p'):
            hora_user = 15;
            break;
        case('q'):
            hora_user = 16;
            break;
        case('r'):
            hora_user = 17;
            break;
        case('s'):
            hora_user = 18;
            break;
        case('t'):
            hora_user = 19;
            break;
        case('u'):
            hora_user = 20;
            break;
        case('v'):
            hora_user = 21;
            break;
        case('w'):
            hora_user = 22;
            break;
        case('x'):
            hora_user = 23;
            break;
        case('y'):
            hora_user = 24;
            break;
    }
    return hora_user;
}
char CharToNumM(char in, char her){ //conversion de seleccion a numero
    if(in == 'a'){
        her = 0;
    }else if(in == 'b'){
        her = 15;
    }else if(in == 'c'){
        her = 30;
    }else if(in == 'd'){
        her = 45;
    }else if(in == 'e'){
        her = 55;
    }
    return her;
}

void WriteToEEPROM(uint8_t dato, uint8_t addr){ //Escribir EEPROM
    EEADR = addr;
    EEDAT = dato;
    
    EECON1bits.EEPGD = 0;
    EECON1bits.WREN = 1;
    
    INTCONbits.GIE = 0;
    RCSTAbits.SPEN = 0;     //enciendo el modulo
    RCSTAbits.CREN = 0;     //activo recepción
    TXSTAbits.TXEN = 0;     //activo transmision 

    EECON2 = 0x55;
    EECON2 = 0xAA;
    
    EECON1bits.WR = 1;
    
    while(PIR2bits.EEIF == 0);  //espero a que escriba
    PIR2bits.EEIF = 0;
    
    EECON1bits.WREN = 0;
    INTCONbits.GIE = 1;
    RCSTAbits.SPEN = 1;     //enciendo el modulo
    RCSTAbits.CREN = 1;     //activo recepción
    TXSTAbits.TXEN = 1;     //activo transmision 

    return;
}

uint8_t ReadToEEPROM(uint8_t addr){ // Leer EEPROM
    EEADR = addr;
    EECON1bits.EEPGD = 0;
    EECON1bits.RD = 1;
    uint8_t dato = EEADR;
    return dato;
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

