#include <stdio.h>
#include <string.h>

#include "cliprint.h"
#include "cmdtree.h"
#include "cmdargs.h"
#include "cmdcore.h"

typedef enum {
	STATE_NODE,
	STATE_LEAF,
	STATE_LEAF_INPUT_ITERATIVE,
	STATE_LEAF_INPUT_ONCE,
} state_t;

#define NODE_DEPTH		4

static state_t sState = STATE_NODE;
static const node_t *pNodePath[ NODE_DEPTH ];
static unsigned int nNodeDepth = 0;
static const node_t *pNodeSelect = nRoot;	/* point to a heap of nodes */
static const args_t *pArgsSelect = NULL;	/* point to a heap of arguments */
static const args_t *pArgsInput = NULL;

void InitializeCmdCore( void )
{
	sState = STATE_NODE;
	nNodeDepth = 0;
	pNodeSelect = nRoot;
	pArgsSelect = pArgsInput = NULL;
}

void PrintCmdPromptPath( void )
{
	unsigned int i;
	
	for( i = 0; i < nNodeDepth; i ++ )
		CliPrintf( ".%s", pNodePath[ i ] ->name );

	if( sState == STATE_LEAF_INPUT_ITERATIVE ||
		sState == STATE_LEAF_INPUT_ONCE ) 
	{
		CliPrintf( ".%s", pArgsInput ->name );
	}
}

static int TreeTravel_EnterArgs( const char * const pszcmd )
{
	const args_t *pArgs = pArgsSelect;
	
	while( pArgs ->name ) {
		
		if( strcmp( pszcmd, pArgs ->name ) == 0 )
			goto label_do_input_arg_once;
		
		pArgs = &pArgs[ 1 ];	/* next argument */
	}
	
	return 0;

label_do_input_arg_once:
	sState = STATE_LEAF_INPUT_ONCE;
	pArgsInput = pArgs;

	return 1;
}

static int TreeTravel_Enter( const char * const pszcmd )
{
	const node_t *pNode = pNodeSelect;
	const args_t * const pArgs = pArgsSelect;
	
	if( pNode == NULL && pArgs )
		return TreeTravel_EnterArgs( pszcmd );

	while( pNode && pNode ->id ) {
		
		if( strcmp( pszcmd, pNode ->name ) == 0 ) 
			goto label_do_enter;
		
		pNode = &pNode[ 1 ];	/* next node */
	}
	
	return 0;
	
label_do_enter:
	switch( pNode ->type ) {
	case NTYPE_INODE:
		pNodePath[ nNodeDepth ++ ] = pNode;
		pNodeSelect = pNode ->next;
		pArgsSelect = pArgsInput = NULL;
		break;

	case NTYPE_LEAF:
		sState = STATE_LEAF;
		pNodePath[ nNodeDepth ++ ] = pNode;
		pNodeSelect = NULL;
		pArgsSelect = pArgsInput = pNode ->args;
		if( pNode ->asize > aBufferSize ) {
			printf( "ERROR!!!!! Arguments buffer is too small!!\n" );
			return 0;
		}
		memset( aBuffer, 0, pNode ->asize );
		break;
		
	default:
		return 0;
	}
		
	return 1;
}

static int TreeTravel_Back( const char * const pszcmd )
{
	const node_t *pNode;
	
	if( strcmp( pszcmd, "back" ) == 0 ||
		strcmp( pszcmd, ".." ) == 0 )
	{
	} else
		return 0;
	
	if( nNodeDepth == 0 )
		return 0;
	else if( nNodeDepth == 1 ) {
		InitializeCmdCore();
		return 1;
	}
	
	nNodeDepth --;
	
	pNode = pNodePath[ nNodeDepth - 1 ];
	
	switch( pNode ->type ) {
	case NTYPE_INODE:
		sState = STATE_NODE;
		pNodeSelect = pNode ->next;
		pArgsSelect = NULL;
		break;
		
	case NTYPE_LEAF:
		sState = STATE_LEAF;
		pNodeSelect = NULL;
		pArgsSelect = pArgsInput = pNode ->args;
		break;
		
	default:
		return 0;
	}
	
	return 1;
}

