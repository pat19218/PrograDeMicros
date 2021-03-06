;Archivo:	proyecto.S
;Dispositivo:	PIC16f887
;Autor:		Cristhofer Patzan
;Compilador:	pic-as (v2.30), MPLABX V5.45
;
;Programa:  3semaforos con indicador de tiempo y opcion a configurar los tiempos
;Hardware:  push button, leds, resistencias, display 7 seg cc y transistores
;
;Creado: 08 de mar, 2021
;Última modificación: 22 de mar, 2021

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
    tiempo1:	   DS 1 ;1 BYTE, 2 DECIMALES
    tiempo2:	   DS 1 ;1 BYTE, 2 DECIMALES
    tiempo3:	   DS 1 ;1 BYTE, 2 DECIMALES
    pretiempo1:	   DS 1 ;1 BYTE, 2 DECIMALES
    pretiempo2:	   DS 1 ;1 BYTE, 2 DECIMALES
    pretiempo3:	   DS 1 ;1 BYTE, 2 DECIMALES
    tiempo:	   DS 1	;registro de precargado
    estado:	   DS 1 ;registro de modo operando
    cont:	   DS 1 ;Segundos que aumento o diminuyo
    segundos:	   DS 1 ;cuenta los segundos
    cont_big:	   DS 1 ;primer delay
    cont_small:	   DS 1	;segundo delay
    semaforo:	   DS 1	;indica manera en encenderse los semaforos(next state)
    op:		   DS 1	;indica en que momento desactivo la secuencia
    parpadea:	   DS 1
    
    decena:	   DS 1
    unidad:	   DS 1
    dividendo:	   DS 1
    var:	   DS 3	;displays, valor del tiempo en semaforo
    resta:	   DS 1
    
    ;datos para cada display y seleccion de display
    banderas:	   DS 1	
    display_var:   DS 7
 
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
  reiniciar_tmr0  ;3ms
  clrf	PORTA
  btfss	banderas, 0 ;chequeo turno del display
  goto	display_0
  btfss	banderas, 1 ;chequeo turno del display
  goto	display_1
  btfss	banderas, 2 ;chequeo turno del display
  goto	display_2
  btfss	banderas, 3 ;chequeo turno del display
  goto	display_3
  btfss	banderas, 4 ;chequeo turno del display
  goto	display_4
  btfss	banderas, 5 ;chequeo turno del display
  goto	display_5
  btfss	banderas, 6 ;chequeo turno del display de confi. tiempo
  goto	display_6
  btfss	banderas, 7 ;chequeo turno del display de confi. tiemp
  goto	display_7
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
    bsf     banderas, 5
    movf    display_var+5, W
    movwf   PORTC
    bsf	    PORTA, 4
    return
   
    ;ver tiempo a modificar
 display_6:
    bsf	    banderas, 6
    movf    display_var+7, W
    movwf   PORTC
    bsf	    PORTA, 7
    return
 display_7:
    clrf    banderas
    movf    display_var+6, W
    movwf   PORTC
    bsf	    PORTA, 6
    return

 T1_int: 
    reiniciar_tmr1  ;50ms
    incf    cont
    movwf   cont, W
    sublw   2	    ;500ms * 2 = 1s
    btfss   ZERO
    goto    return_tm1
    clrf    cont	;si ha pasado un segundo then incrementa la variable
    incf    segundos	;para indicar los segundo transcurridos
    
    btfss   resta, 0	;me asegure de decrementar el tiempo solo hasta cero
    decf    var		;asi me aseguro de que no falle el codigo
    btfsc   ZERO
    bsf	    resta, 0
    
    btfss   resta, 1
    decf    var+1
    btfsc   ZERO
    bsf	    resta, 1
    
    btfss   resta, 2
    decf    var+2
    btfsc   ZERO
    bsf	    resta, 2
    
 return_tm1:
    movlw   1
    xorwf   segm, F
    return
    
 OC_int:
    banksel PORTB
    btfss   PORTB, MODO
    incf    estado	;selecciono el modo por el push button
    movlw   6
    subwf   estado, W	;me aseguro que si llega a la ultima opcion y se 
    btfss   ZERO	;presiona, regrese a la confi 1
    goto    $+3
    movlw   2
    movwf   estado
    
    btfss   PORTB, UP	;incremento el tiempo de configuración
    incf    tiempo, F
    movlw   21		;me aseguro que solo llegue hasta 20 y luego de sea 10
    subwf   tiempo, W
    btfsc   ZERO
    goto    min
    
    btfss   PORTB, DOWN
    decf    tiempo, F
    movlw   9		;me aseguro que solo llegue a 10 y luego sea 20
    subwf   tiempo, W
    btfsc   ZERO
    goto    max
    bcf	    RBIF
    return
  min:
    movlw   10
    movwf   tiempo
    bcf	    RBIF
    return
  max:
    movlw   20
    movwf   tiempo
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
    clrf    TRISB
    bsf	    TRISB, UP
    bsf	    TRISB, DOWN
    bsf	    TRISB, MODO
    
     ;conf. pull-up
    bcf	    OPTION_REG, 7   ;habilito pull-up
    bsf	    WPUB, UP	    ;selecciono que pines
    bsf	    WPUB, DOWN
    bsf	    WPUB, MODO
    
    banksel OSCCON
    bsf	    IRCF2   ;4MHZ = 110
    bsf	    IRCF1
    bcf	    IRCF0
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
    clrf    PORTB
    clrf    segundos
    clrf    semaforo
    clrf    pretiempo1
    clrf    pretiempo2
    clrf    pretiempo3
    clrf    op
    movlw   9
    movwf   tiempo1
    movwf   tiempo2
    movwf   tiempo3
    movf    tiempo1, W
    movwf   var
    movf    tiempo2, W
    movwf   var+1
    movf    tiempo3, W
    movwf   var+2
    movlw   1
    movwf   estado
    movlw   10
    movwf   tiempo
    movlw   11111110B
    movwf   resta
    movlw   11111111B
    movwf   parpadea

   
 ;------------------------------------------------------------------------------
 ;  loop principal
 ;------------------------------------------------------------------------------
 
 loop:
    
    movf    var, W		;le asigno los tiempos respectivos a cada
    movwf   dividendo		;display del semaforo indicado
    call    dividir_10
    call    preparar_display1
    
    movf    var+1, W
    movwf   dividendo
    call    dividir_10
    call    preparar_display2
    
    movf    var+2, W
    movwf   dividendo
    call    dividir_10
    call    preparar_display3
    
    btfss   op, 0
    call    default	;Semaforo funcionando normal
    
    movlw   2
    subwf   estado, W
    btfsc   ZERO
    call    confiS1	;Configurar el tiempo del semaforo 1
    
    movlw   3
    subwf   estado, W
    btfsc   ZERO
    call    confiS2	;Configurar el tiempo del semaforo 2
    
    movlw   4
    subwf   estado, W
    btfsc   ZERO
    call    confiS3	;Configurar el tiempo del semaforo 3
    
    movlw   5
    subwf   estado, W
    btfsc   ZERO
    call    decision	;Acepto los tiempos precargados
    
    goto    loop
    
    default:
	btfss   semaforo, 0 ;primer secuencia, semaforo 1 da via segun el time1
	call    parte1	
	btfss	parpadea, 0 ;la luz parpadea durante 3 segundos
	call	parpadeo1
	btfsc   semaforo, 1 ;luz amarilla durante 3 segundos
	call    parte2		
	btfsc   semaforo, 2 ;2da secuencia, semaforo 2 da via segun el time2
	call    parte3	
	btfss	parpadea, 1 ;la luz parpadea durante 3 segundos
	call	parpadeo2
	btfsc   semaforo, 3 ;luz amarilla durante 3 segundos
	call    parte4	
	btfsc   semaforo, 4 ;3ra secuencia, semaforo 3 da via segun el time1
	call    parte5	
	btfss	parpadea, 2 ;la luz parpadea durante 3 segundos
	call	parpadeo3
	btfsc   semaforo, 5 ;luz amarilla durante 3 segundos
	call    parte6	
	return
	
    confiS1:
	bsf	PORTB, 3
	bcf	PORTB, 4
	bcf	PORTB, 5
	movf	tiempo, W
	movwf	pretiempo1	;tomo el valor actual y lo guardo temporalmente
	movf	pretiempo1, W
	movwf	dividendo	;dividiendo tiene el valor que se piensa subir
	call    dividir_10	;analizo cuantas decenas tiene ese valor
	call    preparar_display;mando los datos correspondientes a cada display
	return

    confiS2:
	bcf	PORTB, 3
	bsf	PORTB, 4
	bcf	PORTB, 5
	movf	tiempo, W
	movwf	pretiempo2
	movf	pretiempo2, W
	movwf	dividendo	;dividiendo tiene el valor que se piensa subir
	call    dividir_10	;analizo cuantas decenas tiene ese valor
	call    preparar_display;mando los datos correspondientes a cada display
	return

    confiS3:
	bcf	PORTB, 3
	bcf	PORTB, 4
	bsf	PORTB, 5
	movf	tiempo, W
	movwf	pretiempo3
	movf	pretiempo3, W
	movwf	dividendo	;dividiendo tiene el valor que se piensa subir
	call    dividir_10	;analizo cuantas decenas tiene ese valor
	call    preparar_display;mando los datos correspondientes a cada display
	return

    decision:
	bsf	op, 0	    ;desactivo el semaforo
	bsf	PORTB, 3
	bsf	PORTB, 4
	bsf	PORTB, 5
	movlw	000110110B  ;indicacion de semafotos
	movwf	PORTD
	movlw	000000110B
	movwf	PORTE
	btfss	PORTB, UP   ;selecion
	call	cargar
	btfss	PORTB, DOWN
	call	retachar
	return


 ;------------------------------------------------------------------------------
 ;	sub rutinas
 ;------------------------------------------------------------------------------
  
 parte1:
    movlw   00100001B	;estado del primer y segundo semaforo
    movwf   PORTD
    movlw   00000100B	;estado del segundo semaforo
    movwf   PORTE
    movlw   6	;le resto los segundo trascurridos al tiempo definido
    subwf   var, W
    btfss   STATUS, 2	;ZERO
    return
    bcf	    parpadea, 0
    bsf	    semaforo, 0	    
    return
    
 parte2:
    movlw   00100010B	;luz amarilla
    movwf   PORTD
    movlw   00000100B
    movwf   PORTE   
    movlw   3
    subwf   segundos, W
    btfsc   ZERO
    call    siguiente2
    return
 siguiente2:
    bcf	    semaforo, 1	;habilito el siguiente semaforo y desabilito el actual
    bsf	    semaforo, 2
    
    movf    tiempo2, W
    movwf   var+1, F
    movlw   11111101B
    movwf   resta
    clrf    segundos
    return
    
 parte3:
    movlw   00001100B	;estado del primer y segundo semaforo
    movwf   PORTD
    movlw   00000100B	;estado del segundo semaforo
    movwf   PORTE
    movlw   6	;le resto los segundo trascurridos al tiempo definido
    subwf   var+1, W
    btfss   STATUS, 2	;ZERO
    return
    bcf	    parpadea, 1
    bcf	    semaforo, 2	 
    return
    
 parte4:
    movlw   00010100B	;luz amarilla
    movwf   PORTD
    movlw   00000100B
    movwf   PORTE   
    movlw   3
    subwf   segundos, W
    btfsc   ZERO
    call    siguiente3
    return
 siguiente3:
    bcf	    semaforo, 3	;habilito el siguiente semafor y desactivo el actual
    bsf	    semaforo, 4
    movf    tiempo3, W
    movwf   var+2, F
    movlw   11111011B
    movwf   resta
    clrf    segundos
    return
 
