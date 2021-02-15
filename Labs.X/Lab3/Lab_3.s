;Archivo:	Lab3.S
;Dispositivo:	PIC16f887
;Autor:		Cristhofer Patzan
;Compilador:	pic-as (v2.30), MPLABX V5.45
;
;Programa:	contador 4 bits y comparador
;Hardware:	LEDs en el puerto C,D y display puerto A Button en el puerto A
;
;Creado: 11 feb, 2021
;Última modificación: 11 feb, 2021

;//////////////////////////////////////////////////////////////////////////////
; Configuration word 1
; PIC16F887 Configuration Bit Settings
; Assembly source line config statements
;//////////////////////////////////////////////////////////////////////////////
PROCESSOR 16F887
#include <xc.inc>

; CONFIG1
  CONFIG  FOSC = INTRC_NOCLKOUT ; Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
  CONFIG  WDTE = OFF            ; Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
  CONFIG  PWRTE = OFF           ; Power-up Timer Enable bit (PWRT disabled)
  CONFIG  MCLRE = OFF           ; RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
  CONFIG  CP = OFF              ; Code Protection bit (Program memory code protection is disabled)
  CONFIG  CPD = OFF             ; Data Code Protection bit (Data memory code protection is disabled)
  CONFIG  BOREN = OFF           ; Brown Out Reset Selection bits (BOR disabled)
  CONFIG  IESO = OFF            ; Internal External Switchover bit (Internal/External Switchover mode is disabled)
  CONFIG  FCMEN = OFF           ; Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
  CONFIG  LVP = ON              ; Low Voltage Programming Enable bit (RB3/PGM pin has PGM function, low voltage programming enabled)

; CONFIG2
  CONFIG  BOR4V = BOR40V        ; Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
  CONFIG  WRT = OFF             ; Flash Program Memory Self Write Enable bits (Write protection off)
  
     
 ;------------------------------------------------------------------------------
 ;  Vector reset
 ;------------------------------------------------------------------------------
 PSECT resVect, class=CODE, abs, delta=2
 
 ORG 00h    ;  posición 0000h para el reset
 resetVec:
    PAGESEL main
    goto main
    
    
 PSECT code, delta=2, abs
 ORG 100h   ;posicion para el código
 
 ;------------------------------------------------------------------------------
 ;  Configuracion microprcesador
 ;------------------------------------------------------------------------------
 
 main:
    banksel ANSEL   ;Selecciono el banco donde esta ANSEL
    clrf    ANSEL
    clrf    ANSELH
    
    banksel TRISA   ;Puerto A pin 6 y 7 entradas el resto salidas
    clrf    TRISA
    bsf	    TRISA, 6
    bsf	    TRISA, 7
    
    clrf    TRISC   ;Pines de salida, luego desabilito pines
    bsf	    TRISC, 4
    bsf	    TRISC, 5
    bsf	    TRISC, 6
    bsf	    TRISC, 7
    
    bcf	    TRISD, 0 ;Pin de salida puerto D
    
    banksel PORTA   ;Me asegure que empiece en cero
    clrf    PORTA
    clrf    PORTD 
    clrf    PORTC 
    
    banksel OSCCON
    bsf	    IRCF2   ;1MHZ = 100
    bcf	    IRCF1
    bcf	    IRCF0
    bsf	    SCS	    ;reloj interno activo
    
    call    conf_tmr0
    
    
 ;------------------------------------------------------------------------------
 ;  loop principal
 ;------------------------------------------------------------------------------
 
 loop:
   ;parte 1
   
    
   ;parte 2 CONTADOR 2 conectado a display
    btfsc   PORTA, 6	
    call    inc_porta	
    btfsc   PORTA, 7	
    call    dec_porta	
    
    goto    loop
    
 ;------------------------------------------------------------------------------
 ;	sub rutinas
 ;------------------------------------------------------------------------------
 
 conf_tmr0:
    banksel TRISA
    
    bcf	    T0CS    ;usar el reloj interno
    bcf	    PSA	    ;usar prescaler
    bsf	    PS2
    bsf	    PS1
    bcf	    PS0	    ;PS = 110 /1:128
    
    banksel PORTA
    call    reiniciar_tmr0
    return
 
reiniciar_tmr0:
    movlw   0
    movwf   TMR0
    bcf	    T0IF
    return
    
 inc_porta:		; loop de incremento de bit por botonazo
    btfsc   PORTA, 6	;
    goto    $-1
    incf    PORTA, F	;
    return
  
  dec_porta:		; loop de incremento de bit por botonazo
    btfsc   PORTA, 7	;
    goto    $-1
    decf    PORTA, F	;
    return
 
 END