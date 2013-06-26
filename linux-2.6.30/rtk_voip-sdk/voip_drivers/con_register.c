#include "voip_proc.h"
#include "voip_init.h"
#include "con_register.h"

typedef struct {
	void *head;
	void *tail;
} link_ptr_t;

static link_ptr_t voip_ioc_link;
static link_ptr_t voip_snd_link;
static link_ptr_t voip_bus_link;
static link_ptr_t voip_dsp_link;

// --------------------------------------------------------
// register  
// --------------------------------------------------------

#define fill_link_next_field( list, num )		\
{												\
	int i;										\
												\
	for( i = 0; i < num - 1; i ++ ) {			\
		list[ i ].link.next = &list[ i + 1 ];	\
	}											\
	list[ i ].link.next = NULL;					\
}

#define fill_link_ptr_head_tail( name, list, num )			\
{															\
	if( voip_ ## name ## _link.head == NULL )				\
		voip_ ## name ## _link.head = ( void * )&list[ 0 ];	\
															\
	if( voip_ ## name ## _link.tail != NULL )				\
		( ( voip_ ## name ##_t * )voip_ ## name ## _link.tail ) ->link.next = &list[ 0 ];	\
																\
	voip_ ## name ## _link.tail = ( void * )&list[ num - 1 ];	\
																\
}

#define IMPLEMENT_REGISTER_VOIP( name )					\
int register_voip_ ##name( voip_ ##name## _t voip_ ##name [], int num )	\
{														\
	if( num <= 0 )										\
		return -1;										\
														\
	fill_link_next_field( voip_ ##name, num );			\
														\
	fill_link_ptr_head_tail( name, voip_ ##name, num );	\
														\
	return 0;											\
}

IMPLEMENT_REGISTER_VOIP( ioc )
IMPLEMENT_REGISTER_VOIP( snd )
IMPLEMENT_REGISTER_VOIP( bus )
IMPLEMENT_REGISTER_VOIP( dsp )

// --------------------------------------------------------
// find function   
// --------------------------------------------------------

#define IMPLEMENT_FIND_FUNCTION( func_name, xname, chk_type, chk_name, chk_name_s, cmp )	\
voip_ ##xname## _t *func_name( chk_type chk_name )	\
{													\
	voip_ ##xname## _t *p_ ##xname = ( voip_ ##xname## _t * )voip_ ##xname## _link.head;	\
															\
	while( p_ ##xname ) {								\
		if( p_ ##xname ->con_ptr )						\
			;	/* attach to something */					\
		else if( cmp == 1 && p_ ##xname ->chk_name_s == chk_name )	\
			return p_ ##xname;							\
		else if( cmp == 2 && ( ( p_ ##xname ->chk_name_s & chk_name ) == chk_name ) )	\
			return p_ ##xname;							\
															\
		p_ ##xname = p_ ##xname ->link.next;		\
	}														\
															\
	return NULL;											\
}

// cmp - equal 
#define IMPLEMENT_FIND_FUNCTION_TYPE1( func_name, xname, chk_type, chk_name, chk_name_s )	\
	IMPLEMENT_FIND_FUNCTION( func_name, xname, chk_type, chk_name, chk_name_s, 1 )
// cmp - and 
#define IMPLEMENT_FIND_FUNCTION_TYPE2( func_name, xname, chk_type, chk_name, chk_name_s )	\
	IMPLEMENT_FIND_FUNCTION( func_name, xname, chk_type, chk_name, chk_name_s, 2 )

IMPLEMENT_FIND_FUNCTION_TYPE1( find_snd_with_snd_type, snd, snd_type_t, snd_type, snd_type );
IMPLEMENT_FIND_FUNCTION_TYPE1( find_bus_with_bus_type, bus, bus_type_t, bus_type, bus_type );
IMPLEMENT_FIND_FUNCTION_TYPE2( find_dsp_with_band_mode, dsp, band_mode_t, band_mode, band_mode_sup );

static voip_bus_t *_find_bus_with_bus_type_band_mode( 
		bus_type_t bus_type, band_mode_t band_mode, int group )
{
	// return continuant bus with same group  
	voip_bus_t *p_bus = ( voip_bus_t * )voip_bus_link.head;
	voip_bus_t *p_bus0 = NULL;
	
	while( p_bus ) {
		
		if( p_bus ->con_ptr )				// in use 
			goto label_do_next;
		
		if( p_bus ->bus_type != bus_type )	// bus is different 
			goto label_do_next;
		
		if( ( p_bus ->band_mode_sup & band_mode ) != band_mode )	// band not match 
			goto label_do_next;
		
		if( group ) {
			if( p_bus0 == NULL ||		// first found 
				p_bus0 ->bus_group != p_bus ->bus_group ) // group are different 
			{
				p_bus0 = p_bus;
				goto label_do_next;
			}
			
			return p_bus0;
		} else {
			return p_bus;
		}
		
label_do_next:		
		p_bus = p_bus ->link.next;
	}
	
	return NULL;	
}

voip_bus_t *find_bus_with_bus_type_band_mode( bus_type_t bus_type, band_mode_t band_mode )
{
	voip_bus_t *p_bus;
	
	p_bus = _find_bus_with_bus_type_band_mode( bus_type, band_mode, 0 );
	
	if( p_bus )
		return p_bus;
	
	// 16k mode special treatment 
	if( band_mode & BAND_MODE_16K ) {
		band_mode &= ~BAND_MODE_16K;
		return _find_bus_with_bus_type_band_mode( bus_type, band_mode, 1 );
	}
	
	return NULL;
}

voip_ioc_t *find_first_ioc( void )
{
	return ( voip_ioc_t * )voip_ioc_link.head;
}

// --------------------------------------------------------
// timeslot assistant functions   
// --------------------------------------------------------

int get_snd_free_timeslot( void )
{
	voip_snd_t *p_snd = ( voip_snd_t * )voip_snd_link.head;
	int TS = 0;
	
	while( p_snd ) {
		
		if( p_snd ->TS1 >= TS )
			TS = p_snd ->TS1 + 2;
		
		if( ( p_snd ->band_mode_sup & BAND_MODE_16K ) &&
			( p_snd ->TS2 < 16 ) &&		// check half 16 TS only 
			( p_snd ->TS2 >= TS ) )
		{
			TS = p_snd ->TS2 + 2;
		}
		
		p_snd = p_snd ->link.next;	
	}

	TS = ( TS + 1 ) & ( ~0x01 );	// alwasy return even TS 
	
	if( TS >= 32 )	// maximum timeslot is 32 
		TS = -1;
	
	return TS;
}

// --------------------------------------------------------
// proc 
// --------------------------------------------------------
enum {
	REG_PROC_COOKIE_IOC,
	REG_PROC_COOKIE_SND,
	REG_PROC_COOKIE_BUS,
	REG_PROC_COOKIE_DSP,
};

#define fill_proc_buff( aname, chname )												\
	n += sprintf( buf + n, #chname " name\n--- -------\n" );									\
																					\
	p_ ## aname = ( const voip_ ##aname## _t * )voip_ ##aname## _link.head;	\
	while( p_ ##aname ) {														\
		n += sprintf( buf + n, "%3d %s\n", p_##aname ->chname, 				\
				( p_ ##aname ->name ? p_ ##aname ->name : "(none)" ) );	\
		p_ ##aname = p_ ##aname ->link.next;								\
	}

static int voip_con_register_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
#define S_PTR( ptr, ptr2 )		( ptr ? ptr ->ptr2, NULL )
#define S_CALL( ptr, ptr2, fn )	( ptr && ptr ->ptr2 && ptr ->ptr2 ->fn ? ptr ->ptr2 ->fn( ptr ) : 0 )

	int n = 0;
	
	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}

#if 1
	switch( ( unsigned int )data ) {
	case REG_PROC_COOKIE_IOC:
		{
			voip_ioc_t *p_ioc;
			voip_snd_t *p_snd_assigned;
			//fill_proc_buff( ioc, ioch );
			n += sprintf( buf + n, "ioch T M S name    ID       pre-assign \n---- - - - ------- -------- ---------- \n" );
			
			p_ioc = ( voip_ioc_t * )voip_ioc_link.head;
			
			while( p_ioc ) {
				
				p_snd_assigned = p_ioc ->pre_assigned_snd_ptr;
				
				n += sprintf( buf + n, "%4X %X %X %X %-7s %08X %2d %-7s \n", 
					p_ioc ->ioch,
					p_ioc ->ioc_type,
					p_ioc ->mode_var, 
					p_ioc ->state_var,
					( p_ioc ->name ? p_ioc ->name : "(none)" ),
					S_CALL( p_ioc, ioc_ops, get_id ),
					( p_snd_assigned ? p_snd_assigned ->sch : -1 ),
					( p_snd_assigned ? p_snd_assigned ->name : "(null)" )
					);
							
				p_ioc = p_ioc ->link.next;
			}
		}
		break;
		
	case REG_PROC_COOKIE_SND:
		{
			const voip_snd_t *p_snd;
			//fill_proc_buff( snd, sch );
			n += sprintf( buf + n, "sch name    TS1/2 T cpu flgs \n--- ------- ----- - --- --------\n" );
			
			p_snd = ( const voip_snd_t * )voip_snd_link.head;
			
			while( p_snd ) {
				n += sprintf( buf + n, "%3d %-7s %2d/%2d %d %-3d %08X\n", 
					p_snd ->sch,
					( p_snd ->name ? p_snd ->name : "(none)" ),
					p_snd ->TS1,
					( p_snd ->band_mode_sup & BAND_MODE_16K ? p_snd ->TS2 : -1 ), 
					p_snd ->snd_type,
					( p_snd ->ipc.f_dsp_cpuid ? p_snd ->ipc.n_dsp_cpuid : -1 ),
					p_snd ->snd_flags.all
					);
							
				p_snd = p_snd ->link.next;
			}
		}
		break;
	case REG_PROC_COOKIE_BUS:
		{
			const voip_bus_t *p_bus;
			//fill_proc_buff( bus, bch );
			n += sprintf( buf + n, "bch name    E TS1/2 \n--- ------- - ----- \n" );
			
			p_bus = ( const voip_bus_t * )voip_bus_link.head;
			
			while( p_bus ) {
				n += sprintf( buf + n, "%3d %-7s %d %2d/%2d\n", 
					p_bus ->bch,
					( p_bus ->name ? p_bus ->name : "(none)" ),
					p_bus ->enabled,
					p_bus ->TS1_var,
					( p_bus ->band_mode_sup & BAND_MODE_16K ? p_bus ->TS2_var : -1 )
					);
							
				p_bus = p_bus ->link.next;
			}
		}
		break;
	case REG_PROC_COOKIE_DSP:
		{
			const voip_dsp_t *p_dsp;
			fill_proc_buff( dsp, dch );
		}
		break;
	}
#else	
	n += sprintf( buf + n, "snd\n-----\n" );
	
	p_snd = ( const voip_snd_t * )voip_snd_link.head;
	while( p_snd ) {
		n += sprintf( buf + n, "%3d %s\n", p_snd ->sch, p_snd ->name );
		p_snd = p_snd ->link.next;
	}
#endif
	
	*eof = 1;
	return n;
#undef S_PTR
#undef S_CALL
}

static int __init voip_con_register_proc_init( void )
{
	create_proc_read_entry( PROC_VOIP_DIR "/" PROC_VOIP_CH_MAPS_DIR "/ioc", 0, NULL, 
			voip_con_register_read_proc, ( void * )REG_PROC_COOKIE_IOC );
	create_proc_read_entry( PROC_VOIP_DIR "/" PROC_VOIP_CH_MAPS_DIR "/snd", 0, NULL, 
			voip_con_register_read_proc, ( void * )REG_PROC_COOKIE_SND );
	create_proc_read_entry( PROC_VOIP_DIR "/" PROC_VOIP_CH_MAPS_DIR "/bus", 0, NULL, 
			voip_con_register_read_proc, ( void * )REG_PROC_COOKIE_BUS );
	create_proc_read_entry( PROC_VOIP_DIR "/" PROC_VOIP_CH_MAPS_DIR "/dsp", 0, NULL, 
			voip_con_register_read_proc, ( void * )REG_PROC_COOKIE_DSP );
	
	return 0;
}

static void __exit voip_con_register_proc_exit( void )
{
	remove_voip_proc_entry( PROC_VOIP_DIR "/" PROC_VOIP_CH_MAPS_DIR "/ioc", NULL );
	remove_voip_proc_entry( PROC_VOIP_DIR "/" PROC_VOIP_CH_MAPS_DIR "/snd", NULL );
	remove_voip_proc_entry( PROC_VOIP_DIR "/" PROC_VOIP_CH_MAPS_DIR "/bus", NULL );
	remove_voip_proc_entry( PROC_VOIP_DIR "/" PROC_VOIP_CH_MAPS_DIR "/dsp", NULL );
}

voip_initcall_proc( voip_con_register_proc_init );
voip_exitcall( voip_con_register_proc_exit );

