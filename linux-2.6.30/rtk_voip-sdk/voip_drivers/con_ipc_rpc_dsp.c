#include "voip_types.h"
#include "voip_ipc.h"
#include "con_register.h"

void ipc_rpc_request( voip_con_t *this, uint16 category, void *data, uint16 len )
{
	/*
	 * protocol = IPC_PROT_RPC
	 * category = IPC_RPC_SLIC / IPC_RPC_DAA
	 * data is rpc_content_t
	 */
	
	extern int ipcSentRpcPacket( unsigned short category, void* rpc_data, unsigned short rpc_len );
	
	ipcSentRpcPacket( category, data, len );
}

void ipc_rpc_ack_parser( ipc_ctrl_pkt_t *ipc_ctrl )
{
	// TODO: do something to check mirror_ack 
	
}

