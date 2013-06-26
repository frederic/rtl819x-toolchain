#ifndef __CON_REGISTER_H__
#define __CON_REGISTER_H__

#include "voip_types.h"
#include "con_define.h"

// ioc_*, snd_*, bus_* and dsp_* include this files

struct voip_ioc_s;
struct voip_snd_s;
struct voip_bus_s;
struct voip_dsp_s;
struct voip_con_s;

// --------------------------------------------------------
// enum definition 
// --------------------------------------------------------

typedef enum {
	IOC_TYPE_LED,
	IOC_TYPE_RELAY,
} ioc_type_t;

typedef enum {
	SND_TYPE_FXS,
	SND_TYPE_DAA,
	SND_TYPE_VDAA,
	SND_TYPE_DECT,
	SND_TYPE_AC,
	SND_TYPE_NONE,	// used to occupy PCM timeslot only (SiTEL use this)
} snd_type_t;

enum {
	BUS_TYPE_PCM = 0x01,	// bit 0
	BUS_TYPE_IIS = 0x02,	// bit 1
	BUS_TYPE_RELAY = 0x04,	// bit 2, it is a relay, so don't need bus
};
typedef uint32 bus_type_t;	// bit mask 

enum {
	BAND_MODE_8K = 0x01,		// bit 0
	BAND_MODE_16K = 0x02,		// bit 1
	BAND_MODE_8K_PLUS = 0x04,	// bit 2 (8k+ mode used by con_ in runtime only)
};
typedef uint32 band_mode_t;	// bit mask 

typedef enum {
	DSP_TYPE_REALTEK,
	DSP_TYPE_AUDIOCODES,
} dsp_type_t;

typedef enum {
	BUS_ROLE_SINGLE,		// normal
	BUS_ROLE_MAIN,			// primary of 8k+ mode 
	BUS_ROLE_MATE,			// secondary of 8k+ mode 
} bus_role_t;

enum {
	CON_SHARE_BUS = 0x01,		// bit 0, two cch map to a bus_ch 
};
typedef uint32 con_share_t;	// bit mask

typedef enum {
	BUS_GROUP_NONE = 0x00,
	BUS_GROUP_PCM03 = 0xA0,
	BUS_GROUP_PCM47 = 0xB3,
	BUS_GROUP_PCM811 = 0xC5,
	BUS_GROUP_PCM1215 = 0xD7,
} bus_group_t;

// --------------------------------------------------------
// io controller (binary) - led/relay
// --------------------------------------------------------

typedef enum {
	IOC_MODE_NORMAL,
	IOC_MODE_BLINKING,	// special mode for LED application (modify state automatically) 
} ioc_mode_t;

typedef enum {
	IOC_STATE_LED_ON,
	IOC_STATE_LED_OFF,
	IOC_STATE_RELAY_CLOSE,
	IOC_STATE_RELAY_OPEN,
} ioc_state_t;

typedef struct {
	// common operation 
	int ( *open )( struct voip_ioc_s *this );
	int ( *set_state )( struct voip_ioc_s *this, ioc_state_t state );
	uint32 ( *get_id )( struct voip_ioc_s *this );	// for display 
} ioc_ops_t;

struct voip_ioc_s {
	// basic info. [given for register. readonly]
	uint32	ioch;
	const char *name;	// length of name can be less than 7 for better look
	ioc_type_t ioc_type;
	struct voip_snd_s *pre_assigned_snd_ptr;	// this ioc is pre-assigned to certain snd 
	const ioc_ops_t *ioc_ops;
	void *priv;		// private data for ioc driver 
	
	// runtime value (used by ioc_core)
	ioc_state_t state_var;	// on/off or close/open 
	ioc_mode_t mode_var;	// ioc_type == IOC_TYPE_LED
	
	// binding value 
	
	// binding info. 
	struct voip_con_s *con_ptr;		// point to parent console 
	
	// link list [console register manage this items]
	struct {
		struct voip_ioc_s *next;
		//struct voip_ioc_s *prev;
	} link;
};

// --------------------------------------------------------
// sound - fxs/daa/vdaa/dect/ac
// --------------------------------------------------------

#define Is_FXS_snd( snd )		( snd && snd ->snd_type == SND_TYPE_FXS )
#define Is_DAA_snd( snd )		( snd && snd ->snd_type == SND_TYPE_DAA )
#define Is_VDAA_snd( snd )		( snd && snd ->snd_type == SND_TYPE_VDAA )

