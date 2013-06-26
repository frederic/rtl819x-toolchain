/*
 *      MIB table declaration
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: mibtbl.c,v 1.30 2009/09/03 06:58:11 keith_huang Exp $
 *
 */

/* Include files */
#include "apmib.h"
#include "mibtbl.h"

/* Global variable definitions */

/*
 * When using flash (set/get/all) command to access the MIB of below table,
 * it needs append a keyword "DEF_" in ahead of mib name when access
 * default setting MIB.
 */

mib_table_entry_T mib_dhcpRsvdIp_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_DHCPRSVDIP_IMPORT
#include "mibdef.h"
#undef MIB_DHCPRSVDIP_IMPORT

#undef MIBDEF
{0}
};

#ifdef WLAN_PROFILE
mib_table_entry_T mib_wlan_profile_tbl1[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_WLAN_PROFILE_IMPORT
#include "mibdef.h"
#undef MIB_WLAN_PROFILE_IMPORT

#undef MIBDEF
{0}
};

mib_table_entry_T mib_wlan_profile_tbl2[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_WLAN_PROFILE_IMPORT
#include "mibdef.h"
#undef MIB_WLAN_PROFILE_IMPORT

#undef MIBDEF
{0}
};

#endif


mib_table_entry_T mib_schedule_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_SCHEDULE_IMPORT
#include "mibdef.h"
#undef MIB_SCHEDULE_IMPORT

#undef MIBDEF
{0}
};

#if defined(CONFIG_RTL_8198_AP_ROOT)
mib_table_entry_T mib_vlanconfig_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_VLAN_CONFIG_IMPORT
#include "mibdef.h"
#undef MIB_VLAN_CONFIG_IMPORT

#undef MIBDEF
{0}
};
#endif

#ifdef HOME_GATEWAY
mib_table_entry_T mib_portfw_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_PORTFW_IMPORT
#include "mibdef.h"
#undef MIB_PORTFW_IMPORT

#undef MIBDEF
{0}
};

mib_table_entry_T mib_ipfilter_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_IPFILTER_IMPORT
#include "mibdef.h"
#undef MIB_IPFILTER_IMPORT

#undef MIBDEF
{0}
};
	
mib_table_entry_T mib_portfilter_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_PORTFILTER_IMPORT
#include "mibdef.h"
#undef MIB_PORTFILTER_IMPORT

#undef MIBDEF
{0}
};
	
mib_table_entry_T mib_macfilter_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_MACFILTER_IMPORT
#include "mibdef.h"
#undef MIB_MACFILTER_IMPORT

#undef MIBDEF
{0}
};
	
mib_table_entry_T mib_triggerport_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_TRIGGERPORT_IMPORT
#include "mibdef.h"
#undef MIB_TRIGGERPORT_IMPORT

#undef MIBDEF
{0}
};

mib_table_entry_T mib_urlfilter_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_URLFILTER_IMPORT
#include "mibdef.h"
#undef MIB_URLFILTER_IMPORT

#undef MIBDEF
{0}
};

#if defined(VLAN_CONFIG_SUPPORTED)
mib_table_entry_T mib_vlanconfig_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_VLAN_CONFIG_IMPORT
#include "mibdef.h"
#undef MIB_VLAN_CONFIG_IMPORT

#undef MIBDEF
{0}
};
#endif

#ifdef ROUTE_SUPPORT
mib_table_entry_T mib_staticroute_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_STATICROUTE_IMPORT
#include "mibdef.h"
#undef MIB_STATICROUTE_IMPORT

#undef MIBDEF
{0}
};	
#endif


#ifdef VPN_SUPPORT
mib_table_entry_T mib_ipsectunnel_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_IPSECTUNNEL_IMPORT
#include "mibdef.h"
#undef MIB_IPSECTUNNEL_IMPORT

#undef MIBDEF
{0}
};
#endif

#endif // #ifdef HOME_GATEWAY

#ifdef TLS_CLIENT
mib_table_entry_T mib_certroot_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_CERTROOT_IMPORT
#include "mibdef.h"
#undef MIB_CERTROOT_IMPORT

#undef MIBDEF
{0}
};	

mib_table_entry_T mib_certuser_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_CERTUSER_IMPORT
#include "mibdef.h"
#undef MIB_CERTUSER_IMPORT
{0}
};		
#endif

#if defined(GW_QOS_ENGINE)
mib_table_entry_T mib_qos_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_QOS_IMPORT
#include "mibdef.h"
#undef MIB_QOS_IMPORT
{0}
};	
#endif

