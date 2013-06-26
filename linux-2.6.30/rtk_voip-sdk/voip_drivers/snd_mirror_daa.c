#include <linux/kernel.h>
#include "voip_init.h"
#include "voip_debug.h"
#ifdef CONFIG_RTK_VOIP_IPC_ARCH
#include "voip_ipc.h"
#endif
#include "con_register.h"

#include "snd_mirror_define.h"

static mirror_daa_priv_t mirror_daa_priv[ CONFIG_RTK_VOIP_DRIVERS_MIRROR_DAA_NR ];
static voip_snd_t snd_mirror_daa[ CONFIG_RTK_VOIP_DRIVERS_MIRROR_DAA_NR ];

// --------------------------------------------------------
// Mirror DAA additional functions 
// --------------------------------------------------------

static void Mirror_DAA_All( struct voip_snd_s *this, const void * all )
{
	mirror_daa_priv_t * const priv = ( mirror_daa_priv_t * )this ->priv;
	const mirror_daa_priv_t * const mirror_all = ( const mirror_daa_priv_t * )all;
	
	*priv = *mirror_all;
}

// --------------------------------------------------------
// Regular DAA functions 
// --------------------------------------------------------

#define BASIC_CONT_SIZE		( ( uint32 )&( ( ( rpc_content_t * )0 ) ->data ) )
#define DAA_OPS_OFFSET( f )	( ( uint32 )&( ( ( snd_ops_daa_t * )0 ) ->f ) )

#define DECLARE_DAA_RPC_VARS( ops, f_size )							\
	voip_con_t * const p_con = this ->con_ptr;						\
	uint8 content[ BASIC_CONT_SIZE + f_size ]; 						\
	rpc_content_t * const rpc_content = ( rpc_content_t * )content;	\
	uint8 * const p_data = rpc_content ->data;						\
																	\
	rpc_content ->cch = p_con ->cch;								\
	rpc_content ->ops_offset = DAA_OPS_OFFSET( ops );

#define PRINT_Y_LV1( x )	if( ( ( mirror_daa_priv_t * )this ->priv ) ->verbose >= 1 )	\
								PRINT_Y( x )
#define PRINT_Y_LV2( x )	if( ( ( mirror_daa_priv_t * )this ->priv ) ->verbose >= 2 )	\
								PRINT_Y( x )

static void DAA_Set_Rx_Gain_mirror_daa( struct voip_snd_s *this, unsigned char rx_gain )
{
	// OK: Host do it by itself 
	PRINT_Y_LV2( "DAA_Set_Rx_Gain_mirror_daa\n" );
}

static void DAA_Set_Tx_Gain_mirror_daa( struct voip_snd_s *this, unsigned char tx_gain )
{
	// OK: Host do it by itself 
	PRINT_Y_LV2( "DAA_Set_Tx_Gain_mirror_daa\n" );
}

static int  DAA_Check_Line_State_mirror_daa( struct voip_snd_s *this )	/* 0: connect, 1: not connect, 2: busy*/
{
	PRINT_Y_LV1( "DAA_Check_Line_State_mirror_daa\n" );
	
	return 0;
}

static void DAA_On_Hook_mirror_daa( struct voip_snd_s *this )
{
	PRINT_Y_LV1( "DAA_On_Hook_mirror_daa\n" );
}

static int DAA_Off_Hook_mirror_daa( struct voip_snd_s *this )
{
	PRINT_Y_LV1( "DAA_Off_Hook_mirror_daa\n" );
	
	return 0;
}

static unsigned char DAA_Hook_Status_mirror_daa( struct voip_snd_s *this, int directly )
{
	mirror_daa_priv_t * const priv = ( mirror_daa_priv_t * )this ->priv;
	
	if( !directly ) {
		PRINT_Y_LV2( "DAA_Hook_Status_mirror_daa\n" );
	}
	
	return ( unsigned char )priv ->hook_status;
}

static unsigned char DAA_Polarity_Reversal_Det_mirror_daa( struct voip_snd_s *this )
{
	mirror_daa_priv_t * const priv = ( mirror_daa_priv_t * )this ->priv;
	
	PRINT_Y_LV2( "DAA_Polarity_Reversal_Det_mirror_daa\n" );
	
	return ( unsigned char )priv ->pol_rev_det;
}

static unsigned char DAA_Bat_DropOut_Det_mirror_daa( struct voip_snd_s *this )
{
	mirror_daa_priv_t * const priv = ( mirror_daa_priv_t * )this ->priv;
	
	PRINT_Y_LV2( "DAA_Bat_DropOut_Det_mirror_daa\n" );
	
	return ( unsigned char )priv ->bat_dropout_det;
}

