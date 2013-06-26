#include <linux/kernel.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_init.h"
#include "voip_proc.h"
#include "voip_debug.h"
#include "con_register.h"
#include "con_ipc_mirror.h"
#include "con_ipc_help_host.h"

static uint32 dsp_cch_count[ CONFIG_RTK_VOIP_DSP_DEVICE_NR ];

// -------------------------------------------------------------
// create invert table: dsp (cpuid/cch) --> host cch 
// -------------------------------------------------------------

#define NO_INVERT_HOST_CCH		( ( uint32 )-1 )

#define INVERT_TABLE_IDX( dsp_cpuid, dsp_cch )	\
		( ( dsp_cpuid ) + ( dsp_cch ) * CONFIG_RTK_VOIP_DSP_DEVICE_NR )

/*
 * assume CONFIG_RTK_VOIP_DSP_DEVICE_NR = 3, and each dsp has 2 cch 
 *
 * dsp_cupid  dsp_cch   index
 *     0         0        0 = 0 + 0 * 3
 *     0         1        3 = 0 + 1 * 3
 *     1         0        1 = 1 + 0 * 3
 *     1         1        4 = 1 + 1 * 3
 *     2         0        2 = 2 + 0 * 3
 *     2         1        5 = 2 + 1 * 3
 */
static uint32 host_cch_invert_table[ CON_CH_NUM ];

void ipc_construct_host_cch_invert_table( voip_con_t voip_con[] )
{
	voip_con_t *p_con;
	int i;
	uint32 idx;
	
	for( i = 0; i < CON_CH_NUM; i ++ )
		host_cch_invert_table[ i ] = NO_INVERT_HOST_CCH;
	
	for( i = 0; i < CON_CH_NUM; i ++ ) {
		
		p_con = &voip_con[ i ];
		
		if( !p_con ->ipc.priv ) 
			continue;
		
		// convert to idx 
		//idx = p_con ->ipc.dsp_cpuid + p_con ->ipc.dsp_cch * CONFIG_RTK_VOIP_DSP_DEVICE_NR;
		idx = INVERT_TABLE_IDX( p_con ->ipc.dsp_cpuid, p_con ->ipc.dsp_cch );
		
		// check overflow 
		if( idx >= CON_CH_NUM ) {
			printk( "Idx too large!! host_cch = %d, idx=%d\n", p_con ->cch, idx );
			continue;			
		}
		
		// check redundant? 
		if( host_cch_invert_table[ idx ] != NO_INVERT_HOST_CCH ) {
			printk( "Field is not empty!! host_cch = %d, idx=%d\n", p_con ->cch, idx );
			continue;
		}
		
		// fill data 
		host_cch_invert_table[ idx ] = p_con ->cch;
	}
}

uint32 ipc_get_host_cch_from_invert_table( uint32 dsp_cpuid, uint32 dsp_cch )
{
	// retrieve con_ptr, only *mux* can use this!! 
	//extern voip_con_t *get_voip_con_ptr( uint32 cch );

	uint32 idx, host_cch = ( uint32 )-1;
	//const voip_con_t *p_con;
	
	idx = INVERT_TABLE_IDX( dsp_cpuid, dsp_cch );
	
	if( idx >= CON_CH_NUM ) 
		goto label_error;
	
	host_cch = host_cch_invert_table[ idx ];
	
	if( host_cch >= CON_CH_NUM )
		goto label_error;
	
	// check again 
#if 1
	return host_cch;
#else
	p_con = get_voip_con_ptr( host_cch );
	
	if( p_con ->ipc.dsp_cpuid == dsp_cpuid &&
		p_con ->ipc.dsp_cch == dsp_cch )
	{
		return host_cch;
	} 
#endif

label_error:	
	printk( "ipc_get_host_cch_from_invert_table (%d,%d) idx=%u host_cch=%d\n", dsp_cpuid, dsp_cch, idx, host_cch );
	
	return ( uint32 )-1;
}

static int voip_host_cch_invert_table_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int n = 0;
	int i;
	
	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	n += sprintf( buf + n, "index dsp_cpuid dsp_cch host_cch\n" );
	n += sprintf( buf + n, "----- --------- ------- --------\n" );
	
	for( i = 0; i < CON_CH_NUM; i ++ ) {
		n += sprintf( buf + n, "  %2d  " "    %2d    " "   %2d   " "   %2d   " "\n",
						i, i % CONFIG_RTK_VOIP_DSP_DEVICE_NR,
						i / CONFIG_RTK_VOIP_DSP_DEVICE_NR,
						host_cch_invert_table[ i ] );
	}
	
	return n;
}

void ipc_print_host_cch_invert_table( void )
{
	int eof;
	char buff[ 512 ];
	
	voip_host_cch_invert_table_read_proc( buff, NULL, 0, 0, &eof, NULL );
	
	PRINT_Y( "%s", buff );
}

static int __init voip_host_cch_invert_table_proc_init( void )
{
	create_proc_read_entry( PROC_VOIP_DIR "/" PROC_VOIP_CH_MAPS_DIR "/invhostcch", 0, NULL, 
			voip_host_cch_invert_table_read_proc, NULL );
	
	return 0;
}

static void __exit voip_host_cch_invert_table_proc_exit( void )
{
	remove_proc_entry( PROC_VOIP_DIR "/" PROC_VOIP_CH_MAPS_DIR "/invhostcch", NULL );
}

