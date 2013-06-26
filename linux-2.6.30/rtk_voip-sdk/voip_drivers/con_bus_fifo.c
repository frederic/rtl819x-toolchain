#include <linux/string.h>
#include "rtk_voip.h"
#include "mem.h"
#include "voip_types.h"
#include "con_register.h"
#include "section_def.h"

// in order to share with two DSPs (used by dsp_xxx_handler_start)
//#ifdef REDUCE_PCM_FIFO_MEMCPY
//__pcmifdata14 static uint32* pRxBufTmp; // For reducing memcpy
//__pcmifdata15 static uint32* pTxBufTmp;
//#else
//__pcmifdata02 static uint32 RxBufTmp[MAX_VOIP_CH_NUM][RX_BUF_SIZE/4];
//__pcmifdata03 static uint32 TxBufTmp[MAX_VOIP_CH_NUM][TX_BUF_SIZE/4];
//#endif

extern short speech_16k[/*66948 - 1000*8*/];  /* 0x0000c700 */

#ifdef SUPPORT_PCM_FIFO
// NOTE: Realtek's LEC access these fifo in size of PCM_PERIOD 
__attribute__((aligned(8)))
__pcmifdata01 static uint16 rx_fifo[CON_CH_NUM][PCM_FIFO_SIZE*RX_BUF_SIZE/(sizeof(uint16))*MAX_BAND_FACTOR];
__pcmifdata04 static uint16 tx_fifo[CON_CH_NUM][PCM_FIFO_SIZE*TX_BUF_SIZE/(sizeof(uint16))*MAX_BAND_FACTOR];
__pcmifdata11 static unsigned char tx_fifo_cnt_w[CON_CH_NUM]={0}, tx_fifo_cnt_r[CON_CH_NUM]={0};
__pcmifdata12 static unsigned char rx_fifo_cnt_w[CON_CH_NUM]={0}, rx_fifo_cnt_r[CON_CH_NUM]={0};
__pcmifdata13 static int fifo_start[CON_CH_NUM];
#endif

static int tx_mute[CON_CH_NUM] = {[0 ... CON_CH_NUM-1]=0};
static int rx_mute[CON_CH_NUM] = {[0 ... CON_CH_NUM-1]=0};

static unsigned char PCM_RX_rx_w[CON_CH_NUM];		// scope is once interrupt 
static int need_PCM_RX[CON_CH_NUM];

#ifdef SUPPORT_PCM_FIFO
static void bus_FIFO_Init(unsigned int cch, int tx, int rx)
{	
	////////////////////////////////////////////////// 
	// TX part 
	if( !tx )
		goto label_tx_done;
		
	tx_fifo_cnt_w[cch]=1; // avoid TE when start pcm
	tx_fifo_cnt_r[cch]=0;
	fifo_start[cch]=0;

label_tx_done:

	////////////////////////////////////////////////// 
	// RX part 
	if( !rx )
		goto label_rx_done;
		
#if DTMF_REMOVAL_FORWARD_SIZE == 3
	/* If odd removal length, one is given as initial value. */
	extern unsigned char dtmf_removal_flag[];
	
	if (dtmf_removal_flag[cch] == 1)
		rx_fifo_cnt_w[cch]=3; 
	else
		rx_fifo_cnt_w[cch]=0; 
#elif PCM_PERIOD == 1
	rx_fifo_cnt_w[cch]=0; 
#else
	/* If even removal length, zero is admited. */
	//???
	rx_fifo_cnt_w[cch]=0; 
#endif
	rx_fifo_cnt_r[cch]=0;

label_rx_done:
	;
}
#endif

void bus_fifo_flush( voip_con_t *this, int tx, int rx )
{
	const uint32 cch = this ->cch;
	
	if( tx ) {
		//tx_mute[ cch ] = 0;
		this ->fifo.tx_read_role = 0;
	}
	
	if( rx ) {
		//rx_mute[ cch ] = 0;
		this ->fifo.rx_written_role = 0;
	}

#ifdef SUPPORT_PCM_FIFO
	bus_FIFO_Init( cch, tx, rx );
#endif	
}

