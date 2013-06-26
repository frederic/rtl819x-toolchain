#ifndef __IPC_ARCH_RX_H__
#define __IPC_ARCH_RX_H__

extern unsigned short get_response_content(unsigned short* pDsp_Id, unsigned char** pCont, unsigned short* pCategory);

extern void ipc_pkt_rx_entry( ipc_ctrl_pkt_t * ipc_pk, uint32 ipc_pkt_len );

#endif /* __IPC_ARCH_RX_H__ */

