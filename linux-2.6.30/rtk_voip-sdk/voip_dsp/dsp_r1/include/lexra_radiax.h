


#ifndef _LEXRA_RADIAX_H
#define _LEXRA_RADIAX_H

/*
 * The following macros are especially useful for __asm__
 * inline assembler.
 */
#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

/*
 * MAC accumulator register names
 */

#define MAC_M0L	$1		/* ma0l */
#define MAC_M0H	$2		/* ma0h */
#define MAC_M0	$3		/* ma0  */

#define MAC_M1L	$5		/* ma1l */
#define MAC_M1H	$6		/* ma1h */
#define MAC_M1	$7		/* ma1  */

#define MAC_M2L	$9		/* ma2l */
#define MAC_M2H	$10		/* ma2h */
#define MAC_M2	$11		/* ma2  */

#define MAC_M3L	$13		/* ma3l */
#define MAC_M3H	$14		/* ma3h */
#define MAC_M3	$15		/* ma3  */


/*
** These are the registers used with the MTRU/MFRU instructions.
** user radiax register names.
*/

/* Circular buffer registers */

#define RADIAX_CBS0	$0		/* cs0 */
#define RADIAX_CBS1	$1		/* cs1 */
#define RADIAX_CBS2	$2		/* cs2 */
#define RADIAX_CBE0	$4		/* ce0 */
#define RADIAX_CBE1	$5		/* ce1 */
#define RADIAX_CBE2	$6		/* ce2 */

/* Zero Overhead Loop Control registers */

#define RADIAX_LPS0	$16		/* ls0 */
#define RADIAX_LPE0	$17		/* le0 */
#define RADIAX_LPC0	$18		/* lc0 */

/* MAC Mode Register */

#define RADIAX_MMD	$24		/* md */

#define SET_MMD_MT	0x1		/* MAC 32*32 truncate mode */
#define SET_MMD_MS	0x2		/* MAC 32 bit saturate mode */
#define SET_MMD_MF	0x4		/* MAC fractional mode */

#define mac_multa_32bit(register,value1, value2)			\
        __asm__ __volatile__(                                   \
        "multa\t"STR(register)",%0,%1\n\t"			\
        : : "r" (value1), "r" (value2));

#define mac_multa2_16bit(register,value1, value2)		\
        __asm__ __volatile__(                                   \
        "multa2\t"STR(register)",%0,%1\n\t"			\
        : : "r" (value1), "r" (value2));

#define read_32bit_mac_register(source)                         \
({ int __res;                                                   \
        __asm__ __volatile__(                                   \
	".set\tpush\n\t"					\
	".set\treorder\n\t"					\
        "mfa\t%0,"STR(source)"\n\t"                             \
	".set\tpop"						\
        : "=r" (__res));                                        \
        __res;})

#define write_32bit_mac_register(register,value)                \
        __asm__ __volatile__(                                   \
        "mta2\t%0,"STR(register)"\n\t"				\
	"nop"							\
        : : "r" (value));


#define read_32bit_radiax_register(source)                      \
({ int __res;                                                   \
        __asm__ __volatile__(                                   \
	".set\tpush\n\t"					\
	".set\treorder\n\t"					\
        "mfru\t%0,"STR(source)"\n\t"                            \
	".set\tpop"						\
        : "=r" (__res));                                        \
        __res;})

#define write_32bit_radiax_register(register,value)             \
        __asm__ __volatile__(                                   \
        "mtru\t%0,"STR(register)"\n\t"				\
	"nop"							\
        : : "r" (value));


#endif /* _LEXRA_RADIAX_H */



