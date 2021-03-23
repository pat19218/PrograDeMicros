;Archivo:	Lab6.S
;Dispositivo:	PIC16f887
;Autor:		Cristhofer Patzan
;Compilador:	pic-as (v2.30), MPLABX V5.45
;
;Programa:  3semaforos con indicador de tiempo y opcion a configurar los tiempos
;Hardware:  push button, leds, resistencias, display 7 seg cc y transistores
;
;Creado: 23 de mar, 2021
;Última modificación: 23 de mar, 2021

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
 ;  Macro
 ;------------------------------------------------------------------------------
  reiniciar_tmr0 macro
    banksel PORTA
    movlw   230
    movwf   TMR0    ;ciclo de 3ms
    bcf	    T0IF
  endm
  
  reiniciar_tmr1 macro
    banksel PORTA
    movlw   0xDC    ;el timmer contara a cada 0.50 segundos
    movwf   TMR1L   ;por lo que cargo 3036 en forma hexadecimal 0x0BDC
    movlw   0x0B
    movwf   TMR1H
    bcf	    PIR1, 0 ;bajo la bandera
  endm
  
 ;------------------------------------------------------------------------------
 ;  Variables
 ;------------------------------------------------------------------------------
 
 PSECT udata_bank0  ;common memory
    tiempo:	   DS 1	;registro de precargado
    cont:	   DS 1 ;Segundos que aumento
    cont2:	   DS 1 ;para 250ms
    segundos:	   DS 1 ;cuenta los segundos
    
    decena:	   DS 1
    unidad:	   DS 1
    dividendo:	   DS 1
    resta:	   DS 1
    
    ;datos para cada display y seleccion de display
    banderas:	   DS 1	
    display_var:   DS 2
 
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
   btfsc   T0IF	    ;TMR0IF
   call    T0_int
   
   btfsc   PIR1, 0  ;TMR1IF
   call    T1_int
   
   btfsc   PIR1, 1  ;TMR2IF
   call    T2_int

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
  reiniciar_tmr0  ;3ms
  clrf	PORTD
  btfss	banderas, 0 ;chequeo turno del display
  goto	display_0
  goto	display_1
  
  ;para el semaforo 1
  display_0:
    btfss   PORTA, 0
    return
    bsf	    banderas, 0
    movf    display_var+0, W
    movwf   PORTC
    bsf	    PORTD, 1
    return
  display_1:
    btfss   PORTA, 0
    return
    bcf	    banderas, 0
    movf    display_var+1, W
    movwf   PORTC
    bsf	    PORTD, 0
    return

 T1_int: 
    reiniciar_tmr1  ;500ms
    incf    cont
    movwf   cont, W
    sublw   2	    ;500ms * 2 = 1s
    btfss   ZERO
    goto    return_tm1
    clrf    cont	;si ha pasado un segundo then incrementa la variable
    incf    segundos	;para indicar los segundo transcurridos
 return_tm1:
    return
    
 T2_int:
    clrf    TMR2    ;Bajo banderas
    bcf	    PIR1, 1 ; TMR2IF
    
    incf    cont2
    movwf   cont2, W
    sublw   5	    ;25ms * 10 = 250ms
    btfss   ZERO
    goto    return_tm2	;si no ha pasado el segundo solo regresa
    clrf    cont2	;si ha pasado un segundo then incrementa la variable
    
    btfsc   PORTA, 0
    goto    next
    bsf	    PORTA, 0
    
return_tm2:   
    return
next:
    bcf	    PORTA,0
    return
 ;------------------------------------------------------------------------------
 ;	TABLA / INICIO DEL CODIGO 
 ;------------------------------------------------------------------------------
    
 PSECT code, delta=2, abs
 ORG 100h   ;posicion para el código
tabla: 
    clrf    PCLATH
    bsf	    PCLATH, 0	;PCLATH = 01
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
   
    banksel OSCCON
    bsf	    IRCF2   ;4MHZ = 110
    bsf	    IRCF1
    bcf	    IRCF0
    bsf	    SCS	    ;reloj interno activo
    
    call    conf_tmr0 
    call    conf_tmr1 
    call    conf_tmr2
    call    conf_interrupt_ena
    
    banksel PORTA   ;Me asegure que empiece en cero
    clrf    PORTA
    clrf    PORTC
    clrf    PORTD
    clrf    banderas
    clrf    segundos

   
 ;------------------------------------------------------------------------------
 ;  loop principal
 ;------------------------------------------------------------------------------
 
 loop:
    movf    segundos, W
    movwf   dividendo
    call    dividir_10
    call    preparar_display1
    
    goto loop
 ;------------------------------------------------------------------------------
 ;				sub rutinas
 ;------------------------------------------------------------------------------
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
    
    movlw   10
    subwf   decena, W
    btfsc   ZERO    
    clrf    decena
    return
    
  preparar_display1:
    
    movf    decena, W	    ;traduzco el binario a decimal de los display 
    call    tabla
    movwf   display_var+1
    
    movf    dividendo, W
    call    tabla
    movwf   display_var
    return
    
 conf_tmr0:
    banksel TRISA
    bcf	    T0CS    ;usar el reloj interno, temporizador
    bcf	    PSA	    ;usar prescaler
    bsf	    PS2
    bsf	    PS1 
    bcf	    PS0	    ;PS = 110 /1:128
    reiniciar_tmr0
    return
 
 conf_interrupt_ena:
    bsf	    GIE
    bsf	    T0IE
    bcf	    T0IF
    bsf	    PIE1, 0 
    bsf	    PIE1, 1  ;TMR2IE
    bcf	    PIR1, 0 ;TMR1IF
    bcf	    PIR1, 1 ;TMR2IF
    return
    
 conf_tmr1:
    banksel TRISA
    bsf	    PIE1, 0	;enable del timer1
    banksel T1CON
    bcf	    T1CON, 1	;TEMPORIZADOR
    bsf	    T1CON, 5
    bsf	    T1CON, 4	;pre escaler 1:8
    bcf	    T1CON, 3	;RC0
    bcf	    T1CON, 2	;sincronizacion
    bsf	    T1CON, 0	;timmer 1 ON
    reiniciar_tmr1
    return
 conf_tmr2:
    BANKSEL PORTA
    movlw   11111111B	;Configuracion del tmr2
    movwf   T2CON
    BANKSEL TRISA
    movlw   98		;valor a comparar, cuenta a cada 0.025s
    movwf   PR2
    BANKSEL PORTA
    clrf    TMR2
    bcf	    PIR1, 1 ;TRM2IF
    
    return
END


