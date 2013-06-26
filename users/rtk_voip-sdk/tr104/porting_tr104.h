#ifndef __PORTING_TR104_H__
#define __PORTING_TR104_H__

/* 
 * There are two additional download types for TR-104: 
 * ¡§4 Tone File¡¨
 * ¡§5 Ringer File¡¨
 * reference: TR-104 APPENDIX B. 
 */
extern int tr104_port_before_download( int file_type, char *target );
extern int tr104_port_after_download( int file_type, char *target );


#endif /* __PORTING_TR104_H__ */

