;Archivo:	Lab3.S
;Dispositivo:	PIC16f887
;Autor:		Cristhofer Patzan
;Compilador:	pic-as (v2.30), MPLABX V5.45
;
;Programa:	contador 4 bits y comparador
;Hardware:	LEDs en el puerto C,D y display puerto A Button en el puerto A
;
;Creado: 11 feb, 2021
;Última modificación: 18 feb, 2021

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
 ;  Variables
 ;------------------------------------------------------------------------------
 
 PSECT udata_bank0  ;common memory
    cont:   DS 1 ;1 byte   
 PSECT udata_bank0  ;common memory
    cont_small:   DS 3 ;1 byte

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
tabla: 
    clrf    PCLATH
    bsf	    PCLATH, 0	;PCLATH = 01
    andwf   0x0f	;me aseguro q solo pasen 4 bits
    addwf   PCL		;PC = PCL + PCLATH + w
    retlw   11111100B	;0  posicion 0
    retlw   01100000B	;1  posicion 1
    retlw   11011010B	;2  posicion 2
    retlw   11110010B	;3  posicion 3
    retlw   01100110B	;4  posicion 4
    retlw   10110110B	;5  posicion 5
    retlw   10111110B	;6  posicion 6
    retlw   11100000B	;7  posicion 7
    retlw   11111110B	;8  posicion 8
    retlw   11110110B	;9  posicion 9
    retlw   11101110B	;A  posicion 10
    retlw   00111110B	;B  posicion 11
    retlw   10011100B	;C  posicion 12
    retlw   01111010B	;D  posicion 13
    retlw   10011110B	;E  posicion 14
    retlw   10001110B	;F  posicion 15
 
 ;------------------------------------------------------------------------------
 ;  Configuracion microprcesador
 ;------------------------------------------------------------------------------
 
 main:
    banksel ANSEL   ;Selecciono el banco donde esta ANSEL
    clrf    ANSEL
    clrf    ANSELH
    
    banksel TRISB   ;Puerto B pin 0 y 1 entradas el Puert A salidas
    clrf    TRISB
    bsf	    TRISB, 0
    bsf	    TRISB, 1
    
    clrf    TRISC   ;Pines de salida, luego desabilito pines
    bsf	    TRISC, 4
    bsf	    TRISC, 5
    bsf	    TRISC, 6
    bsf	    TRISC, 7
    
    clrf    TRISD   ;Pin de salida puerto D
    
    clrf    TRISA   ;Pin de salida puerto A
    
    banksel PORTA   ;Me asegure que empiece en cero
    clrf    PORTA
    movlw   11111100B
    movwf   PORTA
    clrf    PORTB
    clrf    PORTD 
    clrf    PORTC 
    
    banksel OSCCON
    bcf	    IRCF2   ;500kHZ = 011
    bsf	    IRCF1
    bsf	    IRCF0
    bsf	    SCS	    ;reloj interno activo
    
    call    conf_tmr0 
    banksel PORTA
    movlw   0x00
    movwf   cont
    
    
 ;------------------------------------------------------------------------------
 ;  loop principal
 ;------------------------------------------------------------------------------
 
 loop:
   ;parte 1 contador 4 bits usando el timmer-0
   btfsc    T0IF
   call	    inc_portc
   
   ;parte 2 CONTADOR 2 conectado a display
   btfsc   PORTB, 0	
   call    inc_porta	
   btfsc   PORTB, 1	
   call    dec_porta	
   
   ;parte 3
   ;Si la resta es positiva => c=1 z=0
   ;Si la resta es 0        => c=1 z=1
   ;Si la resta es negativa => c=0 z=0
   
   movwf   cont, W
   subwf   PORTC, W	;PORTC - CONT = W
   btfsc   STATUS, 2	;chequeo la bandera de ZERO -PAG31-
   call	   igualdad
   btfsc   STATUS, 2
   bcf	    PORTD, 0
   
   goto    loop
    
 ;------------------------------------------------------------------------------
 ;	sub rutinas
 ;------------------------------------------------------------------------------
 
 conf_tmr0:
    banksel TRISA
    bcf	    T0CS    ;usar el reloj interno, temporizador
    bcf	    PSA	    ;usar prescaler
    bsf	    PS2
    bsf	    PS1
    bsf	    PS0	    ;PS = 111 /1:256
    banksel PORTA
    call    reiniciar_tmr0
    return
 
 reiniciar_tmr0:
    movlw   12
    movwf   TMR0
    bcf	    T0IF
    btfsc   STATUS, 2
    clrf    PORTC
    return
    
 inc_porta:		; loop de incremento de bit por botonazo
    btfsc   PORTB, 0	
    goto    $-1
    incf    cont	;Aumento el valor de mi variable
    movf    cont, W	;Guardo la variable en el registro W
    call    tabla	;voy a la tabla en la posicion del cont y se guarda en w
    movwf   PORTA	;el dato de w lo mando al puerto A
    return
    
  dec_porta:		; loop de incremento de bit por botonazo
    btfsc   PORTB, 1	
    goto    $-1
    decf    cont	;Disminuyo el valor de mi variable
    movf    cont, W	;Guardo la variable en el registro W
    call    tabla	;voy a la tabla en la posicion del cont y se guarda en w
    movwf   PORTA	;el dato de w lo mando al puerto A
    return
  
 inc_portc:		; Incremento puerto C y reseteo el timmer-0
    call    reiniciar_tmr0
    incf    PORTC, F	
    return
    
 igualdad:
    bsf	    PORTD, 0
    call    reiniciar_tmr0
    
    movlw   765		    ;valor inicial del contador
    movwf   cont_small
    decfsz  cont_small, 1   ;decrementar el contador
    goto    $-1		    ;ejecutar línea anterior
    return

 END