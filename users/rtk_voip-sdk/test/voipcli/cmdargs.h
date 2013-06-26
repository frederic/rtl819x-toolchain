#ifndef __CMD_ARGS_H__
#define __CMD_ARGS_H__

/* Use printf to stdout according to arguments buffer and its structure. */
extern void PrintArgsValue( const unsigned char *pArgsBuffer, 
					 const args_t *pArgs );

/* Write value into arguments buffer according to user's input and structure. */
extern void WriteArgsValue( unsigned char *pArgsBuffer, 
					 const args_t *pArgs,
					 const unsigned char *pInput );


#endif /* __CMD_ARGS_H__ */

