#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_control.h"
#include "voip_debug.h"
#include "voip_errno.h"
#include "voip_init.h"
#include "voip_proc.h"
#include "voip_timer.h"

#include "voip_mgr_events.h"

// ----------------------------------------------------------------------
// Queue main routine & general purpose functions 
// ----------------------------------------------------------------------

// multithread protect 
#define VEQ_ENTER_CRITICAL_SECTION()	//unsigned long __flags;	save_flags(__flags); cli();
#define VEQ_LEAVE_CRITICAL_SECTION()	//restore_flags(__flags);

#define VOIP_EVENT_QUEUE_ENTITY_NUM		20

#define MULTIPLE_EVENT_QUEUE			1

#define NO_SINGLE_QUEUE_MAGIC			( ( void * )-1 )

typedef struct {
	VoipEventID id;
#ifndef MULTIPLE_EVENT_QUEUE
	VoipEventMask mask;	// In one of multi-queues, mask is always the same 
#endif
	uint32 p0;
	uint32 p1;
	timetick_t time;
} voip_event_entity_t;

struct voip_event_queue_s;

typedef int ( *extra_process_event_out )( struct voip_event_queue_s *, TstVoipEvent * );

typedef struct voip_event_queue_s {
	// basic info 
	const char *name;
	// extra process 
	struct {
		// call this fn if event exist in specified multi-queues, 
		// or no event but single queue query. 
		extra_process_event_out fn;
		void *priv;
	} extra;
	// entity 
	voip_event_entity_t entity[ VOIP_EVENT_QUEUE_ENTITY_NUM ];
	uint32 ri;
	uint32 wi;
	// counter  
	uint32 drop;	// drop due to full 
	uint32 spin;	// spin due to empty
} voip_event_queue_t;

static voip_event_queue_t voip_event_queue[ CON_CH_NUM ][ VOIP_EVENT_QUEUE_TOTAL_NUM ];

static int _voip_mgr_event_in( voip_event_queue_t *queue, const voip_event_entity_t *entity )
{
	uint32 wi, wi_next;
	int ret = 0;
	
	VEQ_ENTER_CRITICAL_SECTION();
	
	// wi 
	wi = queue ->wi;
	
	// wi_next 
	wi_next = wi + 1;
	
	if( wi_next == VOIP_EVENT_QUEUE_ENTITY_NUM )
		wi_next = 0;
	
	// check full 
	if( queue ->ri == wi_next ) {
		queue ->drop ++;
		ret = -EVOIP_EVENT_FULL;
		goto label_end;
	}
	
	// write it 
	queue ->entity[ wi ] = *entity;
	
	// update wi 
	queue ->wi = wi_next;
	
label_end:
	VEQ_LEAVE_CRITICAL_SECTION();
	
	return ret;
}

static int _voip_mgr_event_out( voip_event_queue_t *queue, voip_event_entity_t *entity )
{
	uint32 ri;
	int ret = 0;
	
	VEQ_ENTER_CRITICAL_SECTION();
	
	// ri 
	ri = queue ->ri;
	
	// check empty 
	if( queue ->wi == ri ) {
		queue ->spin ++;
		ret = -EVOIP_EVENT_EMPTY;
		goto label_end;
	}
	
	// read it
	*entity = queue ->entity[ ri ];
	
	// ri_next
	ri ++;
	
	if( ri == VOIP_EVENT_QUEUE_ENTITY_NUM )
		ri = 0;
	
	// update ri 
	queue ->ri = ri;
	
label_end:
	VEQ_LEAVE_CRITICAL_SECTION();
	
	return ret;
}

static void _voip_mgr_event_flush( voip_event_queue_t *queue )
{
	VEQ_ENTER_CRITICAL_SECTION();
	
	queue ->ri = queue ->wi;
	
	VEQ_LEAVE_CRITICAL_SECTION();
}

typedef struct {
	uint32 queue_idx;
	VoipEventType queue_type;
	VoipEventMask queue_mask;
} event_queue_idx_map_t;

