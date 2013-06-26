#include "con_register.h"
#include "con_bind.h"

static void bind_core( voip_con_t *p_con, voip_snd_t *p_snd, 
						voip_bus_t *p_bus, voip_dsp_t *p_dsp,
						band_mode_t band_mode, con_share_t share )
{
	voip_bus_t * p_bus2 = NULL;
	
	//printk( "bind_core:p_con=%p, p_snd=%p, p_bus=%p, p_dsp=%p, band_mode=%d, share=%d\n", 
	//				p_con, p_snd, p_bus, p_dsp, band_mode, share );
	
	// decide p_bus2 
	if( ( band_mode & BAND_MODE_16K ) && 
		( p_bus ->band_mode_sup & BAND_MODE_16K ) == 0 ) 
	{
		p_bus2 = p_bus + 1;
	}	
	
	// con
	p_con ->share_bind = share;
	p_con ->band_mode_bind = band_mode;
	p_con ->band_factor_var = 1;	// give init value, enable() will modify this again 
	p_con ->band_sample_var = 80;	// give init value, enable() will modify this again 
	
	p_con ->snd_ptr = p_snd;
	p_con ->bus_ptr = p_bus;
	p_con ->bus2_ptr = p_bus2;
	p_con ->dsp_ptr = p_dsp;
	
	// snd
	p_snd ->bus_type_bind = 		// if no bind bus, use snd itself 
				( p_bus ? p_bus ->bus_type : p_snd ->bus_type_sup );
	p_snd ->band_mode_bind = band_mode;

	p_snd ->con_ptr = p_con;
	
	// bus 
	if( p_bus == NULL )
		goto label_bus_done;
		
	//p_bus ->TS1_var = p_snd ->TS1;
	//p_bus ->TS2_var = p_snd ->TS2;
	if( p_bus2 == NULL ) {
		p_bus ->bus_ops ->set_timeslot( p_bus, p_snd ->TS1, p_snd ->TS2 );
		p_bus ->role_bind = BUS_ROLE_SINGLE;
		p_bus ->role_var = BUS_ROLE_SINGLE;	// give init value, enable() will modify this again 
		p_bus ->band_mode_bind = band_mode;
	} else {
		p_bus ->bus_ops ->set_timeslot( p_bus, p_snd ->TS1, 0 );
		p_bus ->role_bind = BUS_ROLE_MAIN;
		p_bus ->role_var = BUS_ROLE_MAIN;	// give init value, enable() will modify this again 
		p_bus ->band_mode_bind = BAND_MODE_8K;		
	}
	
	p_bus ->con_ptr = p_con;
	p_bus ->bus_partner_ptr = p_bus2;

label_bus_done:
	
	// bus2 
	if( p_bus2 ) {
		p_bus2 ->bus_ops ->set_timeslot( p_bus2, p_snd ->TS2, 0 );
		p_bus2 ->role_bind = BUS_ROLE_MATE;
		p_bus2 ->role_var = BUS_ROLE_MATE;	// give init value, enable() will modify this again 
		p_bus2 ->band_mode_bind = BAND_MODE_8K;	
		
		p_bus2 ->con_ptr = p_con;
		p_bus2 ->bus_partner_ptr = p_bus;			
	}
	
	// dsp
	if( p_dsp == NULL )
		goto label_dsp_done;
		
	p_dsp ->band_mode_bind = band_mode;
	
	p_dsp ->con_ptr = p_con;

label_dsp_done:
	;
}