// handle isr 
void isr_bus_reset_need_rx_vars( void )
{
	memset( need_PCM_RX, 0, sizeof( need_PCM_RX ) );
}

__pcmIsr02
uint16 * isr_bus_read_tx( voip_con_t *this, voip_bus_t *p_bus, uint16 *bus_tx )
{
	// if return NULL, caller will fill bus_tx by itself. 
	int i, j;
	const uint32 cch = this ->cch;
	voip_dsp_t * const p_dsp = this ->dsp_ptr;
	uint16 *tx;
	unsigned char tx_r;
	
	
	// ----------------------------------------------------
	// update read role & tx_r 
	switch( p_bus ->role_var ) {
	case BUS_ROLE_MAIN:
		this ->fifo.tx_read_role |= 0x01;
		break;
	case BUS_ROLE_MATE:
		this ->fifo.tx_read_role |= 0x02;
		break;
	default:
		break;
	}
	
	if( ( this ->fifo.tx_read_role & 0x03 ) == 0x03 ) {
		this ->fifo.tx_read_role = 0;
		//printk( "-" );
	}
	
	// ----------------------------------------------------
	// call dsp once PCM tx interrupt 
	if( this ->fifo.tx_read_role == 0 ) {
		this ->con_ops ->isr_lec_buf_inc_windex( this );
		p_dsp ->dsp_ops ->isr_bus_tx_start( p_dsp );
	}
	
	// ----------------------------------------------------
	// check tx fifo empty
	if( tx_fifo_cnt_r[ cch ] == tx_fifo_cnt_w[ cch ] )
		return NULL;	// TX FIFO is empty!
	
	// fifo delay start 
	if( fifo_start[ cch ] == 0 ) {
		// Let RX/TX FIFO has one extra page to avoid TX FIFO empty!
		if ( tx_fifo_cnt_w[ cch ] >= TX_FIFO_START_NUM ) { // suppose tx_fifo_cnt_r[ cch ] is 0 at the begining of a call!!
			fifo_start[ cch ] = 1;
			//printk("tx fifo_start(%d,%d)\n", tx_fifo_cnt_r[ cch ], tx_fifo_cnt_w[ cch ]);
		} else
			return NULL;
	}
	
	// ----------------------------------------------------
	// normal case - fifo_start[ sch ] == 1
	
	// process pre 
	if( this ->fifo.tx_read_role == 0 ) {
		p_dsp ->dsp_ops ->isr_bus_tx_process_pre( p_dsp );
	}
	
	// main process 
	tx_r = tx_fifo_cnt_r[ cch ];
	
	for (i=0; i < PCM_PERIOD; i++) 
	{
		if ( tx_fifo_cnt_w[ cch ] == tx_r ) {
			// Empty
			printk("te(%d-%d) ", cch, i);
			return NULL;
		}
		
		tx = &tx_fifo[ cch ][ tx_r * this ->band_sample_var ];
		
		if( this ->fifo.tx_read_role == 0 ) {
			p_dsp ->dsp_ops ->isr_bus_tx_process( p_dsp, tx );
			this ->con_ops ->isr_lec_buf_tx_process( this, tx, i );
		}
		
		switch( p_bus ->role_var ) {
		case BUS_ROLE_SINGLE:		// normal
#ifdef USE_MEM64_OP
			memcpy64s( ( void * )bus_tx+(i*PCM_PERIOD_10MS_SIZE), tx, PCM_PERIOD_10MS_SIZE/2 * this ->band_factor_var);
#else
			memcpy( ( void * )bus_tx+(i*PCM_PERIOD_10MS_SIZE), tx, PCM_PERIOD_10MS_SIZE * this ->band_factor_var);
#endif
			break;
			
		case BUS_ROLE_MAIN:			// main of 8k+ mode 
			for( j = 0; j < 80; j ++ ) { 
#if 1
				*( bus_tx + i * 80 + j ) = *( tx + j * 2 );	
#else
				static int pos_16k_main = 0;
				
				pos_16k_main += 2;
				if( pos_16k_main >= 0x0000c700 / 2 )
					pos_16k_main = 0;
					
				*( bus_tx + i * 80 + j ) = speech_16k[ pos_16k_main ];
#endif
			}
			break;
			
		case BUS_ROLE_MATE:			// mate of 8k+ mode 
			for( j = 0; j < 80; j ++ ) {
#if 1
				*( bus_tx + i * 80 + j ) = *( tx + 1 + j * 2 );
#else
				static int pos_16k_mate = 1;
				
				pos_16k_mate += 2;
				if( pos_16k_mate >= 0x0000c700 / 2 )
					pos_16k_mate = 1;
					
				*( bus_tx + i * 80 + j ) = speech_16k[ pos_16k_mate ];
#endif
			}
			break;
		}
		
		tx_r = ( tx_r + 1 ) % PCM_FIFO_SIZE;
	}	
	
	// process post  
	if( this ->fifo.tx_read_role == 0 ) {
		this ->con_ops ->isr_lec_buf_tx_process_post( this );
		p_dsp ->dsp_ops ->isr_bus_tx_process_post( p_dsp );
	}
	
	if( this ->fifo.tx_read_role == 0x0 )
		tx_fifo_cnt_r[ cch ] = tx_r;
	
	if( tx_mute[ cch ] )
		return NULL;
	
	return bus_tx;
}

