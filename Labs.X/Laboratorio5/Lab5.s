;Archivo:	Lab5.S
;Dispositivo:	PIC16f887
;Autor:		Cristhofer Patzan
;Compilador:	pic-as (v2.30), MPLABX V5.45
;
;Programa:	contador en portD usando IOC y 5 display multiplexado en portC
;Hardware:	push button, leds, resistencias, display 7 seg cc y transistores
;
;Creado: 28 feb, 2021
;Última modificación: 28 feb, 2021

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
 

 UP	EQU 0
 DOWN	EQU 1
 ;------------------------------------------------------------------------------
 ;  Macro
 ;------------------------------------------------------------------------------
  reiniciar_tmr0 macro
    banksel PORTA
    movlw   180
    movwf   TMR0    ;ciclo de 23ms
    bcf	    T0IF
  endm
  
 ;------------------------------------------------------------------------------
 ;  Variables
 ;------------------------------------------------------------------------------
 
 PSECT udata_bank0  ;common memory
    centena:	DS 1 ;1 byte   
    decena:	DS 1
    unidad:	DS 1
    dividendo:	DS 1
    
    var:	DS 1
    banderas:	DS 1
    nibble:	DS 2
    display_var:DS 5
 
 PSECT udata_shr  ;common memory
    W_temp:	   DS 1 ;1 byte
    STATUS_temp:   DS 1 ;1 byte
    segm:	   DS 1 ;1 byte   

 ;------------------------------------------------------------------------------
 ;  Vector reset
 ;------------------------------------------------------------------------------
 PSECT resVect, class=CODE, abs, delta=2
 
 ORG 00h    ;  posición 0000h para el reset
 resetVec:
    PAGESEL main
    goto main

 ;------------------------------------------------------------------------------
 ;  Vector Interrupt
 ;------------------------------------------------------------------------------
 PSECT intVect, class=CODE, abs, delta=2
 
 ORG 04h    ;  posición 0004h para las interrupciones
 
 push:
    movwf   W_temp
    swapf   STATUS, W
    movwf   STATUS_temp
    
 isr:
   btfsc   T0IF
   call    T0_int
    
   btfsc    RBIF
   call	    OC_int
   
 pop:
    swapf   STATUS_temp, W
    movwf   STATUS
    swapf   W_temp, F
    swapf   W_temp, W
    retfie
 ;------------------------------------------------------------------------------
 ;	sub rutinas de interrupcion
 ;------------------------------------------------------------------------------
 T0_int:
  reiniciar_tmr0  ;50ms
  clrf	PORTA
  
  btfss	banderas, 0
  goto	display_0
  
  btfss	banderas, 1
  goto	display_1
  
  btfss	banderas, 2
  goto	display_5
  
  btfss	banderas, 3
  goto	display_4
  
  btfss	banderas, 4
  goto	display_3
  
  display_0:
    bsf	    banderas, 0
    movf    display_var+0, W
    movwf   PORTC
    bsf	    PORTA, 4
    return
  display_1:
    bsf	    banderas, 1
    movf    display_var+1, W
    movwf   PORTC
    bsf	    PORTA, 3
    return
    
 display_5:
    bsf	    banderas, 2
    movf    display_var+2, W
    movwf   PORTC
    bsf	    PORTA, 0
    return
 display_4:
    bsf	    banderas, 3
    movf    display_var+3, W
    movwf   PORTC
    bsf	    PORTA, 1
    return
 display_3:
    clrf    banderas
    movf    display_var+4, W
    movwf   PORTC
    bsf	    PORTA, 2
    return
 
 OC_int:
    banksel PORTB
    btfss   PORTB, UP
    incf    PORTD
    btfss   PORTB, DOWN
    decf    PORTD
    bcf	    RBIF
    return
 ;------------------------------------------------------------------------------
 ;	TABLA / INICIO DEL CODIGO 
 ;------------------------------------------------------------------------------
    
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
    clrf    ANSEL   ;I/O digitales
    clrf    ANSELH
    
    banksel TRISA
    clrf    TRISA
    clrf    TRISC
    clrf    TRISD
    bsf	    TRISB, UP
    bsf	    TRISB, DOWN
    
     ;conf. pull-up
    bcf	    OPTION_REG, 7   ;habilito pull-up
    bsf	    WPUB, UP	    ;selecciono que pines
    bsf	    WPUB, DOWN    
    
    banksel OSCCON
    bsf	    IRCF2   ;4MHZ = 110
    bsf	    IRCF1
    bcf	    IRCF0
    bsf	    SCS	    ;reloj interno activo
    
    call    conf_tmr0 
    call    conf_interrupt_oc
    call    conf_interrupt_ena
    banksel PORTA   ;Me asegure que empiece en cero
    clrf    PORTA
    clrf    PORTC
    clrf    PORTD
   
 ;------------------------------------------------------------------------------
 ;  loop principal
 ;------------------------------------------------------------------------------
 
 loop:
    ;parte 2
    movf   PORTD, W
    movwf   var
    call    separarar_nibbles
    call    preparar_display
    
    ;parte 3
    movf    PORTD, W
    movwf   dividendo
    
    call    dividir_100
    movf    centena, W
    
    call    dividir_10
    movf    decena, W

    call    dividir_1
    movf    unidad, W

   
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
    bcf	    PS0	    ;PS = 110 /1:128
    reiniciar_tmr0
    return
 
 conf_interrupt_oc:
    banksel TRISB
    bsf	    IOCB, UP
    bsf	    IOCB, DOWN
    
    banksel PORTA
    movf    PORTB, W	;al leer termina condicion de ser distintos (mismatch)
    bcf	    RBIF
    return
 
 conf_interrupt_ena:
    bsf	    GIE
    bsf	    T0IE
    bcf	    T0IF
    bsf	    RBIE
    bcf	    RBIF
    return
    
 separarar_nibbles:
    movf    var, W
    andlw   0x0f
    movwf   nibble
    swapf   var, W
    andlw   0x0f
    movwf   nibble+1
    return
    
 preparar_display:
    movf    nibble, W
    call    tabla
    movwf   display_var
    movf    nibble+1, W
    call    tabla
    movwf   display_var+1
    
    movf    centena, W
    call    tabla
    movwf   display_var+2
    
    movf    decena, W
    call    tabla
    movwf   display_var+3
    
    movf    unidad, W
    call    tabla
    movwf   display_var+4
    return
    
 dividir_100:
   ;Si la resta es positiva => c=1 z=0
   ;Si la resta es 0        => c=1 z=1
   ;Si la resta es negativa => c=0 z=0
    clrf    centena	;limpio mi variable
    movlw   100	
    subwf   dividendo, F; le resto 100 al dividendo
    btfsc   CARRY	; si carry es 0 then 100 es mayor que el puerto
    incf    centena	; si carry es 1 then 100 es menor que el puerto
    btfsc   CARRY
    goto    $-5 
    movlw   100
    addwf   dividendo, F
    return
 
 dividir_10:
    clrf    decena
    movlw   10
    subwf   dividendo, F
    btfsc   CARRY	; si carry es 0 then 10 es mayor que dividendo
    incf    decena	; si carry es 1 then 10 es menor que dividendo
    btfsc   CARRY
    goto    $-5 
    movlw   10
    addwf   dividendo, F
       
    return
  
 dividir_1:
    clrf    unidad
    movlw   1
    subwf   dividendo, F
    btfsc   CARRY	; si carry es 0 then 100 es mayor que el residuo
    incf    unidad	; si carry es 1 then 100 es menor que el residuo
    btfsc   CARRY
    goto    $-5 
    movlw   1
    addwf   dividendo, F
    
    return
    
END