typedef struct {		// FXS - ops  
	// common operation 
	int ( *open )( struct voip_snd_s *this );
	int ( *enable )( struct voip_snd_s *this, int enable );
	
	// for each snd_type 
	void          ( *SLIC_reset )( struct voip_snd_s *this, int codec_law );
	void          ( *FXS_Ring )( struct voip_snd_s *this, unsigned char ringset );
	unsigned char ( *FXS_Check_Ring )( struct voip_snd_s *this );
	unsigned int  ( *FXS_Line_Check )( struct voip_snd_s *this );	// Note: this API may cause watch dog timeout. Should it disable WTD?
	void          ( *SLIC_Set_PCM_state )( struct voip_snd_s *this, int enable );
	unsigned char ( *SLIC_Get_Hook_Status )( struct voip_snd_s *this, int directly );
	
	void ( *Set_SLIC_Tx_Gain )( struct voip_snd_s *this, int tx_gain );
	void ( *Set_SLIC_Rx_Gain )( struct voip_snd_s *this, int rx_gain );
	void ( *SLIC_Set_Ring_Cadence )( struct voip_snd_s *this, unsigned short OnMsec, unsigned short OffMsec );
	void ( *SLIC_Set_Ring_Freq_Amp )( struct voip_snd_s *this, char preset );
	void ( *SLIC_Set_Impendance_Country )( struct voip_snd_s *this, unsigned short country, unsigned short impd );
	void ( *SLIC_Set_Impendance )( struct voip_snd_s *this, unsigned short preset );
	void ( *OnHookLineReversal )( struct voip_snd_s *this, unsigned char bReversal ); //0: Forward On-Hook Transmission, 1: Reverse On-Hook Transmission
	void ( *SLIC_Set_LineVoltageZero )( struct voip_snd_s *this );
	
	uint8 ( *SLIC_CPC_Gen )( struct voip_snd_s *this );
	void  ( *SLIC_CPC_Check )( struct voip_snd_s *this, uint8 pre_linefeed );	// check in timer
	
	void         ( *SendNTTCAR )( struct voip_snd_s *this );
	unsigned int ( *SendNTTCAR_check )( struct voip_snd_s *this, unsigned long time_out );
	
	void ( *disableOscillators )( struct voip_snd_s *this );
	
	void ( *SetOnHookTransmissionAndBackupRegister )( struct voip_snd_s *this ); // use for DTMF caller id
	void ( *RestoreBackupRegisterWhenSetOnHookTransmission )( struct voip_snd_s *this ); // use for DTMF caller id
	
	void ( *FXS_FXO_DTx_DRx_Loopback )( struct voip_snd_s *this, struct voip_snd_s *daa_snd, unsigned int enable );
	void ( *SLIC_OnHookTrans_PCM_start )( struct voip_snd_s *this );
	
	// read/write register/ram
	void ( *SLIC_read_reg )( struct voip_snd_s *this, unsigned int num, unsigned char *len, unsigned char *val );
	void ( *SLIC_write_reg )( struct voip_snd_s *this, unsigned int num, unsigned char *len, unsigned char *val );
	void ( *SLIC_read_ram )( struct voip_snd_s *this, unsigned short num, unsigned int *val );
	void ( *SLIC_write_ram )( struct voip_snd_s *this, unsigned short num, unsigned int val );
	void ( *SLIC_dump_reg )( struct voip_snd_s *this );
	void ( *SLIC_dump_ram )( struct voip_snd_s *this );
	
	void ( *SLIC_show_ID )( struct voip_snd_s *this );
	
	// Mirror SLIC functions 
	void ( *Mirror_SLIC_All )( struct voip_snd_s *this, const void * all );	// 'all' is mirror_slic_priv_t *

} snd_ops_fxs_t;

