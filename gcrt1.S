/* This code has been taken from avr-libc 2.0.0.
   Then it has been modified to build shorter .vectors section.
   By Viktor Klimkovich */

/* Copyright (c) 2002, Marek Michalkiewicz <marekm@amelek.gda.pl>
   Copyright (c) 2007, 2008 Eric B. Weddington
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE. */

#if (__GNUC__ < 3) || (__GNUC__ == 3 && __GNUC_MINOR__ < 3)
#error "GCC version >= 3.3 required"
#endif

#include <avr/io.h>

#define STR(x)   _STR(x)
#define _STR(x)  #x

#if !defined(__zero_reg__)
    #if defined(__AVR_TINY__)
        #define __zero_reg__ r17
    #else
        #define __zero_reg__ r1
    #endif
#endif

#if (__AVR_HAVE_JMP_CALL__)
  #define XJMP jmp
  #define XCALL call
#else
  #define XJMP rjmp
  #define XCALL rcall
#endif

#ifdef LGT8F328P
  #define PROGMEM_SIZE (30*1024)
  #define EEPROM_SIZE  (1*1024 - 2)
#endif


/* load value for last_used_vector from temporary generated file */
#include "gcrt1.inc"

.macro vector i
  .if (. - __vectors <= last_used_vector*4)
    .weak __vector_\i
    /* fix for avr-gcc 8.3.0 */
    //.set __vector_\i, __bad_interrupt
    XJMP __vector_\i
  .endif
.endm

.section .vectors,"ax",@progbits
  .global __vectors
  .func __vectors
__vectors:
    XJMP __init  // __vectors[0]
    .altmacro
    .set i,1
    .rept 127   // __vectors[1..127]
      vector %i
      .set i,i+1
    .endr
  .endfunc


	/* Handle unexpected interrupts (enabled and no handler), which
	   usually indicate a bug.  Jump to the __vector_default function
	   if defined by the user, otherwise jump to the reset address.

	   This must be in a different section, otherwise the assembler
	   will resolve "rjmp" offsets and there will be no relocs.  */

	.text
	.global	__bad_interrupt
	.func	__bad_interrupt
__bad_interrupt:
	.weak	__vector_default
	.set	__vector_default, __vectors
	XJMP	__vector_default
	.endfunc

	.section .init0,"ax",@progbits
	.weak	__init
;	.func	__init
__init:

#ifndef __AVR_ASM_ONLY__
	.weak	__stack

	/* By default, malloc() uses the current value of the stack pointer
	   minus __malloc_margin as the highest available address.

	   In some applications with external SRAM, the stack can be below
	   the data section (in the internal SRAM - faster), and __heap_end
	   should be set to the highest address available for malloc().  */
	.weak	__heap_end
	.set	__heap_end, 0

	.section .init2,"ax",@progbits
	clr	__zero_reg__
	out	AVR_STATUS_ADDR, __zero_reg__
	ldi	r28,lo8(__stack)
#ifdef __AVR_XMEGA__
	out	AVR_STACK_POINTER_LO_ADDR, r28
#ifdef _HAVE_AVR_STACK_POINTER_HI
	ldi	r29,hi8(__stack)
	out	AVR_STACK_POINTER_HI_ADDR, r29
#endif	/* _HAVE_AVR_STACK_POINTER_HI */
#else
#ifdef _HAVE_AVR_STACK_POINTER_HI
	ldi	r29,hi8(__stack)
	out	AVR_STACK_POINTER_HI_ADDR, r29
#endif	/* _HAVE_AVR_STACK_POINTER_HI */
	out	AVR_STACK_POINTER_LO_ADDR, r28
#endif  /* __AVR_XMEGA__ */

#ifdef __AVR_3_BYTE_PC__
	ldi	r16, hh8(pm(__vectors))
	out	_SFR_IO_ADDR(EIND), r16
#endif	/* __AVR_3_BYTE_PC__ */

#ifdef __AVR_HAVE_RAMPD__
	out	AVR_RAMPD_ADDR, __zero_reg__
	out	AVR_RAMPX_ADDR, __zero_reg__
	out	AVR_RAMPY_ADDR, __zero_reg__
	out	AVR_RAMPZ_ADDR, __zero_reg__
#endif

#if defined(__GNUC__) && ((__GNUC__ <= 3) || (__GNUC__ == 4 && __GNUC_MINOR__ <= 3))
#if BIG_CODE
	/* Only for >64K devices with RAMPZ, replaces the default code
	   provided by libgcc.S which is only linked in if necessary.  */

	.section .init4,"ax",@progbits
	.global __do_copy_data
__do_copy_data:
	ldi	r17, hi8(__data_end)
	ldi	r26, lo8(__data_start)
	ldi	r27, hi8(__data_start)
	ldi	r30, lo8(__data_load_start)
	ldi	r31, hi8(__data_load_start)

	/* On the enhanced core, "elpm" with post-increment updates RAMPZ
	   automatically.  Otherwise we have to handle it ourselves.  */

#ifdef __AVR_ENHANCED__
	ldi	r16, hh8(__data_load_start)
#else
	ldi	r16, hh8(__data_load_start - 0x10000)
.L__do_copy_data_carry:
	inc	r16
#endif
	out	AVR_RAMPZ_ADDR, r16
	rjmp	.L__do_copy_data_start
.L__do_copy_data_loop:
#ifdef __AVR_ENHANCED__
	elpm	r0, Z+
#else
	elpm
#endif
	st	X+, r0
#ifndef __AVR_ENHANCED__
	adiw	r30, 1
	brcs	.L__do_copy_data_carry