typedef int ( *filter_action )( const event_queue_idx_map_t *map,
							voip_event_queue_t *queue, void *priv );

static int do_action_on_filtered_queues( voip_event_queue_t n_queue[], 
				VoipEventType filter, VoipEventMask mask,
				filter_action action, void *priv,
				voip_event_queue_t **single_queue )
{
	/* 
	 * n_queue[] is not constant because pass this pointer to action, and 
	 * this pointer will be used to retrieve an event. 
	 */
	
	static const event_queue_idx_map_t event_queue_idx_map[] = {
		{ VOIP_EVENT_IDX_DTMF_DIRTDM,	VET_DTMF, 		VEM_DIRTDM,	},
		{ VOIP_EVENT_IDX_DTMF_DIRIP,	VET_DTMF, 		VEM_DIRIP,	},
		{ VOIP_EVENT_IDX_HOOK, 			VET_HOOK,		0,			},
		{ VOIP_EVENT_IDX_ENERGY,		VET_ENERGY,		0,			}, 
		{ VOIP_EVENT_IDX_DECT, 			VET_DECT, 		0,			}, 
		{ VOIP_EVENT_IDX_FAXMDM, 		VET_FAXMDM, 	0,			}, 
		{ VOIP_EVENT_IDX_RFC2833_MID0, 	VET_RFC2833,	VEM_MID0, 	},
		{ VOIP_EVENT_IDX_RFC2833_MID1, 	VET_RFC2833, 	VEM_MID1, 	},
		{ VOIP_EVENT_IDX_DSP_MID0, 		VET_DSP, 		VEM_MID0, 	},
		{ VOIP_EVENT_IDX_DSP_MID1, 		VET_DSP, 		VEM_MID1, 	},
	};
	
	CT_ASSERT( VOIP_EVENT_QUEUE_TOTAL_NUM == 
				( sizeof( event_queue_idx_map ) / sizeof( event_queue_idx_map[ 0 ] ) ) );
	
	int i;
	voip_event_queue_t *queue;
	const event_queue_idx_map_t *map;
	uint32 ri;
	int ret = 0;
	
	// scan all queues 
	for( i = 0; i < VOIP_EVENT_QUEUE_TOTAL_NUM; i ++ ) {
		
		// get map ptr 
		map = &event_queue_idx_map[ i ];
		
		// get queue ptr 
		queue = &n_queue[ map ->queue_idx ];
		
		// filter - type 
		if( !( map ->queue_type & filter ) )
			continue;
		
		// filter - mask 
		if( ( map ->queue_mask ) &&
			!( map ->queue_mask & mask ) )
		{
			continue;
		}
		
		// fill single_queue info 
		if( single_queue )
			*single_queue = ( *single_queue == NO_SINGLE_QUEUE_MAGIC ? queue : NULL );
		
		// check empty 
		ri = queue ->ri;
		
		if( ri == queue ->wi )
			continue;
			
		// do action 
		if( action )
			ret = action( map, queue, priv );
		
		if( ret < 0 )
			return ret;
	}	
	
	return ret;
}

typedef struct {
	VoipEventQueueIdx idx;
	uint32 time;
	voip_event_queue_t *queue;
#ifdef MULTIPLE_EVENT_QUEUE
	VoipEventMask mask;
#endif
} out_candidate_t;

static int _voip_mgr_event_out_cmp_action( 
			const event_queue_idx_map_t *map, voip_event_queue_t *queue, 
			void *priv )
{
	out_candidate_t *candidate = priv;
	const uint32 ri = queue ->ri;
	
	// compare with candidate 
	if( candidate ->idx == -1 || 
		timetick_after( ( unsigned long )candidate ->time, 
						( unsigned long )queue ->entity[ ri ].time ) )
	{
		candidate ->idx = map ->queue_idx;
		candidate ->time = queue ->entity[ ri ].time;
		candidate ->queue = queue;
#ifdef MULTIPLE_EVENT_QUEUE
		candidate ->mask = map ->queue_mask;
#endif
	}
	
	return 0;
}