typedef struct {		// DAA - ops 
	// common operation 
	int ( *open )( struct voip_snd_s *this );
	int ( *enable )( struct voip_snd_s *this, int enable );
	
	// for each snd_type 
	void ( *DAA_Set_Rx_Gain )( struct voip_snd_s *this, unsigned char rx_gain );
	void ( *DAA_Set_Tx_Gain )( struct voip_snd_s *this, unsigned char tx_gain );
	int  ( *DAA_Check_Line_State )( struct voip_snd_s *this );	/* 0: connect, 1: not connect, 2: busy*/
	void ( *DAA_On_Hook )( struct voip_snd_s *this );
	int  ( *DAA_Off_Hook )( struct voip_snd_s *this );
	unsigned char ( *DAA_Hook_Status )( struct voip_snd_s *this, int directly );
	unsigned char ( *DAA_Polarity_Reversal_Det )( struct voip_snd_s *this );
	unsigned char ( *DAA_Bat_DropOut_Det )( struct voip_snd_s *this );
	int  ( *DAA_Ring_Detection )( struct voip_snd_s *this );
	unsigned int  ( *DAA_Positive_Negative_Ring_Detect )( struct voip_snd_s *this );
	unsigned int  ( *DAA_Get_Polarity )( struct voip_snd_s *this );
	unsigned short( *DAA_Get_Line_Voltage )( struct voip_snd_s *this );
	void ( *DAA_OnHook_Line_Monitor_Enable )( struct voip_snd_s *this );
	void ( *DAA_Set_PulseDial )( struct voip_snd_s *this, unsigned int pulse_enable );
	
	// read/write register/ram
	void ( *DAA_read_reg )( struct voip_snd_s *this, unsigned int num, unsigned char *len, unsigned char *val );
	void ( *DAA_write_reg )( struct voip_snd_s *this, unsigned int num, unsigned char *len, unsigned char *val );
	void ( *DAA_read_ram )( struct voip_snd_s *this, unsigned short num, unsigned int *val );
	void ( *DAA_write_ram )( struct voip_snd_s *this, unsigned short num, unsigned int val );
	void ( *DAA_dump_reg )( struct voip_snd_s *this );
	void ( *DAA_dump_ram )( struct voip_snd_s *this );
	
	// Mirror DAA functions 
	void ( *Mirror_DAA_All )( struct voip_snd_s *this, const void * all );	// 'all' is mirror_daa_priv_t *
} snd_ops_daa_t;

typedef struct {		// VDAA - ops 
	// common operation 
	int ( *open )( struct voip_snd_s *this );
	int ( *enable )( struct voip_snd_s *this, int enable );
	
	// for each snd_type 
	unsigned char ( *virtual_daa_hook_detect )( struct voip_snd_s *this );
	unsigned char ( *virtual_daa_ring_det )( struct voip_snd_s *this );
	char ( *virtual_daa_relay_switch )( struct voip_snd_s *this, unsigned char state );
	unsigned char ( *virtual_daa_ring_incoming_detect )( struct voip_snd_s *this );
} snd_ops_vdaa_t;

typedef struct {		// DECT - ops 
	// common operation 
	int ( *open )( struct voip_snd_s *this );
	int ( *enable )( struct voip_snd_s *this, int enable );
	
	// for each snd_type 
} snd_ops_dect_t;

typedef struct {		// AC - ops 
	// common operation 
	int ( *open )( struct voip_snd_s *this );
	int ( *enable )( struct voip_snd_s *this, int enable );
	
	// for each snd_type 
} snd_ops_ac_t;

typedef struct {		// common - ops 
	// common operation 
	int ( *open )( struct voip_snd_s *this );
	int ( *enable )( struct voip_snd_s *this, int enable );
	
} snd_ops_t;

//typedef union {			// SND - ops union 
//	snd_ops_com_t com;		// common ops 
//} snd_ops_t;

typedef struct {
	uint32 f_dsp_cpuid: 1;	// I know my owner 
	uint32 n_dsp_cpuid: 5;	// dsp_cpuid is what? 2^5=32 
} snd_ipc_t;

typedef union {
	struct {
		uint32 attenuateTx_6dB	:1;	// bus TX attenuate 6dB 
	} b;
	uint32 all;
} snd_flags_t;

