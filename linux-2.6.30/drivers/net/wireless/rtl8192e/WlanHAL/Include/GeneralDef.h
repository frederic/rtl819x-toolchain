#ifndef __INC_GENERALDEF_H
#define __INC_GENERALDEF_H

/*
 *	Note:	1.Only pure value definition can be put here.
 *			2.The definition here should be hardware and platform independent.
 *
*/

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

// Two Marco below are used to this case :"IN" & "OUT" are used to other purpose
#ifndef INPUT
#define INPUT
#endif

#ifndef OUTPUT
#define OUTPUT
#endif



#ifndef TRUE
    #define TRUE                1
#endif
#ifndef FALSE	
    #define FALSE               0
#endif

#ifndef _TRUE
    #define _TRUE               1
#endif
#ifndef _FALSE	
    #define _FALSE              0
#endif

#ifndef BIT
#define BIT(x)		(1 << (x))
#endif


#define BIT0		0x00000001
#define BIT1		0x00000002
#define BIT2		0x00000004
#define BIT3		0x00000008
#define BIT4		0x00000010
#define BIT5		0x00000020
#define BIT6		0x00000040
#define BIT7		0x00000080
#define BIT8		0x00000100
#define BIT9		0x00000200
#define BIT10		0x00000400								
#define BIT11		0x00000800								
#define BIT12		0x00001000								
#define BIT13		0x00002000								
#define BIT14		0x00004000								
#define BIT15		0x00008000								
#define BIT16		0x00010000								
#define BIT17		0x00020000								
#define BIT18		0x00040000								
#define BIT19		0x00080000								
#define BIT20		0x00100000								
#define BIT21		0x00200000								
#define BIT22		0x00400000								
#define BIT23		0x00800000								
#define BIT24		0x01000000								
#define BIT25		0x02000000								
#define BIT26		0x04000000								
#define BIT27		0x08000000								
#define BIT28		0x10000000								
#define BIT29		0x20000000								
#define BIT30		0x40000000								
#define BIT31		0x80000000								

#define BIT32	UINT64_C(0x0000000100000000)
#define BIT33	UINT64_C(0x0000000200000000)
#define BIT34	UINT64_C(0x0000000400000000)
#define BIT35	UINT64_C(0x0000000800000000)
#define BIT36	UINT64_C(0x0000001000000000)
#define BIT37	UINT64_C(0x0000002000000000)
#define BIT38	UINT64_C(0x0000004000000000)
#define BIT39	UINT64_C(0x0000008000000000)
#define BIT40	UINT64_C(0x0000010000000000)
#define BIT41	UINT64_C(0x0000020000000000)
#define BIT42	UINT64_C(0x0000040000000000)
#define BIT43	UINT64_C(0x0000080000000000)
#define BIT44	UINT64_C(0x0000100000000000)
#define BIT45	UINT64_C(0x0000200000000000)
#define BIT46	UINT64_C(0x0000400000000000)
#define BIT47	UINT64_C(0x0000800000000000)
#define BIT48	UINT64_C(0x0001000000000000)
#define BIT49	UINT64_C(0x0002000000000000)
#define BIT50	UINT64_C(0x0004000000000000)
#define BIT51	UINT64_C(0x0008000000000000)
#define BIT52	UINT64_C(0x0010000000000000)
#define BIT53	UINT64_C(0x0020000000000000)
#define BIT54	UINT64_C(0x0040000000000000)
#define BIT55	UINT64_C(0x0080000000000000)
#define BIT56	UINT64_C(0x0100000000000000)
#define BIT57	UINT64_C(0x0200000000000000)
#define BIT58	UINT64_C(0x0400000000000000)
#define BIT59	UINT64_C(0x0800000000000000)
#define BIT60	UINT64_C(0x1000000000000000)
#define BIT61	UINT64_C(0x2000000000000000)
#define BIT62	UINT64_C(0x4000000000000000)
#define BIT63	UINT64_C(0x8000000000000000)


//-----------------------------------------------------------------------------------------
// Use one or multiple of the following value to define the flag, HAL_CODE_BASE.
//-----------------------------------------------------------------------------------------

