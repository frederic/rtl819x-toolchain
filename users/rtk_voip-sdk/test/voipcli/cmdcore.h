#ifndef __CMD_CORE_H__
#define __CMD_CORE_H__

/* Initialize command core */
extern void InitializeCmdCore( void );

/* Print path for user prompt */
extern void PrintCmdPromptPath( void );

/* Command parser and do command */
extern int CmdCoreParser( const char * const pszcmd );


#endif /* __CMD_CORE_H__ */