struct voip_snd_s {
	// basic info. [given for register. readonly]
	uint32	sch;
	const char *name;	// length of name can be less than 7 for better look
	snd_type_t snd_type;
	bus_type_t bus_type_sup;
	uint16	TS1;	// Time slot 1 for PCM bus 
	uint16	TS2;	// Time slot 2 for PCM bus (16K wideband only)
	band_mode_t band_mode_sup;
	snd_flags_t snd_flags;
#ifndef __UC_EDITOR__		// help editor to parse C code 
	union 
#endif
	{
		const snd_ops_fxs_t *fxs_ops;		// snd_type = FXS
		const snd_ops_daa_t *daa_ops;		// snd_type = DAA
		const snd_ops_vdaa_t *vdaa_ops;		// snd_type = VDAA
		const snd_ops_dect_t *dect_ops;		// snd_type = DECT
		const snd_ops_ac_t *ac_ops;			// snd_type = AC	
		const snd_ops_t *snd_ops;	// common ops 
	};
	snd_ipc_t ipc;
	void *priv;		// private data for snd driver 
					// ProSLIC points to ProslicContainer_t
					// Zarlink points to RTKLineObj
	
	// runtime value 
	uint32	enabled;	// bus is enabled, so it starts to run (assigned by con_)
	
	// binding value 
	bus_type_t bus_type_bind;
	band_mode_t band_mode_bind;
	
	// binding info. 
	struct voip_con_s *con_ptr;		// point to parent console 
	
	// link list [console register manage this items]
	struct {
		struct voip_snd_s *next;
		//struct voip_snd_s *prev;
	} link;
};

// --------------------------------------------------------
// bus - pcm/iis
// --------------------------------------------------------

typedef struct {
	// common operation 
	int ( *open )( struct voip_bus_s *this );
	int ( *enable )( struct voip_bus_s *this, int enable );
	int ( *reset_TH )( struct voip_bus_s *this, int tx, int rx );
	int ( *reset_BH )( struct voip_bus_s *this, int tx, int rx );
	
	// bus enable / disable 
	//int  ( *bus_enable )( struct voip_bus_s *this );		// enable to init SLIC 
	//int  ( *bus_disable )( struct voip_bus_s *this );
	//void ( *bus_restart )( struct voip_bus_s *this );
	
	// bus configuration  
	void ( *set_timeslot )( struct voip_bus_s *this, uint32 ts1, uint32 ts2 );
	void ( *set_format )( struct voip_bus_s *this, uint32 format );	// format is BUS_DATA_FORMAT
	
} bus_ops_t;

struct voip_bus_s {
	// basic info. [given for register. readonly]
	uint32	bch;
	const char *name;	// length of name can be less than 7 for better look
	bus_type_t bus_type;
	band_mode_t band_mode_sup;
	bus_group_t bus_group;		// help to bind 8k+ mode 
	const bus_ops_t *bus_ops;
	void *priv;		// private data for bus driver 

	// runtime value 
	uint32 enabled;	// bus is enabled, so it starts to run (assigned by con_)
	bus_role_t role_var;	// role in runtime 
	
	// runtime value for 8+ mode 
	uint32 reseting;	// use by timer to do BH reset, TX: bit1, RX: bit0
	
	// binding value 
	uint16 TS1_var;
	uint16 TS2_var;		// for next generation (pure 16k wideband)
	bus_role_t role_bind;
	band_mode_t band_mode_bind;
	
	// binding info. 
	struct voip_con_s *con_ptr;		// point to parent console 
	struct voip_bus_s *bus_partner_ptr;	// 8k+ mode use 

	// link list [console register manage this items]
	struct {
		struct voip_bus_s *next;
		//struct voip_bus_s *prev;
	} link;
};

// --------------------------------------------------------
// dsp - rtk/ac
// --------------------------------------------------------

typedef struct {
	// common operation 
	int ( *open )( struct voip_dsp_s *this );
	int ( *enable )( struct voip_dsp_s *this, int enable );
	
	// isr handler 
	void ( *isr_bus_tx_start )( struct voip_dsp_s *this );
	void ( *isr_bus_tx_process_pre )( struct voip_dsp_s *this );	// once complete data  
	void ( *isr_bus_tx_process )( struct voip_dsp_s *this, uint16 *pcm_tx );	// once complete data * BUS_PERIOD
	void ( *isr_bus_tx_process_post )( struct voip_dsp_s *this );	// once complete data 
	void ( *isr_bus_rx_start )( struct voip_dsp_s *this );		// once complete data 
	void ( *isr_bus_rx_process )( struct voip_dsp_s *this, uint16 *pcm_rx, const uint16 *lec_ref );	// once complete data * BUS_PERIOD
	
	// misc. function  
	int  ( *stop_type1_fsk_cid_gen_when_phone_offhook )( struct voip_dsp_s *this );
	
	// turn on/off dsp function in ISR 
	void ( *disable_dsp_in_ISR )( struct voip_dsp_s *this );
	void ( *restore_dsp_in_ISR )( struct voip_dsp_s *this );
	
	
} dsp_ops_t;


