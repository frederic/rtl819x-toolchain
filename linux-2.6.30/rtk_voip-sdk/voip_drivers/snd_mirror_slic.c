#include <linux/kernel.h>
#include "voip_init.h"
#include "voip_debug.h"
#ifdef CONFIG_RTK_VOIP_IPC_ARCH
#include "voip_ipc.h"
#endif
#include "con_register.h"

#include "snd_mirror_define.h"

static mirror_slic_priv_t mirror_slic_priv[ CONFIG_RTK_VOIP_DRIVERS_MIRROR_SLIC_NR ];
static voip_snd_t snd_mirror_slic[ CONFIG_RTK_VOIP_DRIVERS_MIRROR_SLIC_NR ];

// --------------------------------------------------------
// Mirror SLIC additional functions 
// --------------------------------------------------------

static void Mirror_SLIC_All( struct voip_snd_s *this, const void * all )
{
	mirror_slic_priv_t * const priv = ( mirror_slic_priv_t * )this ->priv;
	const mirror_slic_priv_t * const mirror_all = ( const mirror_slic_priv_t * )all;
	
	*priv = *mirror_all;
}

// --------------------------------------------------------
// Regular SLIC functions 
// --------------------------------------------------------

#define BASIC_CONT_SIZE		( ( uint32 )&( ( ( rpc_content_t * )0 ) ->data ) )
#define FXS_OPS_OFFSET( f )	( ( uint32 )&( ( ( snd_ops_fxs_t * )0 ) ->f ) )

#define DECLARE_SLIC_RPC_VARS( ops, f_size )						\
	voip_con_t * const p_con = this ->con_ptr;						\
	uint8 content[ BASIC_CONT_SIZE + f_size ]; 						\
	rpc_content_t * const rpc_content = ( rpc_content_t * )content;	\
	uint8 * const p_data = rpc_content ->data;						\
																	\
	rpc_content ->cch = p_con ->cch;								\
	rpc_content ->ops_offset = FXS_OPS_OFFSET( ops );

#define PRINT_Y_LV1( x )	if( ( ( mirror_slic_priv_t * )this ->priv ) ->verbose >= 1 )	\
								PRINT_Y( x )
#define PRINT_Y_LV2( x )	if( ( ( mirror_slic_priv_t * )this ->priv ) ->verbose >= 2 )	\
								PRINT_Y( x )

static void SLIC_reset_mirror_slic( voip_snd_t *this, int codec_law )
{
	PRINT_Y_LV1( "SLIC_reset_mirror_slic\n" );
}

static void FXS_Ring_mirror_slic( voip_snd_t *this, unsigned char ringset )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH
	mirror_slic_priv_t * const priv = ( mirror_slic_priv_t * )this ->priv;
	DECLARE_SLIC_RPC_VARS( FXS_Ring, sizeof( ringset ) );
	
	priv ->ringing = 1;	// give mirror initial value!! (it's ringing)
	
	*( ( unsigned char * )p_data ) = ringset;
	
	p_con ->con_ops ->ipc_rpc_request( p_con, IPC_RPC_SLIC, content, sizeof( content ) );
#else	
	PRINT_Y_LV1( "FXS_Ring_mirror_slic\n" );
#endif
}

static unsigned char FXS_Check_Ring_mirror_slic( voip_snd_t *this )
{
	mirror_slic_priv_t * const priv = ( mirror_slic_priv_t * )this ->priv;
	
	PRINT_Y_LV2( "FXS_Check_Ring_mirror_slic\n" );
	
	return priv ->ringing;
}

static unsigned int FXS_Line_Check_mirror_slic( voip_snd_t *this )	// Note: this API may cause watch dog timeout. Should it disable WTD?
{
	PRINT_Y_LV1( "FXS_Line_Check_mirror_slic\n" );
	
	return 0;
}

static void SLIC_Set_PCM_state_mirror_slic( voip_snd_t *this, int enable )
{
	PRINT_Y_LV1( "SLIC_Set_PCM_state_mirror_slic\n" );
}

static unsigned char SLIC_Get_Hook_Status_mirror_slic( voip_snd_t *this, int directly )
{
	mirror_slic_priv_t * const priv = ( mirror_slic_priv_t * )this ->priv;
	
	if( !directly ) {
		PRINT_Y_LV2( "SLIC_Get_Hook_Status_mirror_slic\n" );
	}
	
	return ( unsigned char )priv ->hook_status;
}
	
static void Set_SLIC_Tx_Gain_mirror_slic( voip_snd_t *this, int tx_gain )
{
	PRINT_Y_LV1( "Set_SLIC_Tx_Gain_mirror_slic\n" );
}

