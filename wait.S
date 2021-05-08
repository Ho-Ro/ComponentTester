/* ************************************************************************
 *
 *   wait functions
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define WAIT_S


/*
 *  includes
 */

/* basic includes */
#include <avr/io.h>

/* local includes */
#include "config.h"           /* global configuration */



.func wait1000ms
.global wait5s		; wait 5 seconds
.global wait4s		; wait 4 seconds
.global wait3s		; wait 3 seconds
.global wait2s		; wait 2 seconds
.global wait1s		; wait 1 seconds
.global wait1000ms	; wait 2 second and wait 1000ms are identical
.global wait500ms	; wait 500ms
.global wait400ms	; wait 400ms
.global wait300ms	; wait 300ms
.global wait200ms	; wait 200ms
.global wait100ms	; wait 100ms
.global wait50ms	; wait 50ms
.global wait40ms	; wait 40ms
.global wait30ms	; wait 30ms
.global wait20ms	; wait 20ms
.global wait10ms	; wait 10ms
.global wait5ms		; wait 5ms
.global wait4ms		; wait 4ms
.global wait3ms		; wait 3ms
.global wait2ms		; wait 2ms
.global wait1ms		; wait 1ms
.global wait500us	; wait 500쨉s
.global wait400us	; wait 400쨉s
.global wait300us	; wait 300쨉s
.global wait200us	; wait 200쨉s
.global wait100us	; wait 100쨉s
.global wait50us	; wait 50쨉s
.global wait40us	; wait 40쨉s
.global wait30us	; wait 30쨉s
.global wait20us	; wait 20쨉s
.global wait10us	; wait 10쨉s

;waiting loops for ATmega at a clock rates of 1MHz, 2MHz, 4MHz and 8MHz
; flash memory usage (includes 2 Bytes Watch Dog reset at wait100ms):
; 68 Byte at 1MHz
; 70 Byte at 2MHz
; 76 Byte at 4MHz
; 74 Byte at 8Mhz
; No registers are used (only stack pointer).
; A maximum of 28 bytes of space for return addresses is used in RAM.
; Every wait call needs only one instruction (rcall).
; I see no way to implement this function with C-language (too tricky).


/*
 *  20탎 and up (clock rate independent)
 */

wait5s:
 rcall wait1s		;1s	12+x return addresses
;proceed with wait4s	;+4s

wait4s:
 rcall wait1s		;1s	12+x return addresses
;proceed with wait3s	;+3s

wait3s:
 rcall wait1s		;1s	12+x return addresses
;proceed with wait2s	;+2s

wait2s:
 rcall wait1s		;1s     12+x return addresses
;proceed with wait1s	;+1s

wait1s:
wait1000ms:	
 rcall wait500ms	;500ms	11+x return addresses
;proceed with wait500ms	;+500ms	

wait500ms:
 rcall wait100ms	;100ms	10+x return addresses
;proceed with wait400ms	;+400ms

wait400ms:
 rcall wait100ms	;100ms	10+x return addresses
;proceed with wait300ms	;+300ms

wait300ms:
 rcall wait100ms        ;100ms	10+x return addresses
;proceed with wait200ms	;+200ms

wait200ms:
 rcall wait100ms	;100ms	10+x return addresses
;proceed with wait100ms ;+100ms

wait100ms:
 wdr			;reset watchdog every 100ms!
 rcall wait50ms	    	;50ms	9+x return addresses
;proceed with wait50ms 	;+50ms

wait50ms:
 rcall wait10ms	    	;10ms	8+x return addresses 
;proceed with wait40ms	;+40ms

wait40ms:
 rcall wait10ms	    	;10ms	8+x return addresses
;proceed with wait30us 	;+30ms

wait30ms:
 rcall wait10ms	    	;10ms	8+x return addresses
;proceed with wait20us 	;+20ms

wait20ms:
 rcall wait10ms	    	;10ms	8+x return addresses
;proceed with wait10ms	;+10ms

wait10ms:
 rcall wait5ms	   	;5ms	7+x return addresses
;proceed with wait5ms	;+5ms

wait5ms:
 rcall wait1ms	   	;1ms	6+x return addresses
;proceed with wait4ms	;+4ms

wait4ms:
 rcall wait1ms	   	;1ms	6+x return addresses
