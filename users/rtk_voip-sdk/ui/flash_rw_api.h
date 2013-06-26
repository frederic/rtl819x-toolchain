#ifndef __FLASH_RW_API_H__
#define __FLASH_RW_API_H__

#include "flash_layout.h"

/* Initialize flash r/w interface */
extern void InitializeFlash( void );

/* Write to flash */
extern void FlashWriteData( unsigned long addr, const void *pData, unsigned int len );
extern void FlashWriteOneByte( unsigned long addr, unsigned char data );
extern void FlashWriteTwoBytes( unsigned long addr, unsigned short data );
extern void FlashWriteFourBytes( unsigned long addr, unsigned long data );

/* Validate writing */
extern void FlashValidateWriting( unsigned int bImmediately );

/* Read from flash */
extern void FlashReadData( unsigned long addr, void *pData, unsigned int len );
extern unsigned char FlashReadOneByte( unsigned long addr );
extern unsigned short FlashReadTwoBytes( unsigned long addr );
extern unsigned long FlashReadFourBytes( unsigned long addr );

/* Terminate flash r/w interface */
extern void TerminateFlash( void );

#ifndef MY_FIELD_OFFSET
#define MY_FIELD_OFFSET( type, field )		( ( unsigned long )&( ( ( type * )0 ) ->field ) )
#endif
#ifndef MY_FIELD_SIZE
#define MY_FIELD_SIZE( type, field )		( sizeof( ( ( type * )0 ) ->field ) )
#endif

#endif /* __FLASH_RW_API_H__ */
