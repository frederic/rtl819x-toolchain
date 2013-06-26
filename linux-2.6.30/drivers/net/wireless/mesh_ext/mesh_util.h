
#ifndef _MESH_UTIL_H_
#define _MESH_UTIL_H_

#include <linux/list.h>
#include "./mesh.h"
#ifdef CONFIG_RTL8192CD
#include "../rtl8192cd/8192cd_util.h"
#else
#include "../rtl8190/8190n_util.h"
#endif

#define is_mesh_6addr_format_without_qos(pframe)	((*((unsigned char*)(pframe)+30) & 0x01))	///< AE field(mesh header) bit0 = 1
#define isMeshPoint(p)		(p&&(p->mesh_neighbor_TBL.State==MP_SUPERORDINATE_LINK_UP||p->mesh_neighbor_TBL.State==MP_SUBORDINATE_LINK_UP))

// Separate 3 define, Because decision MeshPoint/STA condition may different or NEW device (isXXX) in the future.
#define isPossibleNeighbor(p)	(MP_UNUSED != p->mesh_neighbor_TBL.State)
#define isSTA(p)  			(MP_UNUSED == p->mesh_neighbor_TBL.State)
#define isSTA2(p)  			(MP_UNUSED == p.mesh_neighbor_TBL.State)


#ifdef GREEN_HILL
#define MESH_LOCK(x,y)		{ y = save_and_cli(); }
#define MESH_UNLOCK(x,y)	restore_flags(y)
#else
#define MESH_LOCK(x,y)		spin_lock_irqsave(&priv->pshare->x, y)
#define MESH_UNLOCK(x,y)	spin_unlock_irqrestore(&priv->pshare->x, y)
#endif // not GREEN_HILL


/*
 *	@brief	MESH  PeerLink_CAP number routine
 */
#define MESH_PEER_LINK_CAP_NUM(priv)	(priv->mesh_PeerCAP_cap)

// Galileo 2008.06.18
#ifdef _11s_TEST_MODE_
struct Galileo {
	struct rtl8190_priv *priv;
	struct tx_insn txcfg;
	struct timer_list expire_timer;
	unsigned short tx_count;
};
 
struct Galileo_node {
	struct list_head list;
	struct Galileo data;
};
 
struct Galileo_poll {
	struct Galileo_node node[AODV_RREQ_TABLE_SIZE];
	int count;
};
 
#endif


#ifdef PU_STANDARD
typedef struct {
	UINT8 flag;
	UINT8 PUseq;
	UINT8 proxyaddr[MACADDRLEN];
	UINT16 addrNum;
	struct list_head addrs;
} ProxyUpdate;
#endif

#ifdef	_11s_TEST_MODE_
extern void mac12_to_6(unsigned char*, unsigned char*);
#endif

#ifdef CONFIG_RTL8192CD
extern int PathSelection_del_tbl_entry(struct rtl8192cd_priv *priv, char *delMAC);
#else
extern int PathSelection_del_tbl_entry(struct rtl8190_priv *priv, char *delMAC);
#endif

/*
 *	@brief	Set pseudo random number
 *
 *	@param	target		: Variable for set
 *
 *	@retval	target		: Set finish variable.
 *	
 *	PS1. Avoid generator same random number by use get_random_bytes same sequence and time.
 *	PS2. Generator more randomly random number (Avoid get same random number per boot).
 */
#define SET_PSEUDO_RANDOM_NUMBER(target)	{ \
	get_random_bytes(&(target), sizeof(target)); \
	target += (GET_MY_HWADDR[4] + GET_MY_HWADDR[5] + jiffies - priv->net_stats.rx_bytes \
	+ priv->net_stats.tx_bytes + priv->net_stats.rx_errors - priv->ext_stats.beacon_ok); \
}

#endif	// _MESH_UTIL_H_