__pcmIsr02
uint16 * isr_bus_write_rx_TH( voip_con_t *this, voip_bus_t *p_bus, const uint16 *bus_rx )
{
	/* Top Half is running in cli, so do simple job */
	int i, j;
	const uint32 cch = this ->cch;
	voip_dsp_t * const p_dsp = this ->dsp_ptr;
	uint16 *rx;
	unsigned char rx_w;
#if 0
	static int pos_16k = 0;
#endif
	
	// ----------------------------------------------------
	// call dsp once PCM rx interrupt 
	if( this ->fifo.rx_written_role == 0x0 ) {
		this ->con_ops ->isr_lec_buf_inc_rindex( this );
		p_dsp ->dsp_ops ->isr_bus_rx_start( p_dsp );
	}
		
	rx_w = rx_fifo_cnt_w[ cch ];
	
	for( i = 0; i < PCM_PERIOD; i ++ ) 
	{
		if( ( rx_w + 1 ) % PCM_FIFO_SIZE == rx_fifo_cnt_r[ cch ] ) {
			// full
			//printk("rf(%d-%d) ", cch, i);
			return NULL;
		}
		
		rx = &rx_fifo[ cch ][ rx_w * this ->band_sample_var ];
		
		switch( p_bus ->role_var ) {
		case BUS_ROLE_SINGLE:		// normal
#ifdef USE_MEM64_OP
			memcpy64s( rx, bus_rx+(i*PCM_PERIOD_10MS_SIZE/2), PCM_PERIOD_10MS_SIZE/2 * this ->band_factor_var);
#else
			memcpy( rx, bus_rx+(i*PCM_PERIOD_10MS_SIZE/2), PCM_PERIOD_10MS_SIZE * this ->band_factor_var);
#endif
			break;
		case BUS_ROLE_MAIN:
#if 1
			for( j = 0; j < 80; j ++ )
				*( rx + j * 2 ) = *( bus_rx + (i*PCM_PERIOD_10MS_SIZE/2) + j );
#else
			if( cch == 0 )
				break;
			
			for( j = 0; j < 80; j ++ ) {
				*( rx + j * 2 ) = speech_16k[ pos_16k + 2 * j ];
			}
#endif
			break;
		case BUS_ROLE_MATE:
#if 1
			for( j = 0; j < 80; j ++ )
				*( rx + j * 2 + 1 ) = *( bus_rx + (i*PCM_PERIOD_10MS_SIZE/2) + j );
#else
			if( cch == 0 )
				break;
				
			for( j = 0; j < 80; j ++ ) {
				*( rx + j * 2 + 1 ) = speech_16k[ pos_16k + j * 2 + 1 ];
			}
			
			pos_16k += 160;
			
			if( pos_16k >= 0x0000c700 / 2 )
				pos_16k = 0;
#endif
			break;
		}
		
		rx_w = ( rx_w + 1 ) % PCM_FIFO_SIZE;
	}
	
	// update written role & rx_w 
	switch( p_bus ->role_var ) {
	case BUS_ROLE_MAIN:
		this ->fifo.rx_written_role |= 0x01;
		break;
	case BUS_ROLE_MATE:
		this ->fifo.rx_written_role |= 0x02;
		break;
	default:
		break;
	}

	if( ( this ->fifo.rx_written_role & 0x03 ) == 0x03 ) {
		this ->fifo.rx_written_role = 0;
	}
	
	rx = &rx_fifo[ cch ][ rx_fifo_cnt_w[ cch ] * this ->band_sample_var ];
	
	if( this ->fifo.rx_written_role == 0x0 ) {
		// BH will check this variable to do more 
		PCM_RX_rx_w[ cch ] = rx_fifo_cnt_w[ cch ];
		need_PCM_RX[ cch ] = 1;
		
		rx_fifo_cnt_w[ cch ] = rx_w;
	}
	
	//printk( "w%d,%d ", cch, rx_fifo_cnt_w[ cch ] );
	
	return rx;
}