static void Set_SLIC_Rx_Gain_mirror_slic( voip_snd_t *this, int rx_gain )
{
	PRINT_Y_LV1( "Set_SLIC_Rx_Gain_mirror_slic\n" );
}

static void SLIC_Set_Ring_Cadence_mirror_slic( voip_snd_t *this, unsigned short OnMsec, unsigned short OffMsec )
{
	// Host will do this and forward to DSP 
	
	PRINT_Y_LV2( "SLIC_Set_Ring_Cadence_mirror_slic\n" );
}

static void SLIC_Set_Ring_Freq_Amp_mirror_slic( voip_snd_t *this, char preset )
{
	PRINT_Y_LV1( "SLIC_Set_Ring_Freq_Amp_mirror_slic\n" );
}

static void SLIC_Set_Impendance_Country_mirror_slic( voip_snd_t *this, unsigned short country, unsigned short impd )
{
	// Host will do this and forward to DSP 
	
	PRINT_Y_LV2( "SLIC_Set_Impendance_Country_mirror_slic\n" );
}

static void SLIC_Set_Impendance_mirror_slic( voip_snd_t *this, unsigned short preset )
{
	PRINT_Y_LV1( "SLIC_Set_Impendance_mirror_slic\n" );
}

static void OnHookLineReversal_mirror_slic( voip_snd_t *this, unsigned char bReversal ) //0: Forward On-Hook Transmission, 1: Reverse On-Hook Transmission
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH
	DECLARE_SLIC_RPC_VARS( OnHookLineReversal, sizeof( bReversal ) );
	
	*( ( unsigned char * )p_data ) = bReversal;
	
	p_con ->con_ops ->ipc_rpc_request( p_con, IPC_RPC_SLIC, content, sizeof( content ) );

#else	
	PRINT_Y_LV1( "OnHookLineReversal_mirror_slic\n" );
#endif
}

static void SLIC_Set_LineVoltageZero_mirror_slic( voip_snd_t *this )
{
	PRINT_Y_LV1( "SLIC_Set_LineVoltageZero_mirror_slic\n" );
}

static uint8 SLIC_CPC_Gen_mirror_slic( voip_snd_t *this )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH
	DECLARE_SLIC_RPC_VARS( SLIC_CPC_Gen, 0 );
	
	*( ( int * )p_data ) = *( ( int * )p_data );	// avoid compiler warning 
	
	p_con ->con_ops ->ipc_rpc_request( p_con, IPC_RPC_SLIC, content, sizeof( content ) );
	
#else
	PRINT_Y_LV1( "SLIC_CPC_Gen_mirror_slic\n" );
#endif
	
	return 0;	// return value is not important in mirror 
}

static void SLIC_CPC_Check_mirror_slic( voip_snd_t *this, uint8 pre_linefeed )	// check in timer
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH
	DECLARE_SLIC_RPC_VARS( SLIC_CPC_Check, sizeof( pre_linefeed ) );
	
	*( ( uint8 * )p_data ) = pre_linefeed;
	
	p_con ->con_ops ->ipc_rpc_request( p_con, IPC_RPC_SLIC, content, sizeof( content ) );

#else	
	PRINT_Y_LV1( "SLIC_CPC_Check_mirror_slic\n" );
#endif
}

	
static void SendNTTCAR_mirror_slic( voip_snd_t *this )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH
	mirror_slic_priv_t * const priv = ( mirror_slic_priv_t * )this ->priv;
	DECLARE_SLIC_RPC_VARS( SendNTTCAR, 0 );
	
	priv ->SendNTTCAR_chk = 0;
	
	*( ( uint8 * )p_data ) = *( ( uint8 * )p_data );	// avoid compiler warning 
	
	p_con ->con_ops ->ipc_rpc_request( p_con, IPC_RPC_SLIC, content, sizeof( content ) );

#else	
	PRINT_Y_LV1( "SendNTTCAR_mirror_slic\n" );
#endif
}

static unsigned int SendNTTCAR_check_mirror_slic( voip_snd_t *this, unsigned long time_out )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH
	mirror_slic_priv_t * const priv = ( mirror_slic_priv_t * )this ->priv;
	DECLARE_SLIC_RPC_VARS( SendNTTCAR_check, sizeof( time_out ) );
	
	*( ( unsigned long * )p_data ) = time_out;
	
	p_con ->con_ops ->ipc_rpc_request( p_con, IPC_RPC_SLIC, content, sizeof( content ) );
	
	return priv ->SendNTTCAR_chk;