//Element
#define RTL8881AEM      BIT0
#define RTL8192EE       BIT1
#define RTL8192EU       BIT2

//Chip Collection
#define RTL8881A        (RTL8881AEM)
#define RTL8192E        (RTL8192EE| RTL8192EU)

//HAL Common Layer
#define RTL88XX         (RTL8881A | RTL8192E)

//Marco Utility
#define IS_ONLY_RTL8881AEM              ((HAL_CODE_BASE == RTL8881AEM) ? _TRUE : _FALSE)
#define IS_ONLY_RTL8192EE               ((HAL_CODE_BASE == RTL8192EE) ? _TRUE : _FALSE)
#define IS_ONLY_RTL8192EU               ((HAL_CODE_BASE == RTL8192EU) ? _TRUE : _FALSE)
#define IS_ONLY_RTL8881AEM_RTL8192EE    ((HAL_CODE_BASE == (RTL8881AEM|RTL8192EE)) ? _TRUE : _FALSE)

//Chip Existence
#define IS_EXIST_RTL8881AEM                         ((HAL_CODE_BASE & RTL8881AEM) ? _TRUE : _FALSE)
#define IS_EXIST_RTL8192EE                          ((HAL_CODE_BASE & RTL8192EE) ? _TRUE : _FALSE)
#define IS_EXIST_RTL8192EU                          ((HAL_CODE_BASE & RTL8192EU) ? _TRUE : _FALSE)
#define IS_EXIST_RTL8881AEM_RTL8192EE               (IS_EXIST_RTL8881AEM && IS_EXIST_RTL8192EE)

//Chip Series
#define IS_RTL8192E_SERIES              ((HAL_CODE_BASE & RTL8192E) ? _TRUE : _FALSE)
#define IS_RTL8881A_SERIES              ((HAL_CODE_BASE & RTL8881A) ? _TRUE : _FALSE)

//Chip Generation
#define IS_RTL88XX_GENERATION           ((HAL_CODE_BASE & RTL88XX) ? _TRUE : _FALSE)



//-----------------------------------------------------------------------------------------
// Use one or multiple of the following value to define the flag, DEV_BUS_TYPE.
// *** This Setting is necessary to be independent with Chip  ***
//-----------------------------------------------------------------------------------------
#define RT_EMBEDDED_INTERFACE           BIT0
#define RT_PCI_INTERFACE				BIT1
#define RT_USB_INTERFACE				BIT2
#define RT_SDIO_INTERFACE				BIT3

//Marco Utility
#define IS_ONLY_EMBEDDED              ((DEV_BUS_TYPE == RT_EMBEDDED_INTERFACE) ? _TRUE : _FALSE)
#define IS_ONLY_PCI                           ((DEV_BUS_TYPE == RT_PCI_INTERFACE) ? _TRUE : _FALSE)
#define IS_ONLY_USB                         ((DEV_BUS_TYPE == RT_USB_INTERFACE) ? _TRUE : _FALSE)
#define IS_ONLY_SDIO                        ((DEV_BUS_TYPE == RT_SDIO_INTERFACE) ? _TRUE : _FALSE)
#define IS_ONLY_EMBEDDED_PCI       ((DEV_BUS_TYPE == (RT_EMBEDDED_INTERFACE|RT_PCI_INTERFACE)) ? _TRUE : _FALSE)

#define IS_EXIST_EMBEDDED              ((DEV_BUS_TYPE & RT_EMBEDDED_INTERFACE) ? _TRUE : _FALSE)
#define IS_EXIST_PCI                           ((DEV_BUS_TYPE & RT_PCI_INTERFACE) ? _TRUE : _FALSE)
#define IS_EXIST_USB                         ((DEV_BUS_TYPE & RT_USB_INTERFACE) ? _TRUE : _FALSE)
#define IS_EXIST_SDIO                        ((DEV_BUS_TYPE & RT_SDIO_INTERFACE) ? _TRUE : _FALSE)
#define IS_EXIST_EMBEDDED_PCI        (IS_EXIST_EMBEDDED && IS_EXIST_PCI)





#endif // #ifndef __INC_GENERALDEF_H
