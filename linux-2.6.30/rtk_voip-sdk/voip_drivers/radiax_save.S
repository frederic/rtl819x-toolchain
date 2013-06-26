#include "mips.h"
#include "radiax.h"



/*
Word32 save_radiax_s(pt_radiax reg) 
*/
	.section ".text"
	.set	noreorder
	.ent	save_radiax_s
	.globl	save_radiax_s
save_radiax_s:

	mfa	v0, m0l
	swp	v0, (a0)+4
	mfa	v0, m0l, 8
	swp	v0, (a0)+4
	mfa	v0, m0h
	swp	v0, (a0)+4
	mfa	v0, m0h, 8
	swp	v0, (a0)+4

	mfa	v0, m1l
	swp	v0, (a0)+4
	mfa	v0, m1l, 8
	swp	v0, (a0)+4	
	mfa	v0, m1h
	swp	v0, (a0)+4
	mfa	v0, m1h, 8
	swp	v0, (a0)+4


	mfa	v0, m2l
	swp	v0, (a0)+4
	mfa	v0, m2l, 8
	swp	v0, (a0)+4
	mfa	v0, m2h
	swp	v0, (a0)+4
	mfa	v0, m2h, 8
	swp	v0, (a0)+4
	

	mfa	v0, m3l
	swp	v0, (a0)+4
	mfa	v0, m3l, 8
	swp	v0, (a0)+4	
	mfa	v0, m3h
	swp	v0, (a0)+4
	mfa	v0, m3h, 8
	swp	v0, (a0)+4

	mfru v0, mmd
	swp	v0, (a0)+4

	mfru v0, lps0
	swp	v0, (a0)+4
	mfru v0, lpe0
	swp	v0, (a0)+4
	mfru v0, lpc0
	swp	v0, (a0)+4
	

	
	jr		ra
	nop
	.set	reorder
	.end	save_radiax_s



/*
Word32 restore_radiax_s(pt_radiax reg) 
*/
	.section ".text"
	.set	noreorder
	.ent	restore_radiax_s
	.globl	restore_radiax_s

restore_radiax_s:

	lwp		v0, (a0)+4
	nop
	mta2	v0, m0l
	lwp		v0, (a0)+4
	nop	
	mta2.g	v0, m0l
	lwp		v0, (a0)+4
	nop
	mta2	v0, m0h
	lwp		v0, (a0)+4
	nop	
	mta2.g	v0, m0h
	

	lwp		v0, (a0)+4
	nop
	mta2	v0, m1l
	lwp		v0, (a0)+4
	nop	
	mta2.g	v0, m1l
	lwp		v0, (a0)+4
	nop	
	mta2	v0, m1h
	lwp		v0, (a0)+4
	nop	
	mta2.g	v0, m1h


	lwp		v0, (a0)+4
	nop	
	mta2	v0, m2l
	lwp		v0, (a0)+4
	nop	
	mta2.g	v0, m2l
	lwp		v0, (a0)+4
	nop	
	mta2	v0, m2h
	lwp		v0, (a0)+4
	nop	
	mta2.g	v0, m2h
	

	lwp		v0, (a0)+4
	nop	
	mta2	v0, m3l
	lwp		v0, (a0)+4
	nop	
	mta2.g	v0, m3l
	lwp		v0, (a0)+4
	nop	
	mta2	v0, m3h
	lwp		v0, (a0)+4
	nop	
	mta2.g	v0, m3h

	lwp	v0, (a0)+4
	nop	
	mtru v0, mmd

	lwp	v0, (a0)+4
	nop	
	mtru v0, lps0
	
	lwp	v0, (a0)+4
	nop	
	mtru v0, lpe0

	lwp	v0, (a0)+4	
	nop	
	mtru v0, lpc0

	
	jr		ra
	nop
	.set	reorder
	.end	restore_radiax_s
	