parte5:
    movlw   00100100B	;estado del primer y segundo semaforo
    movwf   PORTD
    movlw   00000001B	;estado del segundo semaforo
    movwf   PORTE
    
    movlw   6	;le resto los segundo trascurridos al tiempo definido
    subwf   var+2, W
    btfss   STATUS, 2	;ZERO
    return
    bcf	    parpadea, 2
    bcf	    semaforo, 4	 
    
    return
    
 parte6:
    movlw   00100100B
    movwf   PORTD
    movlw   00000010B
    movwf   PORTE   
    movlw   3
    subwf   segundos, W
    btfsc   ZERO
    call    siguiente4
    return
 siguiente4:
    bcf	    semaforo, 5
    bcf	    semaforo, 0
    movf    tiempo1, W
    movwf   var, F
    movlw   11111110B
    movwf   resta
    clrf    segundos
    return
    
 parpadeo1:
    btfss   segm, 0
    bcf	    PORTD, 0
    btfsc   segm, 0
    bsf	    PORTD, 0
    
    movlw   3
    subwf   var, W
    btfss   ZERO
    return
    bsf	    parpadea, 0
    bsf	    semaforo, 1
    clrf    segundos
    return
 parpadeo2:
    btfss   segm, 0
    bcf	    PORTD, 3
    btfsc   segm, 0
    bsf	    PORTD, 3
    
    movlw   3
    subwf   var+1, W
    btfss   ZERO
    return
    bsf	    parpadea, 1
    bsf	    semaforo, 3
    clrf    segundos
    return
 parpadeo3:
    btfss   segm, 0
    bcf	    PORTE, 0
    btfsc   segm, 0
    bsf	    PORTE, 0

    movlw   3
    subwf   var+2, W
    btfss   ZERO
    return
    bsf	    parpadea, 2
    bsf	    semaforo, 5
    clrf    segundos
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
    
  preparar_display1:
    movf    decena, W	    ;traduzco el binario a decimal de los display 
    call    tabla
    movwf   display_var+1
    
    movf    dividendo, W
    call    tabla
    movwf   display_var
    return
  preparar_display2:
    movf    decena, W	    ;traduzco el binario a decimal de los display 
    call    tabla
    movwf   display_var+3
    
    movf    dividendo, W
    call    tabla
    movwf   display_var+2 
    return
   preparar_display3:
    movf    decena, W	    ;traduzco el binario a decimal de los display 
    call    tabla
    movwf   display_var+5
    
    movf    dividendo, W
    call    tabla
    movwf   display_var+4 
    return   
  preparar_display:
    movf    decena, W	;traduzco el binario a decimal de los display 
    call    tabla
    movwf   display_var+6
    
    movf    dividendo, W
    call    tabla
    movwf   display_var+7
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
    decf    tiempo	    ;paso los pretiempo al tiempo de cada semaforo
    movf    pretiempo1, W
    movwf   tiempo1, F
    movwf   var
    movf    pretiempo2, W
    movwf   tiempo2, F
    movwf   var+1
    movf    pretiempo3, W
    movwf   tiempo3, F
    movwf   var+2
    movlw   00000001B
    movwf   estado
    bcf	    op, 0	;activo secuencia del semaforo y apago leds de confi.
    bcf	PORTB, 3
    bcf	PORTB, 4
    bcf	PORTB, 5
    clrf    display_var+7
    clrf    display_var+6
    movlw   0000110B
    movwf   resta
    clrf    semaforo
    clrf    segundos
    return
    
 retachar:
    movlw   00000001B	;simplemente regreso a la rutina normal sin cargar nada
    movwf   estado
    incf    tiempo
    movf    tiempo1, W
    movwf   var
    movf    tiempo2, W
    movwf   var+1
    movf    tiempo3, W
    movwf   var+2
    bcf	    op, 0
    bcf	PORTB, 3
    bcf	PORTB, 4
    bcf	PORTB, 5
    clrf    display_var+7
    clrf    display_var+6
    movlw   0000110B
    movwf   resta
    clrf    semaforo
    clrf    segundos
    return
END