#endif
.L__do_copy_data_start:
	cpi	r26, lo8(__data_end)
	cpc	r27, r17
	brne	.L__do_copy_data_loop
#ifdef __AVR_HAVE_RAMPD__
	out	AVR_RAMPZ_ADDR, __zero_reg__
#endif /* __AVR_HAVE_RAMPD__*/

#endif /* BIG_CODE */
#endif /* defined(__GNUC__) && ((__GNUC__ <= 3) || (__GNUC__ == 4 && __GNUC_MINOR__ <= 3)) */

	.set	__stack, RAMEND
#endif /* !__AVR_ASM_ONLY__ */

	.section .init9,"ax",@progbits
#ifdef __AVR_ASM_ONLY__
	XJMP	main
#else	/* !__AVR_ASM_ONLY__ */
	XCALL	main
	XJMP	exit
#endif	/* __AVR_ASM_ONLY__ */
;	.endfunc

    .section .note.gnu.avr.deviceinfo, "", @note
#define NOTE_NAME "AVR"
#ifdef __AVR_DEVICE_NAME__
    #define DEVICE_NAME STR(__AVR_DEVICE_NAME__)
#else
    #define DEVICE_NAME ""
#endif

    .long .L__note_name_end - .L__note_name_start
    .long .L__desc_end - .L__desc_start
    .long 1 ; Type 1 - this is the only known note type for AVR.
.L__note_name_start:
    .asciz NOTE_NAME
    .balign 4
.L__note_name_end:
.L__desc_start:
#ifdef FLASHSTART
    .long FLASHSTART
#else
    .long 0
#endif
#ifdef PROGMEM_SIZE
    .long PROGMEM_SIZE
#elif !defined (FLASHEND)
    .long 0
#elif FLASHEND > 0
    .long FLASHEND + 1
#else
    .long FLASHEND
#endif
#ifdef RAMSTART
    .long RAMSTART
#else
    .long 0
#endif
#ifdef RAMSIZE
    .long RAMSIZE
#elif !defined (RAMEND) || !defined (RAMSTART)
    .long 0
#elif RAMEND > 0
    .long RAMEND - RAMSTART + 1
#else
    .long RAMEND
#endif
#ifdef E2START
    .long E2START
#else
    .long 0
#endif
#ifdef EEPROM_SIZE
    .long EEPROM_SIZE
#elif !defined (E2END)
    .long 0
#elif E2END > 0
    .long E2END + 1
#else
    .long E2END
#endif
    /* String offsets table.
    Index 0 - Size of offset table in bytes
    Index 1 - Device name byte offset
    */
.L__stroffsettab_start:
.long .L__stroffsettab_end - .L__stroffsettab_start /* Size of index table in bytes */
.long .L__device_name_start - .L__strtab_start /* Byte offset of device name */
.L__stroffsettab_end:
    /* String table for storing arbitrary strings.
    Offsets are stored in the string offset table above */
.L__strtab_start:
    .byte 0
.L__device_name_start:
    .asciz DEVICE_NAME
.L__device_name_end:
    .byte 0
.L__strtab_end:
.L__desc_end:
    .balign 4

  /* Set TEXT_REGION_ORIGIN symbol. */
#ifdef FLASHSTART
  .weak __TEXT_REGION_ORIGIN__
  .set __TEXT_REGION_ORIGIN__, FLASHSTART
#endif

  /* Set DATA_REGION_ORIGIN symbol. */
#ifdef RAMSTART
  .weak __DATA_REGION_ORIGIN__
  .set __DATA_REGION_ORIGIN__, 0x800000 | RAMSTART
#endif

  /* Set REGION_LENGTH symbol values for well known memory regions.
  The default linker script uses these symbols to set MEMORY 
  region lengths, and by defining these here, the linker can detect
  memory overflows accurately on a per device basis, since the 
  values are picked up from the device header file.
  */

  .weak __FUSE_REGION_LENGTH__
  .set __FUSE_REGION_LENGTH__, FUSE_MEMORY_SIZE

#ifdef PROGMEM_SIZE
  .weak __TEXT_REGION_LENGTH__
  .set __TEXT_REGION_LENGTH__, PROGMEM_SIZE
#elif defined(FLASHEND) && defined(FLASHSTART) && FLASHEND > 0
  .weak __TEXT_REGION_LENGTH__
  .set __TEXT_REGION_LENGTH__, FLASHEND - FLASHSTART + 1
#elif defined(FLASHEND) && FLASHEND > 0
  .weak __TEXT_REGION_LENGTH__
  .set __TEXT_REGION_LENGTH__, FLASHEND + 1
#endif

#ifdef RAMSIZE
  .weak __DATA_REGION_LENGTH__
  .set __DATA_REGION_LENGTH__, RAMSIZE
#elif defined (RAMEND) && defined(RAMSTART) && RAMEND > 0
  .weak __DATA_REGION_LENGTH__
  .set 	__DATA_REGION_LENGTH__, RAMEND - RAMSTART + 1
#elif defined (RAMEND) && RAMEND > 0
  .weak __DATA_REGION_LENGTH__
  .set 	__DATA_REGION_LENGTH__, RAMEND + 1
#endif

#ifdef EEPROM_SIZE
  .weak __EEPROM_REGION_LENGTH__
  .set __EEPROM_REGION_LENGTH__, EEPROM_SIZE
#elif  defined(E2END) && E2END > 0
  .weak __EEPROM_REGION_LENGTH__
  .set __EEPROM_REGION_LENGTH__, E2END + 1
#endif