static int DAA_Ring_Detection_mirror_daa( struct voip_snd_s *this )
{
	PRINT_Y_LV1( "DAA_Ring_Detection_mirror_daa\n" );
	
	return 0;
}

static unsigned int DAA_Positive_Negative_Ring_Detect_mirror_daa( struct voip_snd_s *this )
{
	mirror_daa_priv_t * const priv = ( mirror_daa_priv_t * )this ->priv;
	
	PRINT_Y_LV2( "DAA_Positive_Negative_Ring_Detect_mirror_daa\n" );
	
	return ( unsigned int )priv ->pos_neg_ring;
}

static unsigned int DAA_Get_Polarity_mirror_daa( struct voip_snd_s *this )
{
	mirror_daa_priv_t * const priv = ( mirror_daa_priv_t * )this ->priv;
	
	PRINT_Y_LV2( "DAA_Get_Polarity_mirror_daa\n" );
	
	return ( unsigned int )priv ->polarity;
}

static unsigned short DAA_Get_Line_Voltage_mirror_daa( struct voip_snd_s *this )
{
	mirror_daa_priv_t * const priv = ( mirror_daa_priv_t * )this ->priv;
	
	PRINT_Y_LV2( "DAA_Get_Line_Voltage_mirror_daa\n" );
	
	return ( unsigned short )priv ->line_voltage;
}

static void DAA_OnHook_Line_Monitor_Enable_mirror_daa( struct voip_snd_s *this )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH
	DECLARE_DAA_RPC_VARS( DAA_OnHook_Line_Monitor_Enable, 0 );
	
	*( ( unsigned int * )p_data ) = *( ( unsigned int * )p_data );	// avoid compiler warning 
	
	p_con ->con_ops ->ipc_rpc_request( p_con, IPC_RPC_DAA, content, sizeof( content ) );

#else	
	PRINT_Y_LV1( "DAA_OnHook_Line_Monitor_Enable_mirror_daa\n" );
#endif
}

static void DAA_Set_PulseDial_mirror_daa( struct voip_snd_s *this, unsigned int pulse_enable )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH
	DECLARE_DAA_RPC_VARS( DAA_Set_PulseDial, sizeof( pulse_enable ) );
	
	*( ( unsigned int * )p_data ) = pulse_enable;
	
	p_con ->con_ops ->ipc_rpc_request( p_con, IPC_RPC_DAA, content, sizeof( content ) );

#else	
	PRINT_Y_LV1( "DAA_Set_PulseDial_mirror_daa\n" );
#endif
}

// read/write register/ram
static void DAA_read_reg_mirror_daa( struct voip_snd_s *this, unsigned int num, unsigned char *len, unsigned char *val )
{
	PRINT_Y_LV1( "DAA_read_reg_mirror_daa\n" );
}

static void DAA_write_reg_mirror_daa( struct voip_snd_s *this, unsigned int num, unsigned char *len, unsigned char *val )
{
	PRINT_Y_LV1( "DAA_write_reg_mirror_daa\n" );
}

static void DAA_read_ram_mirror_daa( struct voip_snd_s *this, unsigned short num, unsigned int *val )
{
	mirror_daa_priv_t *p_mirror_daa = this ->priv;
	
	switch( num ) {
	case 0:
		*val = p_mirror_daa ->hook_status;
		break;
	case 1:
		*val = p_mirror_daa ->pol_rev_det;
		break;
	case 2:
		*val = p_mirror_daa ->bat_dropout_det;
		break;
	case 3:
		*val = p_mirror_daa ->pos_neg_ring;
		break;
	case 4:
		*val = p_mirror_daa ->polarity;
		break;
	case 5:
		*val = p_mirror_daa ->line_voltage;
		break;
	case 6:
		*val = p_mirror_daa ->verbose;
		break;
	default:
		*val = -1;
		break;
	}
	
	//PRINT_Y( "DAA_read_ram_mirror_daa\n" );
}

static void DAA_write_ram_mirror_daa( struct voip_snd_s *this, unsigned short num, unsigned int val )
{
	mirror_daa_priv_t *p_mirror_daa = this ->priv;
	
	switch( num ) {
	case 0:
		p_mirror_daa ->hook_status = val;
		break;
	case 1:
		p_mirror_daa ->pol_rev_det = val;
		break;
	case 2:
		p_mirror_daa ->bat_dropout_det = val;
		break;
	case 3:
		p_mirror_daa ->pos_neg_ring = val;
		break;
	case 4:
		p_mirror_daa ->polarity = val;
		break;
	case 5:
		p_mirror_daa ->line_voltage = val;
		break;
	case 6:
		p_mirror_daa ->verbose = val;
		break;
	}
	//PRINT_Y( "DAA_write_ram_mirror_daa\n" );
}

