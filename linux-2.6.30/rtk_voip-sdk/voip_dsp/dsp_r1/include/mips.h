#ifndef CYGONCE_HAL_MIPS_INC
#define CYGONCE_HAL_MIPS_INC

//#define RT8181

/*
##=============================================================================
##
##	mips.inc
##
##	MIPS assembler header file
##
##=============================================================================
#####COPYRIGHTBEGIN####
#                                                                          
# -------------------------------------------                              
# The contents of this file are subject to the Red Hat eCos Public License 
# Version 1.1 (the "License"); you may not use this file except in         
# compliance with the License.  You may obtain a copy of the License at    
# http://www.redhat.com/                                                   
#                                                                          
# Software distributed under the License is distributed on an "AS IS"      
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the 
# License for the specific language governing rights and limitations under 
# the License.                                                             
#                                                                          
# The Original Code is eCos - Embedded Configurable Operating System,      
# released September 30, 1998.                                             
#                                                                          
# The Initial Developer of the Original Code is Red Hat.                   
# Portions created by Red Hat are                                          
# Copyright (C) 1998, 1999, 2000 Red Hat, Inc.                             
# All Rights Reserved.                                                     
# -------------------------------------------                              
#                                                                          
#####COPYRIGHTEND####
##=============================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s): 	nickg
## Contributors:	nickg
## Date:	1997-10-16
## Purpose:	MIPS definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the MIPS CPU family.
## Usage:
##		#include <cyg/hal/mips.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

##-----------------------------------------------------------------------------
## Standard MIPS register names:
*/

#define zero	$0
#define z0	$0
#define at	$1
#define v0	$2
#define v1	$3
#define a0	$4
#define a1	$5
#define a2	$6
#define a3	$7
#define t0	$8
#define t1	$9
#define t2	$10
#define t3	$11
#define t4	$12
#define t5	$13
#define t6	$14
#define t7	$15
#define s0	$16
#define s1	$17
#define s2	$18
#define s3	$19
#define s4	$20
#define s5	$21
#define s6	$22
#define s7	$23
#define s8	$30
#define t8	$24
#define t9	$25
#define k0	$26	/* kernel private register 0 */
#define k1	$27	/* kernel private register 1 */
#define gp	$28	/* global data pointer */
#define sp 	$29	/* stack-pointer */
#define fp	$30	/* frame-pointer */
#define ra	$31	/* return address */
#define pc	$pc	/* pc, used on mips16 */

/*
#define f0	$f0
#define f1	$f1
#define f2	$f2
#define f3	$f3
#define f4	$f4
#define f5	$f5
#define f6	$f6
#define f7	$f7
#define f8	$f8
#define f9	$f9
#define f10	$f10
#define f11	$f11
#define f12	$f12
#define f13	$f13
#define f14	$f14
#define f15	$f15
#define f16	$f16
#define f17	$f17
#define f18	$f18
#define f19	$f19
#define f20	$f20
#define f21	$f21
#define f22	$f22
#define f23	$f23
#define f24	$f24
#define f25	$f25
#define f26	$f26
#define f27	$f27
#define f28	$f28
#define f29	$f29
#define f30	$f30
#define f31	$f31
*/

// Coprocessor 0 registers

//#define	index		$0	// TLB entry index register
//#define random		$1	// TLB random index register
//#define	tlblo0		$2	// TLB even page entry register
//#define	tlblo1		$3	// TLB odd page entry register
//#define config		$3	// Configuration register (TX39 only)
//#define	context		$4	// TLB context register
//#define pagemask	$5	// TLB page size mask
//#define	wired		$6	// TLB wired boundary
#define cachectrl	$7	// Cache control
#define badvr		$8	// Bad virtual address
//#define count		$9	// Timer cycle count register
//#define	tlbhi		$10	// TLB virtual address match register
//#define compare		$11	// Timer compare register
#define status		$12	// Status register	
#define cause		$13	// Exception cause
#define	epc		$14	// Exception pc value
#define prid		$15	// processor ID
//#define config0		$16	// Config register 0

/*
#------------------------------------------------------------------------------

#define FUNC_START(name)	\
        .type name,@function;	\
        .globl name;		\
        .ent   name;		\
name:

#define FUNC_END(name)		\
name##_end:			\
        .end name		\

#------------------------------------------------------------------------------
*/
#endif // ifndef CYGONCE_HAL_MIPS_INC