#if defined(QOS_BY_BANDWIDTH)
mib_table_entry_T mib_qos_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_IPQOS_IMPORT
#include "mibdef.h"
#undef MIB_IPQOS_IMPORT
{0}
};	
#endif

//#ifdef CONFIG_RTK_MESH Keith remove
//new feature:Mesh enable/disable

//#ifdef	_MESH_ACL_ENABLE_ Keith remove
mib_table_entry_T mib_mech_acl_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_MESH_MACFILTER_IMPORT
#include "mibdef.h"
#undef MIB_MESH_MACFILTER_IMPORT
{0}
};	
//#endif Keith remove

#ifdef MIB_TLV
mib_table_entry_T mib_root_table[]={
{MIB_ROOT, "MIB_ROOT",		TABLE_LIST_T, 	0, _UNIT_SIZE(APMIB_T), _UNIT_SIZE(APMIB_T), _UNIT_SIZE(APMIB_T), 0, mib_table},
{0}
};
#else
mib_table_entry_T mib_root_table[]={
{MIB_ROOT, "MIB_ROOT",		TABLE_LIST_T, 	0, _UNIT_SIZE(APMIB_T)},
{0}
};

#endif

mib_table_entry_T mib_table[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_IMPORT
#include "mibdef.h"
#undef MIB_IMPORT

#undef MIBDEF
{0}
};


mib_table_entry_T wlan_acl_addr_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_WLAN_MACFILTER_IMPORT
#include "mibdef.h"
#undef MIB_WLAN_MACFILTER_IMPORT

#undef MIBDEF
{0}
};

mib_table_entry_T wlan_wds_tbl[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_WDS_IMPORT
#include "mibdef.h"
#undef MIB_WDS_IMPORT

#undef MIBDEF
{0}
};

/*
 * When using flash (set/get/all) command to access the MIB of below table,
 * it needs append a keyword "WLANx_" in ahead of mib name.
 * When access default setting, it needs appened a keyword "DEF_" in front of
 * "WLANx_" keyword.
 */

mib_table_entry_T mib_wlan_table[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBWLANID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBWLANID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif

#define MIB_CONFIG_WLAN_SETTING_IMPORT
#include "mibdef.h"
#undef MIB_CONFIG_WLAN_SETTING_IMPORT

#undef MIBDEF

{0}
};

mib_table_entry_T hwmib_root_table[]={
#ifdef MIB_TLV
{_MIBID_NAME(HW_ROOT),		TABLE_LIST_T, 	0, _UNIT_SIZE(HW_SETTING_T), _UNIT_SIZE(HW_SETTING_T), _UNIT_SIZE(HW_SETTING_T), 0, hwmib_table},
#else
{_MIBID_NAME(HW_ROOT),		TABLE_LIST_T, 	0, _UNIT_SIZE(HW_SETTING_T)},
#endif
{0}
};

/*
 * When using flash (set/get/all) command to access the MIB of below table,
 * it needs append a keyword "HW_" in ahead of mib name.
 */

mib_table_entry_T hwmib_table[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBHWID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBHWID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif
#define MIB_HW_IMPORT
#include "mibdef.h"
#undef MIB_HW_IMPORT

#undef MIBDEF
{0}
};


/*
 * When using flash (set/get/all) command to access the MIB of below table,
 * it needs append a keyword "HW_WLANx_" in ahead of mib name.
 */

mib_table_entry_T hwmib_wlan_table[]={
#ifdef MIB_TLV
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
		{_MIBHWID_NAME(_mib_name), _mib_type, _OFFSET_SIZE_FIELD(_mib_parents_ctype, _cname), _UNIT_SIZE(_ctype), _default_value, _next_tbl},
#else
#define MIBDEF(_ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
			{_MIBHWID_NAME(_mib_name), _mib_type, FIELD_OFFSET(_mib_parents_ctype, _cname), FIELD_SIZE(_mib_parents_ctype, _cname)},
#endif
#define MIB_HW_WLAN_IMPORT
#include "mibdef.h"
#undef MIB_HW_WLAN_IMPORT

#undef MIBDEF
{0}
};

#ifdef MIB_TLV

mib_table_entry_T* mib_get_table(CONFIG_DATA_T type)
{
	switch(type)
	{
		case HW_SETTING:
			return hwmib_root_table;
		case DEFAULT_SETTING:
		case CURRENT_SETTING:
			return mib_root_table;
		default:
			return 0;
	}
	
}
#endif