voip_initcall_proc( voip_host_cch_invert_table_proc_init );
voip_exitcall( voip_host_cch_invert_table_proc_exit );

// -------------------------------------------------------------
// assign dsp (cpuid/cch) to host cch 
// -------------------------------------------------------------
//#define MAGIC_NUM( cpuid, cch )		( ( void * )( 0xFFFF | ( cpuid << 8 ) | ( cch ) ) )

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
static ipc_mirror_priv_data_t ipc_mirror_priv_data[ CON_CH_NUM ];
#endif

static inline void ____ipc_assign_data( voip_con_t *p_con, 
										void *ipc_priv, 
										uint16 dsp_cpuid,
										uint16 dsp_cch )
{
	//p_con ->ipc.priv = MAGIC_NUM( dsp_cpuid, dsp_cch_count[ dsp_cpuid ] );
	p_con ->ipc.priv = ipc_priv;
	p_con ->ipc.dsp_cpuid = dsp_cpuid;
	p_con ->ipc.dsp_cch = dsp_cch;
}

static inline void * ____get_ipc_priv_ptr( int idx )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	return MAGIC_IPC_PRIV_SHADOW;
#else
	return &ipc_mirror_priv_data[ idx ];
#endif
}

static void __ipc_assign_dsp_cpuid_cch( voip_con_t voip_con[], int con_num, int snd_num, snd_type_t snd_type )
{
	int i;
	voip_con_t *p_con;
	voip_snd_t *p_snd;
	int ch_per_dsp, dsp_cpuid;
	void *ipc_priv;

	ch_per_dsp = ( ( snd_num + CONFIG_RTK_VOIP_DSP_DEVICE_NR - 1 ) / CONFIG_RTK_VOIP_DSP_DEVICE_NR );
	dsp_cpuid = 0;
	
	for( i = 0; i < con_num; i ++ ) {
	
		p_con = &voip_con[ i ];
		
		if( snd_type != SND_TYPE_NONE ) {	// check type 
			p_snd = p_con ->snd_ptr;
			if( p_snd && p_snd ->snd_type == snd_type )
				;
			else
				continue;
		}
		
		if( p_con ->ipc.priv )	// has been filled!! 
			continue;
		
		// assign data 
		ipc_priv = ____get_ipc_priv_ptr( i );
		
		____ipc_assign_data( p_con, ipc_priv, dsp_cpuid, dsp_cch_count[ dsp_cpuid ] );
		
		// post process 
		dsp_cch_count[ dsp_cpuid ] ++;
		
		if( dsp_cch_count[ dsp_cpuid ] >= ch_per_dsp ) {
			dsp_cpuid ++;
		}
	}
}

void ipc_assign_dsp_cpuid_cch( voip_con_t voip_con[], int num )
{
	/*
	 * Assign policy is very simple.
	 * (DSP number is CONFIG_RTK_VOIP_DSP_DEVICE_NR)
	 *
	 * case 1: If fxs or daa != 0, share fxs/daa to DSP. 
	 * case 2: If fxs and daa == 0, assign continues cch to DSP 
	 */
	int i;
	//int ch_per_dsp, dsp_cch, dsp_cpuid;
	int fxs = 0, daa = 0;
	voip_con_t *p_con;
	voip_snd_t *p_snd;
	void *ipc_priv;
	
	// according to snd predefiend!! 
	for( i = 0; i < num; i ++ ) {
		p_con = &voip_con[ i ];
		p_snd = p_con ->snd_ptr;
		
		if( p_snd == NULL )
			continue;
		
		if( !p_snd ->ipc.f_dsp_cpuid )
			continue;	// processed in below loop!! 
		
		ipc_priv = ____get_ipc_priv_ptr( i );
		
		____ipc_assign_data( p_con, ipc_priv, 
					p_snd ->ipc.n_dsp_cpuid, 
					dsp_cch_count[ p_snd ->ipc.n_dsp_cpuid ] ++ );
	}	
	
	// calculate fxs/daa 
	for( i = 0; i < num; i ++ ) {
		p_con = &voip_con[ i ];
		p_snd = p_con ->snd_ptr;
		
		if( p_snd == NULL )
			continue;
		
		if( p_snd ->ipc.f_dsp_cpuid )
			continue;	// processed in above loop!! 
		
		switch( p_snd ->snd_type ) {
		case SND_TYPE_FXS:
			fxs ++;
			break;
			
		case SND_TYPE_DAA:
			daa ++;
			break;
		
		default:
			printk( "Can't assign cpuid/cch to snd_type=%d\n", p_snd ->snd_type );
			break;
		}
	}
	
	if( fxs == 0 && daa == 0 )
		goto label_case2;
	
	// case 1: fxs or daa != 0
	if( fxs )
		__ipc_assign_dsp_cpuid_cch( voip_con, num, fxs, SND_TYPE_FXS );
	
	if( daa )
		__ipc_assign_dsp_cpuid_cch( voip_con, num, daa, SND_TYPE_DAA );
	
	return;
	
	// case 2: fxs and daa == 0
label_case2:
	
	__ipc_assign_dsp_cpuid_cch( voip_con, num, num, SND_TYPE_NONE );
	
	return;	
}

