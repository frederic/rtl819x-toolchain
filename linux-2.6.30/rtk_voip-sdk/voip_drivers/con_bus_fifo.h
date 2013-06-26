#ifndef __CON_BUS_FIFO_H__
#define __CON_BUS_FIFO_H__

#include "voip_types.h"

// --------------------------------------------------------
// con_ops function   
// --------------------------------------------------------

// handle isr 
extern uint16 * isr_bus_read_tx( voip_con_t *this, voip_bus_t *p_bus, uint16 *bus_tx );
extern uint16 * isr_bus_write_rx_TH( voip_con_t *this, voip_bus_t *p_bus, const uint16 *bus_rx );
extern uint16 * isr_bus_write_rx_BH( voip_con_t *this );
extern uint16 * isr_bus_write_rx_lo( voip_con_t *this );
	
// dsp is caller 
extern uint16 * dsp_write_bus_tx_get_addr( voip_con_t *this );
extern void     dsp_write_bus_tx_done( voip_con_t *this );
extern uint16 * dsp_read_bus_rx_get_addr( voip_con_t *this );
extern uint16 * dsp_read_bus_rx_peek_addr( voip_con_t *this, int offset );
extern void     dsp_read_bus_rx_done( voip_con_t *this );
extern int      dsp_read_bus_rx_get_fifo_size( voip_con_t *this );

// bus fifo
extern void bus_fifo_set_tx_mute( voip_con_t *this, int enable );
extern void bus_fifo_set_rx_mute( voip_con_t *this, int enable );
extern void bus_fifo_clean_tx( voip_con_t *this );
extern void bus_fifo_clean_rx( voip_con_t *this );
extern void bus_fifo_flush( voip_con_t *this, int tx, int rx );


// --------------------------------------------------------
// fifo normal function   
// --------------------------------------------------------


#endif /* __CON_BUS_FIFO_H__ */