static int _voip_mgr_event_out_flush_action( 
			const event_queue_idx_map_t *map, voip_event_queue_t *queue, 
			void *priv )
{
	_voip_mgr_event_flush( queue );
	
	return 0;
}

int voip_mgr_event_in( uint32 chid, VoipEventQueueIdx idx, VoipEventMask mask, 
						VoipEventID id, uint32 p0, uint32 p1 )
{
	extern void mgr_wakeup( void );
	
	voip_event_entity_t entity;
	int ret;
	
	//printk( "voip_mgr_event_in chid=%d id=%08x\n", chid, id );
	
	entity.id = id;
#ifndef MULTIPLE_EVENT_QUEUE
	entity.mask = mask;
#endif
	entity.p0 = p0;
	entity.p1 = p1;
	entity.time = timetick;
	
	ret = _voip_mgr_event_in( &voip_event_queue[ chid ][ idx ], &entity );
	
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP	
	mgr_wakeup();
#endif
	
	return ret;
}

int voip_mgr_event_in_packed( const TstVoipEvent *pVoipEvent )
{
	const uint32 chid = pVoipEvent ->ch_id;
	const VoipEventType filter = pVoipEvent ->type;
	const VoipEventMask mask = pVoipEvent ->mask;
	VoipEventQueueIdx idx;
	
	voip_event_queue_t *single_queue = NO_SINGLE_QUEUE_MAGIC;

#if 0	
	printk( "voip_mgr_event_in_packed\n" );
	printk( "\tfilter=%08X\n", filter );
	printk( "\tmask=%08X\n", mask );
	printk( "\tchid=%d\n", chid );
#endif
	
	do_action_on_filtered_queues( voip_event_queue[ chid ], 
				filter, mask, 
				NULL, NULL,
				&single_queue );
	
	if( single_queue == NULL || single_queue == NO_SINGLE_QUEUE_MAGIC )
		return -EVOIP_EVENT_QUEUE_ERR;
	
	idx = single_queue - &voip_event_queue[ chid ][ 0 ];

#if 0	
	printk( "voip_mgr_event_in_packed\n" );
	printk( "\tsingle_queue=%p \n", single_queue );
	printk( "\t&voip_event_queue[ chid ][ 0 ]=%p\n", &voip_event_queue[ chid ][ 0 ] );
	printk( "\tidx=%d\n", idx );
#endif
	
	return voip_mgr_event_in( chid, idx, mask, 
							pVoipEvent ->id, pVoipEvent ->p0, pVoipEvent ->p1 );
}

int voip_mgr_event_out( TstVoipEvent *pVoipEvent )
{
	// parameter expandsion 
	const uint32 chid = pVoipEvent ->ch_id;
	const VoipEventType filter = pVoipEvent ->type;
	VoipEventMask * const mask = &pVoipEvent ->mask;
	VoipEventID * const id = &pVoipEvent ->id;
	uint32 * const p0 = &pVoipEvent ->p0;
	uint32 * const p1 = &pVoipEvent ->p1;
	uint32 * const time = &pVoipEvent ->time;
	
	// variable to store info 
	out_candidate_t candidate = { .idx = -1, };
	voip_event_entity_t entity;
	voip_event_queue_t *single_queue = NO_SINGLE_QUEUE_MAGIC;
	
	int ret;
	
	// do cmp candidate on filtered queues 
	ret = do_action_on_filtered_queues( voip_event_queue[ chid ], 
				filter, *mask, 
				_voip_mgr_event_out_cmp_action, &candidate,
				&single_queue );
	
	if( ret < 0 )
		goto label_out_error;
	
	// no events 
	if( candidate.idx == -1 ) {
		// ret = 0
		goto label_out_no_event;
	}
	
	// fill entity 
	ret = _voip_mgr_event_out( candidate.queue, &entity );
	
	if( ret < 0 )
		goto label_out_error;
	
	// put entity to function output parameters 
#ifndef MULTIPLE_EVENT_QUEUE	
	*mask = entity.mask;
#else
	*mask = candidate.mask;
#endif
	*id = entity.id;
	*p0 = entity.p0;
	*p1 = entity.p1;
	*time = entity.time;
	
	// extra process (event exists)
	if( candidate.queue ->extra.fn )
		( *candidate.queue ->extra.fn )( candidate.queue, pVoipEvent );
	
	return 0;

label_out_no_event:
	*id = VEID_NONE;
	
	// extra process (no event)
	if( single_queue != NO_SINGLE_QUEUE_MAGIC && single_queue && 
		single_queue ->extra.fn )
	{
		( *single_queue ->extra.fn )( single_queue, pVoipEvent );
	}
	
	return 0;
	
label_out_error:
	*id = VEID_NONE;
	
	return ret;
}

