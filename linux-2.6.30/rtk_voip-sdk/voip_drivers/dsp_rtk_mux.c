#include "voip_types.h"
#include "con_register.h"
#include "dsp_rtk_mux.h"
#include "dsp_rtk_define.h"

extern uint32 dch_from_cch( uint32 cch );

uint32 cch_from_rtk_dch( uint32 dch )
{
	DECLARE_CON_FROM_RTK_DCH( dch )
	
	return p_con ->cch;
}

void bus_fifo_clean_tx_dch( uint32 dch )
{
	DECLARE_CON_FROM_RTK_DCH( dch );
	
	p_con ->con_ops ->bus_fifo_clean_tx( p_con );
}