#else	
	PRINT_Y_LV1( "SendNTTCAR_check_mirror_slic\n" );
	
	return 0;
#endif
}
	
static void disableOscillators_mirror_slic( voip_snd_t *this )
{
	PRINT_Y_LV1( "disableOscillators_mirror_slic\n" );
}
	
static void SetOnHookTransmissionAndBackupRegister_mirror_slic( voip_snd_t *this ) // use for DTMF caller id
{
	PRINT_Y_LV1( "SetOnHookTransmissionAndBackupRegister_mirror_slic\n" );
}

static void RestoreBackupRegisterWhenSetOnHookTransmission_mirror_slic( voip_snd_t *this ) // use for DTMF caller id
{
	PRINT_Y_LV1( "RestoreBackupRegisterWhenSetOnHookTransmission_mirror_slic\n" );
}

static void FXS_FXO_DTx_DRx_Loopback_mirror_slic( voip_snd_t *this, voip_snd_t *daa_snd, unsigned int enable )
{
	PRINT_Y_LV1( "FXS_FXO_DTx_DRx_Loopback_mirror_slic\n" );
}

static void SLIC_OnHookTrans_PCM_start_mirror_slic( voip_snd_t *this )
{
	PRINT_Y_LV1( "SLIC_OnHookTrans_PCM_start_mirror_slic\n" );
}

// read/write register/ram
static void SLIC_read_reg_mirror_slic( voip_snd_t *this, unsigned int num, unsigned char *len, unsigned char *val )
{
	PRINT_Y_LV1( "SLIC_read_reg_mirror_slic\n" );
}

static void SLIC_write_reg_mirror_slic( voip_snd_t *this, unsigned int num, unsigned char *len, unsigned char *val )
{
	PRINT_Y_LV1( "SLIC_write_reg_mirror_slic\n" );
}

static void SLIC_read_ram_mirror_slic( voip_snd_t *this, unsigned short num, unsigned int *val )
{
	mirror_slic_priv_t *p_mirror_slic = this ->priv;
	
	switch( num ) {
	case 0:
		*val = p_mirror_slic ->hook_status;
		break;
	case 1:
		*val = p_mirror_slic ->ringing;
		break;
	case 2:
		*val = p_mirror_slic ->SendNTTCAR_chk;
		break;
	case 3:
		*val = p_mirror_slic ->verbose;
		break;
	default:
		*val = -1;
		break;
	}

	//PRINT_Y( "SLIC_read_ram_mirror_slic\n" );
}

static void SLIC_write_ram_mirror_slic( voip_snd_t *this, unsigned short num, unsigned int val )
{
	mirror_slic_priv_t *p_mirror_slic = this ->priv;
	
	switch( num ) {
	case 0:
		p_mirror_slic ->hook_status = val;
		break;
	case 1:
		p_mirror_slic ->ringing = val;
		break;
	case 2:
		p_mirror_slic ->SendNTTCAR_chk = val;
		break;
	case 3:
		p_mirror_slic ->verbose = val;
		break;
	}
	
	//PRINT_Y( "SLIC_write_ram_mirror_slic\n" );
}

static void SLIC_dump_reg_mirror_slic( voip_snd_t *this )
{
	PRINT_Y_LV1( "SLIC_dump_reg_mirror_slic\n" );
}

static void SLIC_dump_ram_mirror_slic( voip_snd_t *this )
{
	PRINT_Y_LV1( "SLIC_dump_ram_mirror_slic\n" );
}
	
static void SLIC_show_ID_mirror_slic( voip_snd_t *this )
{
	PRINT_Y_LV1( "SLIC_show_ID_mirror_slic\n" );
}

static int enable_mirror_slic( voip_snd_t *this, int enable )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH
	DECLARE_SLIC_RPC_VARS( enable, sizeof( enable ) );
	
	*( ( int * )p_data ) = enable;
	
	p_con ->con_ops ->ipc_rpc_request( p_con, IPC_RPC_SLIC, content, sizeof( content ) );

#else	
	//PRINT_Y( "enable_mirror_slic\n" );
#endif
	
	return 0;
}

// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------