static int policy1_dect_fxs_daa( voip_con_t voip_con[], int num )
{
#define BAND_MODE_P1	BAND_MODE_8K

	int i = 0;
	voip_snd_t *p_snd;
	voip_bus_t *p_bus;
	voip_dsp_t *p_dsp;
	snd_type_t target_snd_type = SND_TYPE_DECT;
	
	while( i < num ) {
		
		// find snd 
		p_snd = find_snd_with_snd_type( target_snd_type );
		
		if( p_snd == NULL ) {
			if( target_snd_type == SND_TYPE_DECT ) {
				target_snd_type = SND_TYPE_FXS;
				continue;
			} else if( target_snd_type == SND_TYPE_FXS ) {
				target_snd_type = SND_TYPE_DAA;
				continue;
			} else
				break;	// no more FXS/FXO
		}
		
		// find bus 
		p_bus = find_bus_with_bus_type( p_snd ->bus_type_sup );
		
		if( p_bus == NULL )
			break;	// no bus to support this snd 
		
		// find dsp
		p_dsp = find_dsp_with_band_mode( BAND_MODE_P1 );
		
		if( p_dsp == NULL )
			break;	// no dsp to support BAND_MODE_P1
		
		// ok! bind them 
		bind_core( &voip_con[ i ], p_snd, p_bus, p_dsp, 
					BAND_MODE_P1, 0 );
		
		// 	
		i ++;
	}
	
	return i;
#undef BAND_MODE_P1
}

static int policy2_fxs_daa_share( voip_con_t voip_con[], int num )
{
#define BAND_MODE_P2	BAND_MODE_8K

	int i = 0;
	voip_snd_t *p_snd;
	voip_bus_t *p_bus;
	voip_dsp_t *p_dsp;
	snd_type_t target_snd_type = SND_TYPE_FXS;
	int idx_fxs = 0, idx_fxs_num = 0;
	
	while( i < num ) {
		
		// find snd 
		p_snd = find_snd_with_snd_type( target_snd_type );
		
		if( p_snd == NULL ) {
			if( target_snd_type == SND_TYPE_FXS ) {
				target_snd_type = SND_TYPE_DAA;
				idx_fxs_num = i;
				continue;
			} else
				break;	// no more FXS/FXO
		}
		
		// DAA can equal or less FXS
		if( target_snd_type == SND_TYPE_DAA && idx_fxs >= idx_fxs_num )
			break;
		
		// find bus 
		if( target_snd_type == SND_TYPE_DAA ) {
			p_bus = voip_con[ idx_fxs ++ ].bus_ptr;
		} else {
			p_bus = find_bus_with_bus_type( p_snd ->bus_type_sup );
			
			if( p_bus == NULL )
				break;	// no bus to support this snd 
		}
		
		// find dsp
		p_dsp = find_dsp_with_band_mode( BAND_MODE_P2 );
		
		if( p_dsp == NULL )
			break;	// no dsp to support BAND_MODE_P1
		
		// ok! bind them 
		bind_core( &voip_con[ i ], p_snd, p_bus, p_dsp, 
					BAND_MODE_P2, CON_SHARE_BUS );
		
		// 	
		i ++;
	}
	
	return i;
#undef BAND_MODE_P2
}

static int policy_single_type_only( voip_con_t voip_con[], int num, 
							const snd_type_t target_snd_type )
{
	int i = 0;
	voip_snd_t *p_snd;
	voip_bus_t *p_bus;
	voip_dsp_t *p_dsp;
	band_mode_t band_mode = BAND_MODE_8K;
	
	//printk( "voip_con=%p, num=%d\n", voip_con, num );
	
	while( i < num ) {
		
		// find snd 
		p_snd = find_snd_with_snd_type( target_snd_type );
		
		if( p_snd == NULL ) {
			break;	// no more FXS
		}
				
		// find bus 
		p_bus = find_bus_with_bus_type( p_snd ->bus_type_sup );
		
		if( p_bus == NULL )
			break;	// no bus to support this snd 
		
		// check band mode 
		band_mode = p_snd ->band_mode_sup & p_bus ->band_mode_sup;
		band_mode &= ( BAND_MODE_8K | BAND_MODE_16K );
		
		if( !band_mode )
			break;
		
		if( band_mode & BAND_MODE_8K )
			band_mode = BAND_MODE_8K;
		else if( band_mode & BAND_MODE_16K )
			band_mode = BAND_MODE_16K;
		
		// find dsp
		p_dsp = find_dsp_with_band_mode( band_mode );
		
		if( p_dsp == NULL )
			break;	// no dsp to support BAND_MODE_P1
		
		// ok! bind them 
		bind_core( &voip_con[ i ], p_snd, p_bus, p_dsp, 
					band_mode, 0 );
		
		// 	
		i ++;
	}
	
	return i;
}