struct voip_dsp_s {
	// basic info. [given for register. readonly]
	uint32	dch;
	const char *name;	// length of name can be less than 7 for better look
	dsp_type_t dsp_type;
	band_mode_t band_mode_sup;
	const dsp_ops_t *dsp_ops;
	void *priv;		// private data for dsp driver 
	
	// runtime value 
	uint32	enabled;	// bus is enabled, so it starts to run (assigned by con_)

	// binding gives value 
	band_mode_t band_mode_bind;
	
	// binding info. 
	struct voip_con_s *con_ptr;		// point to parent console 
	
	// link list [console register manage this items]
	struct {
		struct voip_dsp_s *next;
		//struct voip_dsp_s *prev;
	} link;
};

// --------------------------------------------------------
// con - main control 
// --------------------------------------------------------

typedef struct {
	// common operation 
	int ( *open )( struct voip_con_s *this );
	int ( *enable )( struct voip_con_s *this, int enable );
	void ( *init )( struct voip_con_s *this );
	
	int ( *suspend )( struct voip_con_s *this );
	int ( *resume )( struct voip_con_s *this );
	
	// bus fifo - handle isr 
	uint16 * ( *isr_bus_read_tx )( struct voip_con_s *this, struct voip_bus_s *p_bus, uint16 *bus_tx );
	uint16 * ( *isr_bus_write_rx_TH )( struct voip_con_s *this, struct voip_bus_s *p_bus, const uint16 *bus_rx );
	uint16 * ( *isr_bus_write_rx_BH )( struct voip_con_s *this );
	uint16 * ( *isr_bus_write_rx_lo )( struct voip_con_s *this );
		
	// bus fifo - dsp is caller 
	uint16 * ( *dsp_write_bus_tx_get_addr )( struct voip_con_s *this );
	void     ( *dsp_write_bus_tx_done )( struct voip_con_s *this );
	uint16 * ( *dsp_read_bus_rx_get_addr )( struct voip_con_s *this );
	uint16 * ( *dsp_read_bus_rx_peek_addr )( struct voip_con_s *this, int offset );
	void     ( *dsp_read_bus_rx_done )( struct voip_con_s *this );
	int      ( *dsp_read_bus_rx_get_fifo_size )( struct voip_con_s *this );
	
	// bus fifo - common 
	void ( *bus_fifo_set_tx_mute )( struct voip_con_s *this, int enable );
	void ( *bus_fifo_set_rx_mute )( struct voip_con_s *this, int enable );
	void ( *bus_fifo_clean_tx )( struct voip_con_s *this );
	void ( *bus_fifo_clean_rx )( struct voip_con_s *this );
	
	void ( *bus_fifo_flush )( struct voip_con_s *this, int tx, int rx );
	
	// lec buf - common 
	void ( *lec_buf_init_sync_point )( struct voip_con_s *this );
	void ( *lec_buf_flush )( struct voip_con_s *this );
	
	// lec buf - handle isr 
	void ( *isr_lec_buf_inc_windex )( struct voip_con_s *this );
	void ( *isr_lec_buf_tx_process )( struct voip_con_s *this, uint16 *pcm_tx, uint32 step );
	void ( *isr_lec_buf_tx_process_post )( struct voip_con_s *this );
	void ( *isr_lec_buf_inc_rindex )( struct voip_con_s *this );
	const uint16 * ( *isr_lec_buf_get_ref_addr )( struct voip_con_s *this, uint32 step );
	void ( *isr_lec_buf_sync_p )(  struct voip_con_s *this, const uint16 *pcm_rx, uint32 step );
	
	// IPC mirror request (host to DSP)
	void ( *ipc_mirror_request )( struct voip_con_s *this, uint16 category, void *data, uint16 len );
	
	// IPC RPC request (DSP to host)
	void ( *ipc_rpc_request )( struct voip_con_s *this, uint16 category, void *data, uint16 len );	
} con_ops_t;

typedef struct {
	uint32 rx_written_role;		// for 8k+ mode (16k) only
	uint32 tx_read_role;		// for 8k+ mode (16k) only
} con_bus_fifo_t;

