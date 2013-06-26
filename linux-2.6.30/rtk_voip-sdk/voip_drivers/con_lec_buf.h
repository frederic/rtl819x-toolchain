#ifndef __CON_LEC_BUF_H__
#define __CON_LEC_BUF_H__

#include "voip_types.h"
#include "con_register.h"

// lec buf - common 
// give initial sync point after binding 
extern void lec_buf_init_sync_point( voip_con_t *this );
extern void lec_buf_flush( voip_con_t *this );

// lec buf - handle isr 
extern void isr_lec_buf_inc_windex( voip_con_t *this );
extern void isr_lec_buf_tx_process( voip_con_t *this, uint16 *pcm_tx, uint32 step );
extern void isr_lec_buf_tx_process_post( voip_con_t *this );
extern void isr_lec_buf_inc_rindex( voip_con_t *this );
extern const uint16 *isr_lec_buf_get_ref_addr( voip_con_t *this, uint32 step );
extern void isr_lec_buf_sync_p( voip_con_t *this, const uint16 *pcm_rx, uint32 step );

#endif /* __CON_LEC_BUF_H__ */