static int policy3_fxs_only( voip_con_t voip_con[], int num )
{
	return policy_single_type_only( voip_con, num, SND_TYPE_FXS );
}

static int policy4_dect_fxs_daa_fully( voip_con_t voip_con[], int num )
{
	int i = 0;
	voip_snd_t *p_snd;
	voip_bus_t *p_bus;
	voip_dsp_t *p_dsp;
	snd_type_t target_snd_type = SND_TYPE_DECT;
	
	while( i < num ) {
		
		// find snd 
		p_snd = find_snd_with_snd_type( target_snd_type );
		
		if( p_snd == NULL ) {
			if( target_snd_type == SND_TYPE_DECT ) {
				target_snd_type = SND_TYPE_FXS;
				continue;
			} else if( target_snd_type == SND_TYPE_FXS ) {
				target_snd_type = SND_TYPE_DAA;
				continue;
			} else
				break;	// no more FXS/FXO/DECT
		}
		
		// find bus 
		p_bus = find_bus_with_bus_type_band_mode
					( p_snd ->bus_type_sup, p_snd ->band_mode_sup );
		
		if( p_bus == NULL )
			break;	// no bus to support this snd 
		
		// find dsp
		p_dsp = find_dsp_with_band_mode( p_snd ->band_mode_sup );
		
		if( p_dsp == NULL )
			break;	// no dsp to support BAND_MODE_P1
		
		// ok! bind them 
		bind_core( &voip_con[ i ], p_snd, p_bus, p_dsp, 
					p_snd ->band_mode_sup, 0 );
		
		// 	
		i ++;
	}
	
	return i;
}

static int policy5_ac_only( voip_con_t voip_con[], int num )
{
	return policy_single_type_only( voip_con, num, SND_TYPE_AC );
}

static int policy6_fxs_daa_sndonly( voip_con_t voip_con[], int num )
{
	int i = 0;
	voip_snd_t *p_snd;
	snd_type_t target_snd_type = SND_TYPE_FXS;
	
	while( i < num ) {
		
		// find snd 
		p_snd = find_snd_with_snd_type( target_snd_type );
		
		if( p_snd == NULL ) {
			if( target_snd_type == SND_TYPE_FXS )
				target_snd_type = SND_TYPE_DAA;
			else
				break;	// no more FXS/DAA
				
			continue;
		}
		
		// ok! bind them (snd only!!)
		bind_core( &voip_con[ i ], p_snd, NULL, NULL, 
					BAND_MODE_8K, 0 );
		
		//
		i ++;
	}
	
	return i;
}

int auto_bind_con_with_policy( voip_con_t voip_con[], int num, bind_policy_t policy )
{
	if( policy == BIND_POLICY_DECT_FXS_DAA )
		return policy1_dect_fxs_daa( voip_con, num );
	else if( policy == BIND_POLICY_FXS_DAA_SHARE )
		return policy2_fxs_daa_share( voip_con, num );
	else if( policy == BIND_POLICY_FXS_ONLY )
		return policy3_fxs_only( voip_con, num );
	else if( policy == BIND_POLICY_DECT_FXS_DAA_FULLY )
		return policy4_dect_fxs_daa_fully( voip_con, num );
	else if( policy == BIND_POLICY_AC_ONLY )
		return policy5_ac_only( voip_con, num );
	else if( policy == BIND_POLICY_FXS_DAA_SNDONLY )
		return policy6_fxs_daa_sndonly( voip_con, num );
	
	return 0;
}