;proceed with wait3ms	;+3ms	

wait3ms:
 rcall wait1ms	   	;1ms	6+x return addresses
;proceed with wait2ms	;+2ms

wait2ms:
 rcall wait1ms	   	;1ms	6+x return addresses
;proceed with wait1ms	;+1ms

wait1ms:
 rcall wait500us    	;500탎	5+x return addresses
;laeuft in wait500us	;+500탎

wait500us:
 rcall wait100us    	;100탎	4+x return addresses
;proceed with wait400us	;+400탎

wait400us:
 rcall wait100us    	;100탎	4+x return addresses
;proceed with wait300us	;+300탎

wait300us:
 rcall wait100us    	;100탎	4+x return addresses
;proceed with wait200us	;+200탎

wait200us:
 rcall wait100us    	;100탎	4+x return addresses
;proceed with wait100us	;+100탎	

wait100us:
 rcall wait50us	    	;50탎	3+x return addresses
;proceed with wait50us	;+50탎	

wait50us:
 rcall wait10us     	;10탎	2+x return addresses
;proceed with wait40us	;+40탎

wait40us:
 rcall wait10us     	;10탎	2+x return addresses
;proceed with wait30us	;+30탎

wait30us:
 rcall wait10us     	;10탎	2+x return addresses
;proceed with wait20us	;+20탎

wait20us:
 rcall wait10us		;10탎	2+x return addresses
;proceed with wait10us	;+10탎


/*
 *  10탎 and below based on MCU clock rate
 */

wait10us:		;	1+x return addresses

#if CPU_FREQ == 1000000
;1MHz version			x = 0
;rcall needs 3 clock cycles for ATmega8L
 nop			;if call needs 4 clock cycles, remove one nop!
 nop
 nop
;ret needs 4 clock cycles (1탎 each)
#endif


#if CPU_FREQ == 2000000
;2MHz version			x = 1
 rcall wait5us		;5s
;proceed with wait5us	;+5�

.global wait5us		;wait 5탎
wait5us:
 nop
 nop
 nop
#endif


#if CPU_FREQ == 4000000
;4MHz version			x = 2
.global wait5us		;wait 5탎
.global wait4us		;wait 4탎
.global wait2us		;wait 2탎
 rcall wait5us		;5탎
;proceed with wait5us	;+5탎

wait5us:
 nop
 nop
 nop
 nop			;1탎
;proceed with wait4us	;+4탎

wait4us:
rcall wait2us		;2탎
;proceed with wait2us	;+2탎

wait2us:
 nop			;2탎
#endif


#if CPU_FREQ == 8000000
;8MHz Version			x = 2
.global wait5us		;wait 5탎
.global wait4us		;wait 4탎
.global wait3us		;wait 3탎
.global wait2us		;wait 2탎
.global wait1us		;wait 1탎
 rcall wait5us		;5탎
;proceed with wait5us	;+5탎

wait5us:
 rcall wait1us		;1탎
;proceed with wait4us	;+4탎

wait4us:
 rcall wait1us		;1탎
;proceed with wait3us	;+3us

wait3us:
 rcall wait1us		;1탎
;proceed with wait2us	;+2탎

wait2us:
 rcall wait1us		;1탎
;proceed with wait1us	;+1탎

wait1us:
 nop			;1탎
#endif


#if CPU_FREQ == 16000000
;16MHz Version			x = 2
.global wait5us		;wait 5탎
.global wait4us		;wait 4탎
.global wait3us		;wait 3탎
.global wait2us		;wait 2탎
.global wait1us		;wait 1탎
 rcall wait5us		;5탎
;proceed with wait5us	;+5탎

wait5us:
 rcall wait1us		;1탎
;proceed with wait4us	;+4탎

wait4us:
 rcall wait1us		;1탎
;proceed with wait3us	;+3탎

wait3us:
 rcall wait1us		;1탎
;proceed with wait1us	;+2탎

wait2us:
 rcall wait1us		;1탎
;proceed with wait1us	;+1탎

wait1us:
 rcall wait500ns	;500ns
;proceed with wait500ms	;+500ns

wait500ns:
 nop			;500ns
#endif


 ret



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */

/* source management */
#undef WAIT_S



/* ************************************************************************
 *   EOF
 * ************************************************************************ */
