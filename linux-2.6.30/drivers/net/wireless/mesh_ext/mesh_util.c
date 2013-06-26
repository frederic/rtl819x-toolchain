/*
 *      Handling routines for Mesh in 802.11 Utils
 *
 *      PS: All extern function in ../8190n_headers.h
 */
#define _MESH_UTILS_C_

#ifdef CONFIG_RTL8192CD
#include "../rtl8192cd/8192cd.h"
#include "../rtl8192cd/8192cd_headers.h"
#else
#include "../rtl8190/8190n.h"
#include "../rtl8190/8190n_headers.h"
#endif
#include "./mesh_util.h"
#include <linux/ctype.h>

void hex_dump(void *data, int size)
{
    /* dumps size bytes of *data to stdout. Looks like:
     * [0000] 75 6E 6B 6E 6F 77 6E 20
     *                  30 FF 00 00 00 00 39 00 unknown 0.....9.
     * (in a single line of course)
     */

    unsigned char *p = data;
    unsigned char c;
    int n;
    char bytestr[4] = {0};
    char addrstr[10] = {0};
    char hexstr[ 16*3 + 5] = {0};
    char charstr[16*1 + 5] = {0};
    for(n=1;n<=size;n++) {
        if (n%16 == 1) {
            /* store address for this line */
            snprintf(addrstr, sizeof(addrstr), "%.4x",
               ((unsigned int)p-(unsigned int)data) );
        }

        c = *p;
        if (isalnum(c) == 0) {
            c = '.';
        }

        /* store hex str (for left side) */
        snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
        strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

        /* store char str (for right side) */
        snprintf(bytestr, sizeof(bytestr), "%c", c);
        strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);

        if(n%16 == 0) {
            /* line completed */
            panic_printk("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
            hexstr[0] = 0;
            charstr[0] = 0;
        } else if(n%8 == 0) {
            /* half line: add whitespaces */
            strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
            strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
        }
        p++; /* next byte */
    }

    if (strlen(hexstr) > 0) {
        /* print rest of buffer if not empty */
        panic_printk("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
    }
}

#ifdef CONFIG_RTK_MESH

int PathSelection_del_tbl_entry(DRV_PRIV *priv, char *delMAC)
{
	struct path_sel_entry *pEntry = 0;	
	unsigned long flags, i=0;

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID) // 1.Pathsel_table in root interface NOW!! 2.Spare for Mesh work with Multiple AP (Please see Mantis 0000107 for detail)
	if(!IS_ROOT_INTERFACE(priv))
		return FAIL;
#endif

	for(;i<(1 << priv->pathsel_table->table_size_power);++i)
	{
		pEntry = (struct path_sel_entry *)(priv->pathsel_table->entry_array[i].data);
		if((memcmp(delMAC, pEntry->nexthopMAC,MACADDRLEN)==0) ) {

			SAVE_INT_AND_CLI(flags);
			remove_path_entry(priv, pEntry->destMAC);
			RESTORE_INT(flags);
		}			
	}		
	return SUCCESS;
}

//#ifdef	_11s_TEST_MODE_

void mac12_to_6(unsigned char *mac1, unsigned char *mac2)
{
	short m=0;
	for( ; m<MACADDRLEN; m++)
		mac2[m] = (((mac1[1|(m<<1)])>='a')?(mac1[1|(m<<1)]-('a'-0xa)):(mac1[1|(m<<1)]-'0'))|((((mac1[m<<1])>='a')?(mac1[m<<1]-('a'-0xa)):(mac1[m<<1]-'0'))<<4);
}
//#endif

/*
 *	@brief	Count MESH Association number and display
 *
 *	@param	priv:priv
 *	@param	pstat: pstat
 *	@param	act: action
 *			INCREASE = New peer MP (action: minus peer cap)
 *			DECREASE = Delete exist connect MP (acton: Plus peer cap)
 *
 *	@retval	void
 */