static void DAA_dump_reg_mirror_daa( struct voip_snd_s *this )
{
	PRINT_Y_LV1( "DAA_dump_reg_mirror_daa\n" );
}

static void DAA_dump_ram_mirror_daa( struct voip_snd_s *this )
{
	PRINT_Y_LV1( "DAA_dump_ram_mirror_daa\n" );
}

static int enable_mirror_daa( struct voip_snd_s *this, int enable )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH
	DECLARE_DAA_RPC_VARS( enable, sizeof( enable ) );
	
	*( ( int * )p_data ) = enable;
	
	p_con ->con_ops ->ipc_rpc_request( p_con, IPC_RPC_DAA, content, sizeof( content ) );

#else	
	//PRINT_Y( "enable_mirror_daa\n" );
#endif
	
	return 0;
}

// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------

static const snd_ops_daa_t snd_mirror_daa_ops = {
	// common operation 
	.enable = enable_mirror_daa,
	
	// for each snd_type 
	.DAA_Set_Rx_Gain = DAA_Set_Rx_Gain_mirror_daa,
	.DAA_Set_Tx_Gain = DAA_Set_Tx_Gain_mirror_daa,
	.DAA_Check_Line_State = DAA_Check_Line_State_mirror_daa,
	.DAA_On_Hook = DAA_On_Hook_mirror_daa,
	.DAA_Off_Hook = DAA_Off_Hook_mirror_daa,
	.DAA_Hook_Status = DAA_Hook_Status_mirror_daa,
	.DAA_Polarity_Reversal_Det = DAA_Polarity_Reversal_Det_mirror_daa,
	.DAA_Bat_DropOut_Det = DAA_Bat_DropOut_Det_mirror_daa,
	.DAA_Ring_Detection = DAA_Ring_Detection_mirror_daa,
	.DAA_Positive_Negative_Ring_Detect = DAA_Positive_Negative_Ring_Detect_mirror_daa,
	.DAA_Get_Polarity = DAA_Get_Polarity_mirror_daa,
	.DAA_Get_Line_Voltage = DAA_Get_Line_Voltage_mirror_daa,
	.DAA_OnHook_Line_Monitor_Enable = DAA_OnHook_Line_Monitor_Enable_mirror_daa,
	.DAA_Set_PulseDial = DAA_Set_PulseDial_mirror_daa,
	
	// read/write register/ram
	.DAA_read_reg = DAA_read_reg_mirror_daa,
	.DAA_write_reg = DAA_write_reg_mirror_daa,
	.DAA_read_ram = DAA_read_ram_mirror_daa,
	.DAA_write_ram = DAA_write_ram_mirror_daa,
	.DAA_dump_reg = DAA_dump_reg_mirror_daa,
	.DAA_dump_ram = DAA_dump_ram_mirror_daa,
	
	// Mirror DAA functions 
	.Mirror_DAA_All = Mirror_DAA_All,
};

static int __init voip_snd_init_mirror_daa( void )
{
	int i;
	uint16 TS = CONFIG_RTK_VOIP_DRIVERS_MIRROR_DAA_TS;
	
	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_MIRROR_DAA_NR; i ++ ) {
		snd_mirror_daa[ i ].sch = i;
		snd_mirror_daa[ i ].name = "daa(m)";
		snd_mirror_daa[ i ].snd_type = SND_TYPE_DAA;
		snd_mirror_daa[ i ].bus_type_sup = BUS_TYPE_PCM;
		snd_mirror_daa[ i ].TS1 = TS;
		snd_mirror_daa[ i ].TS2 = 0;
		snd_mirror_daa[ i ].band_mode_sup = BAND_MODE_8K;
		snd_mirror_daa[ i ].daa_ops = &snd_mirror_daa_ops;
		snd_mirror_daa[ i ].priv = ( void * )&mirror_daa_priv[ i ];
		
		TS += 2;
		
		// initial priv 
		mirror_daa_priv[ i ].verbose = 1;
	}
	
	register_voip_snd( snd_mirror_daa, CONFIG_RTK_VOIP_DRIVERS_MIRROR_DAA_NR );
	
	return 0;
}

voip_initcall_snd( voip_snd_init_mirror_daa );