__pcmIsr02
uint16 * isr_bus_write_rx_BH( voip_con_t *this )
{
	/* Bottom Half can be more job */
	int i;
	const uint32 cch = this ->cch;
	voip_dsp_t * const p_dsp = this ->dsp_ptr;
	unsigned char rx_w;
	uint16 *rx;
	const uint16 *lec_ref;
	
	if( !need_PCM_RX[ cch ] )
		return NULL;
	
	need_PCM_RX[ cch ] = 0;
	rx_w = PCM_RX_rx_w[ cch ];
	
	for (i=0; i < PCM_PERIOD; i++) 
	{
		if ( ( rx_w + 1 ) % PCM_FIFO_SIZE == rx_fifo_cnt_r[ cch ] ) {
			// full
			printk("rf_BH(%d-%d) ", cch, i);
			return NULL;
		}
		
		// get pcm_rx and lec_tx_ref
		rx = &rx_fifo[ cch ][ rx_w * this ->band_sample_var ];
		lec_ref = this ->con_ops ->isr_lec_buf_get_ref_addr( this, i );
		
		// check sync p
#ifdef LEC_G168_ISR_SYNC_P
		this ->con_ops ->isr_lec_buf_sync_p( this, rx, i );
#endif
		
		// rx process 
		p_dsp ->dsp_ops ->isr_bus_rx_process( p_dsp, rx, lec_ref );
		
		rx_w = ( rx_w + 1 ) % PCM_FIFO_SIZE;
	}
	
	// help to call lo 
	this ->con_ops ->isr_bus_write_rx_lo( this );
	
	return rx;
}

uint16 * isr_bus_write_rx_lo( voip_con_t *this )
{
	// special process for loopback (be dsp role) 
	// loopback is only pass this ->rx to that ->tx, so 
	// this ->tx comes from that ->rx. 
	voip_con_t *that = this ->con_lo_ptr;
	uint16 *rx, *tx;
	uint32 i;
	
	if( that == NULL || that ->enabled == 0 )
		return NULL;
	
	// get this rx pointer 
	rx = this ->con_ops ->dsp_read_bus_rx_get_addr( this );
	
	if( rx == NULL )
		return NULL;
	
	// get that tx pointer 
	tx = that ->con_ops ->dsp_write_bus_tx_get_addr( that );
	
	if( tx == NULL )
		return NULL;
	
	// check band mode 
	if( this ->band_sample_var != that ->band_sample_var ) {
		printk( "band mode not match!\n" );
		return NULL;
	}
	
	// start to copy 
	for( i = 0; i < this ->band_sample_var; i ++ )
		tx[ i ] = rx[ i ];
	
	// done
	this ->con_ops ->dsp_read_bus_rx_done( this );
	that ->con_ops ->dsp_write_bus_tx_done( that );
	
	return rx;
}