static int TreeTravel_Root( const char * const pszcmd )
{
	if( strcmp( pszcmd, "root" ) == 0 ||
		strcmp( pszcmd, "/" ) == 0 )
	{
	} else
		return 0;
	
	InitializeCmdCore();
	
	return 1;
}

static void DoCmd_NodeLs( void )
{
	const node_t *pNode = pNodeSelect;

	while( pNode ->id ) {
		
		CliPrintf( "%s\n", pNode ->name );
		
		pNode = &pNode[ 1 ];	/* next node */
	}
}

static void DoCmd_LeafLs( void )
{
	const args_t *pArgs = pArgsSelect;

	while( pArgs ->name ) {
		
		CliPrintf( "%s\t", pArgs ->name );
		
		PrintArgsValue( aBuffer, pArgs );
		
		CliPrintf( "\n" );
		
		pArgs = &pArgs[ 1 ];	/* next argument */
	}
}

static int DoCmd_Ls( const char * const pszcmd )
{
	if( strcmp( pszcmd, "ls" ) == 0 )
		;
	else
		return 0;
		
	switch( sState ) {
	case STATE_NODE:
		DoCmd_NodeLs();
		break;
		
	case STATE_LEAF:
		DoCmd_LeafLs();
		break;
		
	case STATE_LEAF_INPUT_ITERATIVE:
	case STATE_LEAF_INPUT_ONCE:
	default:
		break;
	}
	
	return 1;
}

static int DoCmd_Args( const char * const pszcmd )
{
	if( strcmp( pszcmd, "args" ) == 0 )
		;
	else
		return 0;
	
	if( sState == STATE_LEAF ) {
		pArgsInput = pArgsSelect;
		sState = STATE_LEAF_INPUT_ITERATIVE;
	} else
		return 0;
		
	return 1;
}

static int DoCmd_LeafInput( const char * const pszcmd )
{
	if( sState == STATE_LEAF_INPUT_ITERATIVE || 
		sState == STATE_LEAF_INPUT_ONCE )
	{
	} else
		return 0;
	
	if( pszcmd[ 0 ] != 0 )
		WriteArgsValue( aBuffer, pArgsInput, pszcmd );
	
	CliPrintf( "%s\t", pArgsInput ->name );
	PrintArgsValue( aBuffer, pArgsInput );
	CliPrintf( "\n" );
	
	if( sState == STATE_LEAF_INPUT_ONCE )
		goto label_back_one_level;
	
	pArgsInput = &pArgsInput[ 1 ];	/* enter next one */
	
	if( pArgsInput ->name == NULL ) {
label_back_one_level:
		pArgsInput = pArgsSelect;
		sState = STATE_LEAF;
	}
	
	return 1;
}

static int DoCmd_Run( const char * const pszcmd )
{
	const node_t * pNode;
	
	if( sState == STATE_LEAF && strcmp( pszcmd, "run" ) == 0 )
		;
	else
		return 0;
	
	if( nNodeDepth == 0 )
		return 0;
		
	pNode = pNodePath[ nNodeDepth - 1 ];
	
	if( pNode ->type != NTYPE_LEAF )
		return 0;
	
	CmdNodeExecute( pNode );
	
	return 1;
}

int CmdCoreParser( const char * const pszcmd )
{
	if( DoCmd_LeafInput( pszcmd ) )
		return 1;
	
	if( DoCmd_Ls( pszcmd ) )
		return 1;
	
	if( DoCmd_Args( pszcmd ) )
		return 1;
	
	if( DoCmd_Run( pszcmd ) )
		return 1;
	
	if( TreeTravel_Enter( pszcmd ) )
		return 1;
	
	if( TreeTravel_Back( pszcmd ) )
		return 1;

	if( TreeTravel_Root( pszcmd ) )
		return 1;
	
	return 0;
}