typedef struct {
	#define MAGIC_IPC_PRIV_SHADOW	( ( void * )0xFFF425D3UL )
	void *priv;			// point to ipc_mirror_priv_data_t. NULL indicates not mirror 
	// below are valid only if priv != NULL 
	uint16 dsp_cpuid;
	uint16 dsp_cch;
} con_ipc_t;

struct voip_con_s {
	// basic info.
	uint32	cch;
	const con_ops_t *con_ops;
	
	// runtime value 
	uint32 enabled;	// bus is enabled, so it starts to run 
	uint32 suspend;	// used by suspend/resume function 
	struct voip_con_s *con_lo_ptr;	// loopback cch ptr 

	uint32 band_factor_var;	// narrowband: 1, wideband: 2 
	uint32 band_sample_var;	// narrowband: 80, wideband: 160 
	
	// binding value 
	con_share_t share_bind;
	band_mode_t band_mode_bind;
	
	// binding info. 
	struct voip_snd_s *snd_ptr;		// point to child sound 
	struct voip_bus_s *bus_ptr;		// point to child bus 
	struct voip_bus_s *bus2_ptr;	// point to child bus 2 (8k+ mode only)
	struct voip_dsp_s *dsp_ptr;		// point to child dsp 
	
	struct voip_ioc_s *ioc_led0_ptr;	// point to child ioc (LED 0)
	struct voip_ioc_s *ioc_led1_ptr;	// point to child ioc (LED 1)
	struct voip_ioc_s *ioc_relay_ptr;	// point to child ioc (SLIC Relay)
	
	// sub-system variable 
	con_bus_fifo_t fifo;
	con_ipc_t ipc;
};

typedef struct voip_ioc_s voip_ioc_t;
typedef struct voip_snd_s voip_snd_t;
typedef struct voip_bus_s voip_bus_t;
typedef struct voip_dsp_s voip_dsp_t;
typedef struct voip_con_s voip_con_t;

// ========================================================
// function  
// ========================================================

// register
extern int register_voip_ioc( voip_ioc_t voip_ioc[], int num );
extern int register_voip_snd( voip_snd_t voip_snd[], int num );
extern int register_voip_bus( voip_bus_t voip_bus[], int num );
extern int register_voip_dsp( voip_dsp_t voip_dsp[], int num );

// find
extern voip_snd_t *find_snd_with_snd_type( snd_type_t snd_type );
extern voip_bus_t *find_bus_with_bus_type( bus_type_t bus_type );
extern voip_bus_t *find_bus_with_bus_type_band_mode( bus_type_t bus_type, band_mode_t band_mode );
extern voip_dsp_t *find_dsp_with_band_mode( band_mode_t band_mode );
extern voip_ioc_t *find_first_ioc( void );

// timeslot assistant 
extern int get_snd_free_timeslot( void );

// access control channel (CONST)
extern const voip_con_t *get_const_con_ptr( uint32 cch );


// ========================================================
// variables   
// ========================================================

// ========================================================
// debug message printk   
// ========================================================
#define CHK_MSG		printk

// ========================================================
// declare macro for convenient usage   
// ========================================================

#define DECLARE_SND_FROM_CON( con )				\
	voip_snd_t * const p_snd = con ->snd_ptr;	\
	const snd_ops_com_t * const p_snd_ops = sndOpsSafelyFromSnd( p_snd )

#define DECLARE_SND_DAA_FROM_CON( con )			\
	voip_snd_t * const p_snd = con ->snd_ptr;	\
	const snd_ops_daa_t * const p_daa_ops = daaOpsSafelyFromSnd( p_snd )

#define DECLARE_SND_FXS_FROM_CON( con )			\
	voip_snd_t * const p_snd = con ->snd_ptr;	\
	const snd_ops_fxs_t * const p_fxs_ops = fxsOpsSafelyFromSnd( p_snd )

#define DECLARE_BUS_FROM_CON( con )				\
	voip_bus_t * const p_bus = con ->bus_ptr;	\
	const bus_ops_t * const p_bus_ops = busOpsSafelyFromBus( p_bus )

#define DECLARE_DSP_FROM_CON( con )				\
	voip_dsp_t * const p_dsp = con ->dsp_ptr;	\
	const dsp_ops_t * const p_dsp_ops = dspOpsSafelyFromDsp( p_dsp )


#endif // __CON_REGISTER_H__