// dsp is caller 
uint16 * dsp_write_bus_tx_get_addr( voip_con_t *this )
{
	const uint32 cch = this ->cch;
	unsigned char tx_w = tx_fifo_cnt_w[ cch ];
	
	if( ( tx_w + 1 ) % PCM_FIFO_SIZE == tx_fifo_cnt_r[ cch ] ) {
		// full
#if 1
		static uint32 a = 0;
		if( a < 100 ) {
			printk("tf(%d-%d/%d) ", cch, tx_w, rx_fifo_cnt_w[ cch ]);
			a ++;
		}
#else
		printk("tf(%d-%d) ", cch, tx_w);
#endif
		return NULL;
	}
	
	return &tx_fifo[ cch ][ tx_w * this ->band_sample_var ];
}

void dsp_write_bus_tx_done( voip_con_t *this )
{
	const uint32 cch = this ->cch;
	unsigned char tx_w = tx_fifo_cnt_w[ cch ];
	
	tx_w = ( tx_w + 1 ) % PCM_FIFO_SIZE;
	
	if( tx_w == tx_fifo_cnt_r[ cch ] ) {
		// full
		printk("tf_done(%d) ", cch);
		return;
	}
	
	tx_fifo_cnt_w[ cch ] = tx_w;
}

uint16 * dsp_read_bus_rx_get_addr( voip_con_t *this )
{
	const uint32 cch = this ->cch;
	unsigned char rx_r = rx_fifo_cnt_r[ cch ];
	uint16 *rx;
	
	if( rx_r == rx_fifo_cnt_w[ cch ] ) {
		// empty
		//printk( "re(%d) ", cch );
		return NULL;
	}
	
	rx = &rx_fifo[ cch ][ rx_r * this ->band_sample_var ];
	
	if( rx_mute[ cch ] )
		memset( rx, 0, 80 * 2 );
	
	return rx;
}

uint16 * dsp_read_bus_rx_peek_addr( voip_con_t *this, int offset )
{
	const uint32 cch = this ->cch;
	unsigned char rx_r = rx_fifo_cnt_r[ cch ];
	
	while( 1 ) {
		if( rx_r == rx_fifo_cnt_w[ cch ] )	// empty 
			return NULL;
		
		if( offset <= 0 )
			break;
		
		offset --;
		rx_r = ( rx_r + 1 ) % PCM_FIFO_SIZE;
	} 
	
	return &rx_fifo[ cch ][ rx_r * this ->band_sample_var ];
}

void dsp_read_bus_rx_done( voip_con_t *this )
{
	const uint32 cch = this ->cch;
	unsigned char rx_r = rx_fifo_cnt_r[ cch ];
	
	if( rx_r == rx_fifo_cnt_w[ cch ] ) {
		// empty
		printk( "re_done(%d) ", cch );
		return;
	}
	
	rx_fifo_cnt_r[ cch ] = ( rx_r + 1 ) % PCM_FIFO_SIZE;
}

int dsp_read_bus_rx_get_fifo_size( voip_con_t *this )
{
	const uint32 cch = this ->cch;
	unsigned char rx_r = rx_fifo_cnt_r[ cch ];
	unsigned char rx_w = rx_fifo_cnt_w[ cch ];
	
	if( rx_w >= rx_r )
		return rx_w - rx_r;
	else {
		return ( PCM_FIFO_SIZE - rx_r ) + rx_w;
	}
}

void bus_fifo_set_tx_mute( voip_con_t *this, int enable )
{
	const uint32 cch = this ->cch;
	
	tx_mute[ cch ] = enable;
}

void bus_fifo_set_rx_mute( voip_con_t *this, int enable )
{
	const uint32 cch = this ->cch;
	
	rx_mute[ cch ] = enable;
}

void bus_fifo_clean_tx( voip_con_t *this )
{
	const uint32 cch = this ->cch;
	
	memset( &tx_fifo[ cch ][ 0 ], 0, PCM_FIFO_SIZE*TX_BUF_SIZE);
}

void bus_fifo_clean_rx( voip_con_t *this )
{
	const uint32 cch = this ->cch;
	
	memset( &rx_fifo[ cch ][ 0 ], 0, PCM_FIFO_SIZE*RX_BUF_SIZE);
}