int voip_mgr_event_flush( TstFlushVoipEvent *pFlushVoipEvent )
{
	extern void Reset_Fax_Check_Flags( uint32 ch_id );
	
	// parameter expandsion 
	const uint32 chid = pFlushVoipEvent ->ch_id;
	const VoipEventType filter = pFlushVoipEvent ->type;
	VoipEventMask const mask = pFlushVoipEvent ->mask;
	
	int ret;
	
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// additional reset 
	if( filter & VET_FAXMDM )
		Reset_Fax_Check_Flags( chid );
#endif
	
	// do flush on filtered queues 
	ret = do_action_on_filtered_queues( voip_event_queue[ chid ], 
				filter, mask, 
				_voip_mgr_event_out_flush_action, NULL,
				NULL );
	
	return ret;
}

// ----------------------------------------------------------------------
// Extra process for evnet out  
// ----------------------------------------------------------------------

typedef struct {
	VoipEventID	prev_id;
} extra_hook_priv_t;

static extra_hook_priv_t extra_hook_priv[ CON_CH_NUM ];

static int extra_process_event_out_hook( voip_event_queue_t *queue, TstVoipEvent *pVoipEvent )
{
#ifndef AUDIOCODES_VOIP
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#ifdef SW_DTMF_CID
	extern void cid_dtmf_gen_init(int32 chid);
#endif
#endif
#endif
	
	extra_hook_priv_t * const extra_hook_priv = queue ->extra.priv;
	
	// ----------------------------------------------------------------
	// copy from hook_out() - maintain xxx_STILL_yyy
	if ( pVoipEvent ->id == VEID_NONE ) // FIFO empty
	{
		switch( extra_hook_priv ->prev_id ) {
		
		// FXO case 
		case VEID_HOOK_FXO_ON_HOOK:
			pVoipEvent ->id = VEID_HOOK_FXO_STILL_ON_HOOK;
			break;
		
		case VEID_HOOK_FXO_FLASH_HOOK:
			pVoipEvent ->id = VEID_HOOK_FXO_STILL_OFF_HOOK;
			break;
			
		case VEID_HOOK_FXO_OFF_HOOK:
			pVoipEvent ->id = VEID_HOOK_FXO_STILL_OFF_HOOK;
			break;
		
		// FXS case 
		case VEID_HOOK_PHONE_ON_HOOK:
			pVoipEvent ->id = VEID_HOOK_PHONE_STILL_ON_HOOK;
			break;
		
		case VEID_HOOK_PHONE_FLASH_HOOK:
			pVoipEvent ->id = VEID_HOOK_PHONE_STILL_OFF_HOOK;
			break;
		
		case VEID_HOOK_PHONE_OFF_HOOK:
			pVoipEvent ->id = VEID_HOOK_PHONE_STILL_OFF_HOOK;
			break;
		
		default:
			PRINT_MSG( "Unkndown prev ID (%08X) on chid=%d\n", 
								extra_hook_priv ->prev_id,
								pVoipEvent ->ch_id );
			return 0;
			break;
		}
	}
	else
	{
		switch( pVoipEvent ->id ) {
		// FXO case 
		case VEID_HOOK_FXO_ON_HOOK:
		case VEID_HOOK_FXO_FLASH_HOOK:
		case VEID_HOOK_FXO_OFF_HOOK:
		// FXS case
		case VEID_HOOK_PHONE_ON_HOOK:
		case VEID_HOOK_PHONE_FLASH_HOOK:
		case VEID_HOOK_PHONE_OFF_HOOK:
			extra_hook_priv ->prev_id = pVoipEvent ->id;
			break;
			
		default:
			break;
		}
	}
	
	// ----------------------------------------------------------------
	// copy from do_mgr_VOIP_MGR_SLIC_HOOK()
	switch( pVoipEvent ->id ) {
	case VEID_HOOK_PHONE_OFF_HOOK:
		PRINT_MSG("PHONE_OFF_HOOK(%d)\n", pVoipEvent ->ch_id);
#ifndef AUDIOCODES_VOIP
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#ifdef SW_DTMF_CID
		cid_dtmf_gen_init( pVoipEvent ->ch_id );
#endif
#endif
#endif
		break;
	case VEID_HOOK_PHONE_ON_HOOK:
		PRINT_MSG("PHONE_ON_HOOK(%d)\n", pVoipEvent ->ch_id);
		break;
	case VEID_HOOK_PHONE_FLASH_HOOK:
        PRINT_MSG("PHONE_FLASH_HOOK(%d)\n", pVoipEvent ->ch_id);
		break;

	case VEID_HOOK_FXO_ON_HOOK:
		PRINT_MSG("FXO_ON_HOOK(%d)\n", pVoipEvent ->ch_id);
		break;

	case VEID_HOOK_FXO_OFF_HOOK:
		PRINT_MSG("FXO_OFF_HOOK(%d)\n", pVoipEvent ->ch_id);
		break;

	case VEID_HOOK_FXO_RING_ON:
		PRINT_MSG("FXO_RING_ON(%d)\n", pVoipEvent ->ch_id);
		break;

	case VEID_HOOK_FXO_RING_OFF:
		PRINT_MSG("FXO_RING_OFF(%d)\n", pVoipEvent ->ch_id);
		break;

	case VEID_HOOK_FXO_BUSY_TONE:
		PRINT_MSG("FXO_BUSY_TONE(%d)\n", pVoipEvent ->ch_id);
		break;

	case VEID_HOOK_FXO_RING_TONE_ON:
		PRINT_MSG("FXO_RING_TONE_ON(%d)\n", pVoipEvent ->ch_id);
		break;

	case VEID_HOOK_FXO_RING_TONE_OFF:
		PRINT_MSG("FXO_RING_TONE_OFF(%d)\n", pVoipEvent ->ch_id);
		break;

	case VEID_HOOK_FXO_POLARITY_REVERSAL:
		PRINT_MSG("FXO_POLARITY_REVERSAL(%d)\n", pVoipEvent ->ch_id);
		break;

	case VEID_HOOK_FXO_CURRENT_DROP:
		PRINT_MSG("FXO_CURRENT_DROP(%d)\n", pVoipEvent ->ch_id);
		break;
	
	case VEID_HOOK_OUTBAND_FLASH_EVENT:
		PRINT_MSG("OUTBAND_FLASH_EVENT(%d)\n", pVoipEvent ->ch_id);
		break;
	default:
		break;
	}
	
	return 0;
}