static snd_ops_fxs_t snd_mirror_slic_ops = {
	// common operation 
	.enable = enable_mirror_slic,
	
	// for each snd_type 
	.SLIC_reset = SLIC_reset_mirror_slic,
	.FXS_Ring = FXS_Ring_mirror_slic,
	.FXS_Check_Ring = FXS_Check_Ring_mirror_slic,
	.FXS_Line_Check = FXS_Line_Check_mirror_slic,	// Note: this API may cause watch dog timeout. Should it disable WTD?
	.SLIC_Set_PCM_state = SLIC_Set_PCM_state_mirror_slic,
	.SLIC_Get_Hook_Status = SLIC_Get_Hook_Status_mirror_slic,
	
	.Set_SLIC_Tx_Gain = Set_SLIC_Tx_Gain_mirror_slic,
	.Set_SLIC_Rx_Gain = Set_SLIC_Rx_Gain_mirror_slic,
	.SLIC_Set_Ring_Cadence = SLIC_Set_Ring_Cadence_mirror_slic,
	.SLIC_Set_Ring_Freq_Amp = SLIC_Set_Ring_Freq_Amp_mirror_slic,
	.SLIC_Set_Impendance_Country = SLIC_Set_Impendance_Country_mirror_slic, 
	.SLIC_Set_Impendance = SLIC_Set_Impendance_mirror_slic,
	.OnHookLineReversal = OnHookLineReversal_mirror_slic,	//0: Forward On-Hook Transmission, 1: Reverse On-Hook Transmission
	.SLIC_Set_LineVoltageZero = SLIC_Set_LineVoltageZero_mirror_slic,
	
	.SLIC_CPC_Gen = SLIC_CPC_Gen_mirror_slic,
	.SLIC_CPC_Check = SLIC_CPC_Check_mirror_slic,	// check in timer
	
	.SendNTTCAR = SendNTTCAR_mirror_slic,
	.SendNTTCAR_check = SendNTTCAR_check_mirror_slic,
	
	.disableOscillators = disableOscillators_mirror_slic,
	
	.SetOnHookTransmissionAndBackupRegister = SetOnHookTransmissionAndBackupRegister_mirror_slic,	// use for DTMF caller id
	.RestoreBackupRegisterWhenSetOnHookTransmission = RestoreBackupRegisterWhenSetOnHookTransmission_mirror_slic,	// use for DTMF caller id
	
	.FXS_FXO_DTx_DRx_Loopback = FXS_FXO_DTx_DRx_Loopback_mirror_slic,
	.SLIC_OnHookTrans_PCM_start = SLIC_OnHookTrans_PCM_start_mirror_slic,
	
	// read/write register/ram
	.SLIC_read_reg = SLIC_read_reg_mirror_slic,
	.SLIC_write_reg = SLIC_write_reg_mirror_slic,
	.SLIC_read_ram = SLIC_read_ram_mirror_slic,
	.SLIC_write_ram = SLIC_write_ram_mirror_slic,
	.SLIC_dump_reg = SLIC_dump_reg_mirror_slic,
	.SLIC_dump_ram = SLIC_dump_ram_mirror_slic,
	
	.SLIC_show_ID = SLIC_show_ID_mirror_slic,
	
	// Mirror SLIC functions 
	.Mirror_SLIC_All = Mirror_SLIC_All,
};

static int __init voip_snd_init_mirror_slic( void )
{
	int i;
	uint16 TS = CONFIG_RTK_VOIP_DRIVERS_MIRROR_SLIC_TS;
	
	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_MIRROR_SLIC_NR; i ++ ) {
		snd_mirror_slic[ i ].sch = i;
		snd_mirror_slic[ i ].name = "slic(m)";
		snd_mirror_slic[ i ].snd_type = SND_TYPE_FXS;
		snd_mirror_slic[ i ].bus_type_sup = BUS_TYPE_PCM;
		snd_mirror_slic[ i ].TS1 = TS;
#ifdef CONFIG_RTK_VOIP_DRIVERS_MIRROR_SLIC_WIDEBAND
		snd_mirror_slic[ i ].TS2 = TS + 16;
		snd_mirror_slic[ i ].band_mode_sup = BAND_MODE_8K | BAND_MODE_16K;
#else
		snd_mirror_slic[ i ].TS2 = 0;
		snd_mirror_slic[ i ].band_mode_sup = BAND_MODE_8K;
#endif
		snd_mirror_slic[ i ].fxs_ops = &snd_mirror_slic_ops;
		snd_mirror_slic[ i ].priv = &mirror_slic_priv[ i ];
		
		TS += 2;
		
		// initial priv 
		mirror_slic_priv[ i ].verbose = 1;
	}
	
	register_voip_snd( snd_mirror_slic, CONFIG_RTK_VOIP_DRIVERS_MIRROR_SLIC_NR );
	
	return 0;
}

voip_initcall_snd( voip_snd_init_mirror_slic );

