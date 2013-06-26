//============================================================
// File Name: Type_def.h
//
// Description:
//
//============================================================


#ifndef	__TYPE_DEF_H__
#define __TYPE_DEF_H__

typedef void				VOID,*PVOID;
typedef unsigned char		BOOLEAN,*PBOOLEAN;

typedef unsigned char		u1Byte,*pu1Byte;
typedef unsigned short		u2Byte,*pu2Byte;
typedef unsigned long		u4Byte,*pu4Byte;
typedef unsigned long long	u8Byte,*pu8Byte;

#if 1
/* In ARM platform, system would use the type -- "char" as "unsigned char"
 * And we only use s1Byte/ps1Byte as INT8 now, so changes the type of s1Byte/ps1Byte.*/
    typedef signed char		s1Byte,*ps1Byte;
#else
typedef char				s1Byte,*ps1Byte;
#endif
typedef short				s2Byte,*ps2Byte;
typedef long				s4Byte,*ps4Byte;
typedef long long			s8Byte,*ps8Byte;

typedef unsigned long		UINT32,*pUINT32;
typedef unsigned char		UINT8;
typedef unsigned short		UINT16;
typedef signed char			INT8;
typedef signed short		INT16;
typedef signed long			INT32;
typedef unsigned int		UINT;
typedef signed int			INT;
typedef unsigned long long	UINT64;
typedef signed long long	INT64;


#ifndef BIT0
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
#define BIT32		0x100000000

#endif

//	Example:
//		BIT_LEN_MASK_32(0) => 0x00000000
//		BIT_LEN_MASK_32(1) => 0x00000001
//		BIT_LEN_MASK_32(2) => 0x00000003
//		BIT_LEN_MASK_32(32) => 0xFFFFFFFF
//
#define BIT_LEN_MASK_32(__BitLen) \
	(0xFFFFFFFF >> (32 - (__BitLen)))

#define BIT_LEN_MASK_8(__BitLen) \
		(0xFF >> (8 - (__BitLen)))

//
// Byte Swapping routine.
//
#define EF1Byte	
#define EF2Byte 	le16_to_cpu
#define EF4Byte		le32_to_cpu

#define LE_P1BYTE_TO_HOST_1BYTE(__pStart) \
	(EF1Byte(*((u1Byte *)(__pStart))))

//
//	Description:
//		Return 4-byte value in host byte ordering from
//		4-byte pointer in litten-endian system.
//
#define LE_P4BYTE_TO_HOST_4BYTE(__pStart) \
	(EF4Byte(*((u4Byte *)(__pStart))))
	

//
//	Description:
//		Translate subfield (continuous bits in little-endian) of 4-byte value in litten byte to
//		4-byte value in host byte ordering.
//
#define LE_BITS_TO_4BYTE(__pStart, __BitOffset, __BitLen) \
	( \
		( LE_P4BYTE_TO_HOST_4BYTE(__pStart) >> (__BitOffset) ) \
		& \
		BIT_LEN_MASK_32(__BitLen) \
	)

#define LE_BITS_TO_1BYTE(__pStart, __BitOffset, __BitLen) \
	( \
		( LE_P1BYTE_TO_HOST_1BYTE(__pStart) >> (__BitOffset) ) \
		& \
		BIT_LEN_MASK_8(__BitLen) \
	)

//
// TX report 2 format in Rx desc
//
#define GET_TX_RPT2_DESC_PKT_LEN_88E(__pRxStatusDesc)				LE_BITS_TO_4BYTE( __pRxStatusDesc, 0, 9)
#define GET_TX_RPT2_DESC_MACID_VALID_1_88E(__pRxStatusDesc) 	LE_BITS_TO_4BYTE( __pRxStatusDesc+16, 0, 32)
#define GET_TX_RPT2_DESC_MACID_VALID_2_88E(__pRxStatusDesc) 	LE_BITS_TO_4BYTE( __pRxStatusDesc+20, 0, 32)

#define GET_TX_REPORT_TYPE1_RERTY_0(__pAddr)						LE_BITS_TO_4BYTE( __pAddr, 0, 16)
#define GET_TX_REPORT_TYPE1_RERTY_1(__pAddr)						LE_BITS_TO_1BYTE( __pAddr+2, 0, 8)
#define GET_TX_REPORT_TYPE1_RERTY_2(__pAddr)						LE_BITS_TO_1BYTE( __pAddr+3, 0, 8)
#define GET_TX_REPORT_TYPE1_RERTY_3(__pAddr)						LE_BITS_TO_1BYTE( __pAddr+4, 0, 8)
#define GET_TX_REPORT_TYPE1_RERTY_4(__pAddr)						LE_BITS_TO_1BYTE( __pAddr+4+1, 0, 8)
#define GET_TX_REPORT_TYPE1_DROP_0(__pAddr) 					LE_BITS_TO_1BYTE( __pAddr+4+2, 0, 8)
#define GET_TX_REPORT_TYPE1_DROP_1(__pAddr) 					LE_BITS_TO_1BYTE( __pAddr+4+3, 0, 8)

#endif