int g_disable_announce_fax;
int g_fast_report_enable=1;
int g_disable_report_fax=0;
int g_enable_fax_dis;

static int extra_process_event_out_faxmdm( voip_event_queue_t *queue, TstVoipEvent *pVoipEvent )
{
	// Use .p1 to store translated ID 
	//
	extern uint32 fax_modem_det_mode[];
	
	pVoipEvent ->p1 = 0;	// default translated ID 
	
	switch( pVoipEvent ->id )
	{
		// Audiocodes DSP 
		case VEID_FAXMDM_AUDIOCODES_FAX:
			pVoipEvent ->p1 = 1;	// 1: fax
			return 0;
		case VEID_FAXMDM_AUDIOCODES_MODEM:
			pVoipEvent ->p1 = 2;	// 2: modem
			return 0;
			
		// Realtek DSP 
		case VEID_FAXMDM_FAST_FAXTONE_LOCAL:
			if (g_fast_report_enable==0)
				break;
		case VEID_FAXMDM_FAX_CED:
			pVoipEvent ->p1 = 1;
			//printk("fax_modem_out: 1\n");
			break;
		case VEID_FAXMDM_FAX_DIS_RX:
			if (g_enable_fax_dis)
				pVoipEvent ->p1 = 5;
			else
				pVoipEvent ->p1 = 2;
			//printk("fax_modem_out: 2\n");
			break;
		case VEID_FAXMDM_FAST_MODEMTONE_LOCAL:
			if (g_fast_report_enable==0)
				break;
		case VEID_FAXMDM_MODEM_LOCAL:
			pVoipEvent ->p1 = 3;
			//printk("fax_modem_out: 3\n");
			break;
		case VEID_FAXMDM_MODEM_REMOTE:
			pVoipEvent ->p1 = 4;
			//printk("fax_modem_out: 4\n");
			break;
		case VEID_FAXMDM_V21FLAG_LOCAL:
			if (!g_enable_fax_dis)
				pVoipEvent ->p1 = 5;
			else
				pVoipEvent ->p1 = 0;
			//printk("fax_modem_out: 2\n");
			break;
		default:
			pVoipEvent ->p1 = 0;
			break;
	}

	if (fax_modem_det_mode[pVoipEvent ->ch_id] == 1)
	{
		if (pVoipEvent ->p1 > 0)
			pVoipEvent ->p1 = 1;
	}
	else if (fax_modem_det_mode[pVoipEvent ->ch_id] == 2)
	{
		if ( (pVoipEvent ->p1 == 1) || (pVoipEvent ->p1 == 2) )
			pVoipEvent ->p1 = 0;
	}

#if 0	// flush!! move to application 
	if (stVoipCfg.cfg == 1)
	{
		voip_event_flush_fax_modem_fifo(stVoipCfg.ch_id, 1);
		pVoipEvent ->p1 = 0;
	}
#endif
	
	if (g_disable_report_fax)
		pVoipEvent ->p1 = 0;
	
	return 0;
}