void mesh_cnt_ASSOC_PeerLink_CAP(DRV_PRIV *priv, struct stat_info *pstat, int act)
{
	unsigned long	flags;
	UINT8	modify = TRUE;

	SAVE_INT_AND_CLI(flags);

#if 0
	LOG_MESH_MSG("N:%d, M:%d, a:%s\n", MESH_PEER_LINK_CAP_NUM(priv), GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor, act ? "+" : "-");
#endif	

	if (DECREASE == act) {
		if (MESH_PEER_LINK_CAP_NUM(priv) < GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor)
			MESH_PEER_LINK_CAP_NUM(priv)++;
		else {
			modify = FALSE;
			MESH_DEBUG_MSG("MESH PeerLink CAP Number Error (%d)!\n", GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor);
		}
	} else {
		if (MESH_PEER_LINK_CAP_NUM(priv) > 0)
			MESH_PEER_LINK_CAP_NUM(priv)--;
		else {
			modify = FALSE;
			MESH_DEBUG_MSG("MESH PeerLink CAP Number Error (0)!\n");
		}
	}

	RESTORE_INT(flags);

#if 0
	LOG_MESH_MSG("N:%d, M:%d, %s\n",
	MESH_PEER_LINK_CAP_NUM(priv), GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor, modify ? "TRUE" : "FALSE");
#endif

	if (TRUE == modify) {
		update_beacon(priv);		// Because MESH_PEER_LINK_CAP_NUM modify (beacon include WLAN Mesh Capacity)
		
		if (DECREASE == act)
		{
			PathSelection_del_tbl_entry(priv, pstat->hwaddr);	// call del pathselection table entry
		}
	}

#if (MESH_DBG_LV & MESH_DBG_SIMPLE)
	printk("Mesh assoc_num: %s(Max:%d, Remain:%d) %02X:%02X:%02X:%02X:%02X:%02X\n",
		act?"++":"--",
		GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor,
		MESH_PEER_LINK_CAP_NUM(priv),
		pstat->hwaddr[0],
		pstat->hwaddr[1],
		pstat->hwaddr[2],
		pstat->hwaddr[3],
		pstat->hwaddr[4],
		pstat->hwaddr[5]);
#endif // (MESH_DBG_LV & MESH_DBG_SIMPLE)

}

/*
 *	@brief	Set MESH Association Max number (call by web interface?)
 *			Note: If setting value(meshCapSetValue) less system current connection number,  MESH_PEER_LINK_CAP_NUM become negative, Denied any new connection.
 *			Delete exist connect ONLY!!
 *				SO 
 *					MESH_PEER_LINK_CAP_NUM = 0							: setting = connection, Denied any new connection
 *	 				MESH_PEER_LINK_CAP_NUM > 0 (< NUM_AVAILABLE_PEER)	: setting > connection ,Allow new connect
 * 					MESH_PEER_LINK_CAP_NUM = NUM_AVAILABLE_PEER		: connection = 0 ,Allow new connect
 *
 *	@param	priv:priv
 *	@param	meshCapSetValue: set connect number
 *
 *	@retval	void
 */
void mesh_set_PeerLink_CAP(DRV_PRIV *priv, UINT16 meshCapSetValue)
{
	struct list_head	*plist;
	unsigned long flags;
	UINT16	count = 0;

	SAVE_INT_AND_CLI(flags);
	
	meshCapSetValue &= MESH_PEER_LINK_CAP_CAPACITY_MASK;

	if (NUM_AVAILABLE_PEER < meshCapSetValue)
		meshCapSetValue = NUM_AVAILABLE_PEER;	// Exceed hard code MAX number, Lock on hardcode MAX number

	GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor = meshCapSetValue;	// Writeback runtime Max value (Avoid web interface fault)

	//  Recalculate current connection number by association list (Avoid problem when  CAP value error)
	plist = &(priv->mesh_mp_hdr);
	while (plist->next != &(priv->mesh_mp_hdr))	// 1.Check index  2.Check is it least element? (Because  next pointer to mesh_mp_hdr itself) 
	{
		count++;
		plist = plist->next;		// pointer to next element's list_head struct
	}

	MESH_PEER_LINK_CAP_NUM(priv) = (INT16)(meshCapSetValue - count);	// Setting - current connection number  -> CAP (NOTE: Allow POS/NEG value)
	RESTORE_INT(flags);
	init_beacon(priv);
	return;
}
#endif // _DOT11_MESH_MODE

