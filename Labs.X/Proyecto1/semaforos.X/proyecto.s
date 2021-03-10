;Archivo:	proyecto.S
;Dispositivo:	PIC16f887
;Autor:		Cristhofer Patzan
;Compilador:	pic-as (v2.30), MPLABX V5.45
;
;Programa:  3semaforos con indicador de tiempo y opcion a configurar los tiempos
;Hardware:  push button, leds, resistencias, display 7 seg cc y transistores
;
;Creado: 08 de mar, 2021
;Última modificación: 09 de mar, 2021

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
 
 MODO	EQU 0
 UP	EQU 1
 DOWN	EQU 2
 ;------------------------------------------------------------------------------
 ;  Macro
 ;------------------------------------------------------------------------------
  reiniciar_tmr0 macro
    banksel PORTA
    movlw   180
    movwf   TMR0    ;ciclo de 23ms
    bcf	    T0IF
  endm
  
  reiniciar_tmr1 macro
    banksel PORTA
    movlw   0xDC    ;el timmer contara a cada 0.25 segundos
    movwf   TMR1L   ;por lo que cargo 3036 en forma hexadecimal 0x0BDC
    movlw   0x0B
    movwf   TMR1H
    bcf	    PIR1, 0 ;bajo la bandera
  endm
  
 ;------------------------------------------------------------------------------
 ;  Variables
 ;------------------------------------------------------------------------------
 
 PSECT udata_bank0  ;common memory
    tiempo1:	   DS 1 ;1 BYTE, 2 DECIMALES
    tiempo2:	   DS 1 ;1 BYTE, 2 DECIMALES
    tiempo3:	   DS 1 ;1 BYTE, 2 DECIMALES
    tiempo:	   DS 1	;registro de precargado
    estado:	   DS 1 ;registro de modo operando
    cont:	   DS 1 ;Segundos que aumento o diminuyo
    
    ;datos para cada display y seleccion de display
    banderas:	DS 1
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
   
   btfsc   PIR1, 0  ;TMR1IF
   call    T1_int
    
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
  goto	display_2
  
  btfss	banderas, 3
  goto	display_3
  
  btfss	banderas, 4
  goto	display_4
  
  btfss	banderas, 5
  goto	display_5
  
  
  ;para el semaforo 1
  display_0:
    bsf	    banderas, 0
    movf    display_var+0, W
    movwf   PORTC
    bsf	    PORTA, 1
    return
  display_1:
    bsf	    banderas, 1
    movf    display_var+1, W
    movwf   PORTC
    bsf	    PORTA, 0
    return
    
  ;para el semaforo 2  
  display_2:
    bsf	    banderas, 2
    movf    display_var+2, W
    movwf   PORTC
    bsf	    PORTA, 3
    return
  display_3:
    bsf	    banderas, 3
    movf    display_var+3, W
    movwf   PORTC
    bsf	    PORTA, 2
    return

  ;para el semaforo 3
  display_4:
    bsf	    banderas, 4
    movf    display_var+4, W
    movwf   PORTC
    bsf	    PORTA, 5
    return
 display_5:
    ;bsf     banderas, 5
    clrf    banderas
    movf    display_var+5, W
    movwf   PORTC
    bsf	    PORTA, 4
    return

 T1_int: 
    reiniciar_tmr1  ;25ms
    incf    cont
    movwf   cont, W
    sublw   4	    ;25ms * 4 = 1s
    btfss   ZERO
    goto    return_tm0
    clrf    cont
    incf    segm
 return_tm0:
    return
    
 OC_int:
    banksel PORTB
    
    btfss   PORTB, MODO
    incf    estado
    
    btfss   PORTB, UP
    incf    tiempo
    
    btfss   PORTB, DOWN
    decf    tiempo
    
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
    clrf    TRISE
    bsf	    TRISB, UP
    bsf	    TRISB, DOWN
    bsf	    TRISB, MODO
    
     ;conf. pull-up
    bcf	    OPTION_REG, 7   ;habilito pull-up
    bsf	    WPUB, UP	    ;selecciono que pines
    bsf	    WPUB, DOWN
    bsf	    WPUB, MODO
    
    banksel OSCCON
    bsf	    IRCF2   ;8MHZ = 111
    bsf	    IRCF1
    bsf	    IRCF0
    bsf	    SCS	    ;reloj interno activo
    
    call    conf_tmr0 
    call    conf_tmr1
    call    conf_interrupt_oc
    call    conf_interrupt_ena
    
    banksel PORTA   ;Me asegure que empiece en cero
    clrf    PORTA
    clrf    PORTC
    clrf    PORTD
    clrf    PORTE
    movlw   10
    movwf   tiempo1
    movwf   tiempo2
    movwf   tiempo3
    movlw   1
    movwf   estado
    movwf   tiempo
    movlw   11
    movwf   veces
   
 ;------------------------------------------------------------------------------
 ;  loop principal
 ;------------------------------------------------------------------------------
 
 loop:
    
    movlw   1
    subwf   estado, W
    btfsc   ZERO
    goto    default
    
    movlw   2
    subwf   estado, W
    btfsc   ZERO
    goto    confiS1
    
    movlw   3
    subwf   estado, W
    btfsc   ZERO
    goto    confiS2
    
    movlw   4
    subwf   estado, W
    btfsc   ZERO
    goto    confiS3
    
    movlw   5
    subwf   estado, W
    btfsc   ZERO
    goto    decision
    
    goto    loop
    
    default:
	bsf	PORTD,0	;enciendo led verde del semaforo y los demas en rojo
	bcf	PORTD,1
	bcf	PORTD,2
	bcf	PORTD,3
	bcf	PORTD,4
	bsf	PORTD,5
	bcf	PORTE,0
	bcf	PORTE,1
	bsf	PORTE,2
	
	movlw	3
	subwf	tiempo1, W
	btfsc	ZERO
	call	parpadeo
	
	goto    loop

    confiS1:
	bcf	PORTD,0	;primer semaforo
	bsf	PORTD,1
	bsf	PORTD,2
	bcf	PORTD,3	;segundo semaforo
	bcf	PORTD,4
	bsf	PORTD,5
	bcf	PORTE,0	;tercer semaforo 
	bcf	PORTE,1
	bsf	PORTE,2
	goto    loop

    confiS2:
	bcf	PORTD,0	;primer semaforo
	bcf	PORTD,1
	bsf	PORTD,2
	bcf	PORTD,3	;segundo semaforo
	bsf	PORTD,4
	bsf	PORTD,5
	bcf	PORTE,0	;tercer semaforo 
	bcf	PORTE,1
	bsf	PORTE,2
	
	goto    loop

    confiS3:
	bcf	PORTD,0	;primer semaforo
	bcf	PORTD,1
	bsf	PORTD,2
	bcf	PORTD,3	;segundo semaforo
	bcf	PORTD,4
	bsf	PORTD,5
	bcf	PORTE,0	;tercer semaforo 
	bsf	PORTE,1
	bsf	PORTE,2
	
	goto    loop

    decision:
	bcf	PORTD,0	;primer semaforo
	bsf	PORTD,1
	bsf	PORTD,2
	bcf	PORTD,3	;segundo semaforo
	bsf	PORTD,4
	bsf	PORTD,5
	bcf	PORTE,0	;tercer semaforo 
	bsf	PORTE,1
	bsf	PORTE,2
	
	btfss	PORTB, UP
	call	cargar
	btfss	PORTB, DOWN
	call	retachar
	
	goto    loop


 ;------------------------------------------------------------------------------
 ;	sub rutinas
 ;------------------------------------------------------------------------------
 parpadeo:
    
 
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
    bsf	    IOCB, MODO
    
    banksel PORTA
    movf    PORTB, W	;al leer termina condicion de ser distintos (mismatch)
    bcf	    RBIF
    return
 
 conf_interrupt_ena:
    bsf	    GIE
    
    bsf	    T0IE
    bcf	    T0IF
    
    bsf	    PIE1, 0 
    bcf	    PIR1, 0 ;TMR1IF
    
    bsf	    RBIE
    bcf	    RBIF
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
    
 cargar:
    return
 retachar:
    movlw    1
    movwf   estado
    return
END