// ----------------------------------------------------------------------
// Initialze  
// ----------------------------------------------------------------------

static int __init voip_events_init( void )
{	
#define _M_EXTRA_PRIV_NULL	NULL, 0
#define _M_PRIV( array )	array, \
							( sizeof( array ) / ( sizeof( array ) / sizeof( array[ 0 ] ) ) )

	static const struct {
		const char * name;
		extra_process_event_out extra;
		void *extra_priv;
		int   extra_priv_size;
	} queue_basic_initial[] = {
		{ "DTMF(TDM)",	NULL, _M_EXTRA_PRIV_NULL },
		{ "DTMF(IP)", 	NULL, _M_EXTRA_PRIV_NULL },
		{ "HOOK", 		extra_process_event_out_hook, _M_PRIV( extra_hook_priv ) },
		{ "ENERGY", 	NULL, _M_EXTRA_PRIV_NULL },
		{ "DECT", 		NULL, _M_EXTRA_PRIV_NULL },
		{ "FAXMDM", 	extra_process_event_out_faxmdm, _M_EXTRA_PRIV_NULL },
		{ "RFC2833(0)", NULL, _M_EXTRA_PRIV_NULL },
		{ "RFC2833(1)", NULL, _M_EXTRA_PRIV_NULL },
		{ "DSP(0)", 	NULL, _M_EXTRA_PRIV_NULL },
		{ "DSP(1)", 	NULL, _M_EXTRA_PRIV_NULL },
	};

#undef _M_EXTRA_PRIV_NULL
#undef _M_PRIV
	
	CT_ASSERT( VOIP_EVENT_QUEUE_TOTAL_NUM == 
				( sizeof( queue_basic_initial ) / sizeof( queue_basic_initial[ 0 ] ) ) );
	
	extern int Is_DAA_Channel(int chid);
	int i, q;
	
	for( i = 0; i < CON_CH_NUM; i ++ ) {
		
		// fill priv data 
		extra_hook_priv[ i ].prev_id = ( Is_DAA_Channel( i ) ?
						VEID_HOOK_FXO_ON_HOOK : VEID_HOOK_PHONE_ON_HOOK );
		
		// fill queue info 
		for( q = 0; q < VOIP_EVENT_QUEUE_TOTAL_NUM; q ++ ) {
			voip_event_queue[ i ][ q ].name = queue_basic_initial[ q ].name;
			voip_event_queue[ i ][ q ].extra.fn = queue_basic_initial[ q ].extra;
			voip_event_queue[ i ][ q ].extra.priv = 
									( char * )queue_basic_initial[ q ].extra_priv +
									queue_basic_initial[ q ].extra_priv_size * i;
		}
	}
	
	return 0;
}

voip_initcall( voip_events_init );

// ----------------------------------------------------------------------
// Proc 
// ----------------------------------------------------------------------

static int __sprintf_voip_events_detail( char *buf, const voip_event_queue_t *queue )
{
	int n = 0;
	
	uint32 ri = queue ->ri;
	const uint32 wi = queue ->wi;
	const voip_event_entity_t *entity;
	
	n += sprintf( buf + n, " %s\n\tfn=%p priv=%p drop=%u spin=%u\n", queue ->name, 
											queue ->extra.fn, queue ->extra.priv, 
											queue ->drop, queue ->spin );
	
	// headline
	if( ri != wi ) {
		n += sprintf( buf + n, "\t   ID       p0       p1      time\n" );
		//                        -------- -------- -------- -------- 
	}
	
	while( ri != wi ) {
		
		// get entity 
		entity = &queue ->entity[ ri ];
		
		// sprintf entity 
		n += sprintf( buf + n, "\t%08X %08X %08X %08X\n", 
							entity ->id, 
							entity ->p0, entity ->p1,
							entity ->time );
		
		// ri ++
		ri ++;
		
		if( ri == VOIP_EVENT_QUEUE_ENTITY_NUM )
			ri ++;
	}
	
	return n;
}

static int sprintf_voip_events_detail( char *buf, off_t off, int ch )
{
	int n = 0;
	int i;
	
	for( i = 0; i < VOIP_EVENT_QUEUE_TOTAL_NUM; i ++ ) {
		if( i == off ) {
			n += sprintf( buf + n, "Queue %d:", i );
			n += __sprintf_voip_events_detail( buf + n, &voip_event_queue[ ch ][ i ] );
			
			break;
		}
	}
	
	return n;
}

static int voip_events_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	/*
	 * This may be a *large* data, so we implement sequential reading. 
	 * A key point is '*start = 1', and it causes 'off' = 0, 1, 2, .... 
	 * Then, we use this clue to read data from each queue. 
	 */
	 
	int ch;//, ss;
	int n = 0;
	
	if( IS_CH_PROC_DATA( data ) ) {
		ch = CH_FROM_PROC_DATA( data );
		
		if( off == 0 )
			n += sprintf( buf + n, "channel=%d\n", ch );
			
		n += sprintf_voip_events_detail( buf + n, off, ch );
	} else {
		//ss = SS_FROM_PROC_DATA( data );
		//n = sprintf( buf, "session=%d\n", ss );
	}
	
	*start = ( char * )1;	// use magic 'start' to maintain iteration (off += start)
	
	if( n == 0 )
		*eof = 1;
	
	return n;
}

static int __init voip_events_proc_init( void )
{
	create_voip_channel_proc_read_entry( "events", voip_events_read_proc );
	
	return 0;
}

static void __exit voip_events_proc_exit( void )
{
	remove_voip_channel_proc_entry( "events" );
}

voip_initcall_proc( voip_events_proc_init );
voip_exitcall( voip_events_proc_exit );

