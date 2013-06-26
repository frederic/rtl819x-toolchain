/* 
 *  Unix SMB/CIFS implementation.
 *  RPC Pipe client / server routines
 *
 *  Copyright (C) Andrew Tridgell		1992-1997,
 *  Copyright (C) Gerald (Jerry) Carter		2006.
 *  Copyright (C) Guenther Deschner		2007-2008.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/* This is the implementation of the wks interface. */

#include "includes.h"

#undef DBGC_CLASS
#define DBGC_CLASS DBGC_RPC_SRV

/* WKS_Q_QUERY_INFO - probably a capabilities request */
typedef struct q_wks_query_info_info
{
	uint32 ptr_srv_name;         /* pointer (to server name?) */
	UNISTR2 uni_srv_name;        /* unicode server name starting with '\\' */
	uint16 switch_value;         /* info level 100 (0x64) */
} WKS_Q_QUERY_INFO;

/* WKS_INFO_100 - level 100 info */     
typedef struct wks_info_100_info
{ 
	uint32 platform_id;          /* 0x0000 01f4 - unknown */
	uint32 ptr_compname;       /* pointer to server name */
	uint32 ptr_lan_grp ;       /* pointer to domain name */
	uint32 ver_major;          /* 4 - unknown */
	uint32 ver_minor;          /* 0 - unknown */
	
	UNISTR2 uni_compname;      /* unicode server name */
	UNISTR2 uni_lan_grp ;      /* unicode domain name */
} WKS_INFO_100;

/* WKS_R_QUERY_INFO - probably a capabilities request */
typedef struct r_wks_query_info_info
{
	uint16 switch_value;          /* 100 (0x64) - switch value */

	/* for now, only level 100 is supported.  this should be an enum container */
	uint32 ptr_1;              /* pointer 1 */
	WKS_INFO_100 *wks100;      /* workstation info level 100 */

	NTSTATUS status;             /* return status */
} WKS_R_QUERY_INFO;

/*******************************************************************
 Fill in the values for the struct wkssvc_NetWkstaInfo100.
 ********************************************************************/

static void create_wks_info_100(WKS_INFO_100 *info100)
{
	info100->platform_id	 = PLATFORM_ID_NT;	/* unknown */
	info100->ver_major	 = lp_major_announce_version();
	info100->ver_minor	 = lp_minor_announce_version();

	init_buf_unistr2(&info100->uni_compname, &info100->ptr_compname, global_myname());
	init_buf_unistr2(&info100->uni_lan_grp, &info100->ptr_lan_grp, lp_workgroup());

	return;
}


/*******************************************************************
 Init
 ********************************************************************/

static void init_wks_q_query_info(WKS_Q_QUERY_INFO *q_u,
				char *server, uint16 switch_value)  
{
	DEBUG(5,("init_wks_q_query_info\n"));

	init_buf_unistr2(&q_u->uni_srv_name, &q_u->ptr_srv_name, server);
	q_u->switch_value = switch_value;
}

/*******************************************************************
 Reads or writes a WKS_Q_QUERY_INFO structure.
********************************************************************/

static bool wks_io_q_query_info(const char *desc, WKS_Q_QUERY_INFO *q_u, prs_struct *ps, int depth)
{
	if (q_u == NULL)
		return False;

	prs_debug(ps, depth, desc, "wks_io_q_query_info");
	depth++;

	if(!prs_align(ps))
		return False;

	if(!prs_uint32("ptr_srv_name", ps, depth, &q_u->ptr_srv_name))
		return False;
	if(!smb_io_unistr2("", &q_u->uni_srv_name, q_u->ptr_srv_name, ps, depth))
		return False;
	if(!prs_align(ps))
		return False;

	if(!prs_uint16("switch_value", ps, depth, &q_u->switch_value))
		return False;
	if(!prs_align(ps))
		return False;

	return True;
}

/*******************************************************************
 Reads or writes a WKS_INFO_100 structure.
********************************************************************/

static bool wks_io_wks_info_100(const char *desc, WKS_INFO_100 *inf, prs_struct *ps, int depth)
{
	if (inf == NULL)
		return False;

	prs_debug(ps, depth, desc, "wks_io_wks_info_100");
	depth++;

	if(!prs_align(ps))
		return False;

	if(!prs_uint32("platform_id ", ps, depth, &inf->platform_id)) /* 0x0000 01f4 - unknown */
		return False;
	if(!prs_uint32("ptr_compname", ps, depth, &inf->ptr_compname)) /* pointer to computer name */
		return False;
	if(!prs_uint32("ptr_lan_grp ", ps, depth, &inf->ptr_lan_grp)) /* pointer to LAN group name */
		return False;
	if(!prs_uint32("ver_major   ", ps, depth, &inf->ver_major)) /* 4 - major os version */
		return False;
	if(!prs_uint32("ver_minor   ", ps, depth, &inf->ver_minor)) /* 0 - minor os version */
		return False;

	if(!smb_io_unistr2("", &inf->uni_compname, inf->ptr_compname, ps, depth))
		return False;
	if(!prs_align(ps))
		return False;

	if(!smb_io_unistr2("", &inf->uni_lan_grp, inf->ptr_lan_grp , ps, depth))
		return False;
	if(!prs_align(ps))
		return False;

	return True;
}

/*******************************************************************
 Inits WKS_R_QUERY_INFO.

 only supports info level 100 at the moment.

 ********************************************************************/

static void init_wks_r_query_info(WKS_R_QUERY_INFO *r_u,
			   uint32 switch_value, WKS_INFO_100 *wks100,
			   NTSTATUS status)  
{
	DEBUG(5,("init_wks_r_unknown_0: %d\n", __LINE__));

	r_u->switch_value = switch_value;  /* same as in request */

	r_u->ptr_1     = 1;          /* pointer 1 */
	r_u->wks100    = wks100;

	r_u->status    = status;
}

/*******************************************************************
 Reads or writes a structure.
********************************************************************/

static bool wks_io_r_query_info(const char *desc, WKS_R_QUERY_INFO *r_u, prs_struct *ps, int depth)
{
	if (r_u == NULL)
		return False;

	prs_debug(ps, depth, desc, "wks_io_r_query_info");
	depth++;

	if(!prs_align(ps))
		return False;

	if(!prs_uint16("switch_value", ps, depth, &r_u->switch_value)) /* level 100 (0x64) */
		return False;
	if(!prs_align(ps))
		return False;

	if(!prs_uint32("ptr_1       ", ps, depth, &r_u->ptr_1)) /* pointer 1 */
		return False;
	if(!wks_io_wks_info_100("inf", r_u->wks100, ps, depth))
		return False;

	if(!prs_ntstatus("status      ", ps, depth, &r_u->status))
		return False;

	return True;
}
/*******************************************************************
 api_wks_query_info
 ********************************************************************/
static NTSTATUS _wks_query_info(pipes_struct *p, WKS_Q_QUERY_INFO *q_u, WKS_R_QUERY_INFO *r_u)
{
	WKS_INFO_100 *wks100 = NULL;

	wks100 = TALLOC_ZERO_P(p->mem_ctx, WKS_INFO_100);

	if (!wks100)
		return NT_STATUS_NO_MEMORY;

	create_wks_info_100(wks100);
	init_wks_r_query_info(r_u, q_u->switch_value, wks100, NT_STATUS_OK);

	return r_u->status;
}

static bool api_wks_query_info(pipes_struct *p)
{
	WKS_Q_QUERY_INFO q_u;
	WKS_R_QUERY_INFO r_u;
	prs_struct *data = &p->in_data.data;
	prs_struct *rdata = &p->out_data.rdata;

	ZERO_STRUCT(q_u);
	ZERO_STRUCT(r_u);

	/* grab the net share enum */
	if(!wks_io_q_query_info("", &q_u, data, 0))
		return false;

	r_u.status = _wks_query_info(p, &q_u, &r_u);
	/* store the response in the SMB stream */
	if(!wks_io_r_query_info("", &r_u, rdata, 0))
		return false; 
	return true;
}

/*******************************************************************
 \PIPE\wkssvc commands
 ********************************************************************/
static struct api_struct api_wkssvc_cmds[] =
{
	{ "WKS_Q_QUERY_INFO", NDR_WKSSVC_NETWKSTAGETINFO, api_wks_query_info }
};

void wkssvc_get_pipe_fns(struct api_struct **fns, int *n_fns)
{
	*fns = api_wkssvc_cmds;
	*n_fns = sizeof(api_wkssvc_cmds) / sizeof(struct api_struct);
}

NTSTATUS rpc_wkssvc_init(void)
{
	return rpc_pipe_register_commands(SMB_RPC_INTERFACE_VERSION, "wkssvc", "wkssvc", api_wkssvc_cmds, sizeof(api_wkssvc_cmds) / sizeof(struct api_struct));
}

/********************************************************************
 only supports info level 100 at the moment.
 ********************************************************************/
#if 0
WERROR _wkssvc_NetWkstaGetInfo(pipes_struct *p, struct wkssvc_NetWkstaGetInfo *r)
{
	struct wkssvc_NetWkstaInfo100 *wks100 = NULL;
	
	/* We only support info level 100 currently */
	
	if ( r->in.level != 100 ) {
		return WERR_UNKNOWN_LEVEL;
	}

	if ( (wks100 = TALLOC_ZERO_P(p->mem_ctx, struct wkssvc_NetWkstaInfo100)) == NULL ) {
		return WERR_NOMEM;
	}

	create_wks_info_100( wks100 );
	
	r->out.info->info100 = wks100;

	return WERR_OK;
}
#endif

#if 0
/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetWkstaSetInfo(pipes_struct *p, struct wkssvc_NetWkstaSetInfo *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetWkstaEnumUsers(pipes_struct *p, struct wkssvc_NetWkstaEnumUsers *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrWkstaUserGetInfo(pipes_struct *p, struct wkssvc_NetrWkstaUserGetInfo *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrWkstaUserSetInfo(pipes_struct *p, struct wkssvc_NetrWkstaUserSetInfo *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetWkstaTransportEnum(pipes_struct *p, struct wkssvc_NetWkstaTransportEnum *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrWkstaTransportAdd(pipes_struct *p, struct wkssvc_NetrWkstaTransportAdd *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrWkstaTransportDel(pipes_struct *p, struct wkssvc_NetrWkstaTransportDel *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrUseAdd(pipes_struct *p, struct wkssvc_NetrUseAdd *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrUseGetInfo(pipes_struct *p, struct wkssvc_NetrUseGetInfo *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrUseDel(pipes_struct *p, struct wkssvc_NetrUseDel *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrUseEnum(pipes_struct *p, struct wkssvc_NetrUseEnum *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrMessageBufferSend(pipes_struct *p, struct wkssvc_NetrMessageBufferSend *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrWorkstationStatisticsGet(pipes_struct *p, struct wkssvc_NetrWorkstationStatisticsGet *r) 
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrLogonDomainNameAdd(pipes_struct *p, struct wkssvc_NetrLogonDomainNameAdd *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrLogonDomainNameDel(pipes_struct *p, struct wkssvc_NetrLogonDomainNameDel *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrJoinDomain(pipes_struct *p, struct wkssvc_NetrJoinDomain *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrUnjoinDomain(pipes_struct *p, struct wkssvc_NetrUnjoinDomain *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrRenameMachineInDomain(pipes_struct *p, struct wkssvc_NetrRenameMachineInDomain *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrValidateName(pipes_struct *p, struct wkssvc_NetrValidateName *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrGetJoinInformation(pipes_struct *p, struct wkssvc_NetrGetJoinInformation *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrGetJoinableOus(pipes_struct *p, struct wkssvc_NetrGetJoinableOus *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 _wkssvc_NetrJoinDomain2
 ********************************************************************/

WERROR _wkssvc_NetrJoinDomain2(pipes_struct *p,
			       struct wkssvc_NetrJoinDomain2 *r)
{
#if 1 /* tonywu */
	return WERR_NOT_SUPPORTED;
#else
	struct libnet_JoinCtx *j = NULL;
	char *cleartext_pwd = NULL;
	char *admin_domain = NULL;
	char *admin_account = NULL;
	WERROR werr;
	struct nt_user_token *token = p->pipe_user.nt_user_token;

	if (!r->in.domain_name) {
		return WERR_INVALID_PARAM;
	}

	if (!r->in.admin_account || !r->in.encrypted_password) {
		return WERR_INVALID_PARAM;
	}

	if (!user_has_privileges(token, &se_machine_account) &&
	    !nt_token_check_domain_rid(token, DOMAIN_GROUP_RID_ADMINS) &&
	    !nt_token_check_domain_rid(token, BUILTIN_ALIAS_RID_ADMINS)) {
		DEBUG(5,("_wkssvc_NetrJoinDomain2: account doesn't have "
			"sufficient privileges\n"));
		return WERR_ACCESS_DENIED;
	}

	if ((r->in.join_flags & WKSSVC_JOIN_FLAGS_MACHINE_PWD_PASSED) ||
	    (r->in.join_flags & WKSSVC_JOIN_FLAGS_JOIN_UNSECURE)) {
		return WERR_NOT_SUPPORTED;
	}

	werr = decode_wkssvc_join_password_buffer(p->mem_ctx,
						  r->in.encrypted_password,
						  &p->session_key,
						  &cleartext_pwd);
	if (!W_ERROR_IS_OK(werr)) {
		return werr;
	}

	split_domain_user(p->mem_ctx,
			  r->in.admin_account,
			  &admin_domain,
			  &admin_account);

	werr = libnet_init_JoinCtx(p->mem_ctx, &j);
	if (!W_ERROR_IS_OK(werr)) {
		return werr;
	}

	j->in.domain_name	= r->in.domain_name;
	j->in.account_ou	= r->in.account_ou;
	j->in.join_flags	= r->in.join_flags;
	j->in.admin_account	= admin_account;
	j->in.admin_password	= cleartext_pwd;
	j->in.debug		= true;
	j->in.modify_config     = lp_config_backend_is_registry();
	j->in.msg_ctx		= smbd_messaging_context();

	become_root();
	werr = libnet_Join(p->mem_ctx, j);
	unbecome_root();

	if (!W_ERROR_IS_OK(werr)) {
		DEBUG(5,("_wkssvc_NetrJoinDomain2: libnet_Join failed with: %s\n",
			j->out.error_string ? j->out.error_string :
			dos_errstr(werr)));
	}

	TALLOC_FREE(j);
	return werr;
#endif
}

/********************************************************************
 _wkssvc_NetrUnjoinDomain2
 ********************************************************************/

WERROR _wkssvc_NetrUnjoinDomain2(pipes_struct *p,
				 struct wkssvc_NetrUnjoinDomain2 *r)
{
#if 1 /* tonywu */
	return WERR_NOT_SUPPORTED;
#else
	struct libnet_UnjoinCtx *u = NULL;
	char *cleartext_pwd = NULL;
	char *admin_domain = NULL;
	char *admin_account = NULL;
	WERROR werr;
	struct nt_user_token *token = p->pipe_user.nt_user_token;

	if (!r->in.account || !r->in.encrypted_password) {
		return WERR_INVALID_PARAM;
	}

	if (!user_has_privileges(token, &se_machine_account) &&
	    !nt_token_check_domain_rid(token, DOMAIN_GROUP_RID_ADMINS) &&
	    !nt_token_check_domain_rid(token, BUILTIN_ALIAS_RID_ADMINS)) {
		DEBUG(5,("_wkssvc_NetrUnjoinDomain2: account doesn't have "
			"sufficient privileges\n"));
		return WERR_ACCESS_DENIED;
	}

	werr = decode_wkssvc_join_password_buffer(p->mem_ctx,
						  r->in.encrypted_password,
						  &p->session_key,
						  &cleartext_pwd);
	if (!W_ERROR_IS_OK(werr)) {
		return werr;
	}

	split_domain_user(p->mem_ctx,
			  r->in.account,
			  &admin_domain,
			  &admin_account);

	werr = libnet_init_UnjoinCtx(p->mem_ctx, &u);
	if (!W_ERROR_IS_OK(werr)) {
		return werr;
	}

	u->in.domain_name	= lp_realm();
	u->in.unjoin_flags	= r->in.unjoin_flags |
				  WKSSVC_JOIN_FLAGS_JOIN_TYPE;
	u->in.admin_account	= admin_account;
	u->in.admin_password	= cleartext_pwd;
	u->in.debug		= true;
	u->in.modify_config     = lp_config_backend_is_registry();
	u->in.msg_ctx		= smbd_messaging_context();

	become_root();
	werr = libnet_Unjoin(p->mem_ctx, u);
	unbecome_root();

	if (!W_ERROR_IS_OK(werr)) {
		DEBUG(5,("_wkssvc_NetrUnjoinDomain2: libnet_Unjoin failed with: %s\n",
			u->out.error_string ? u->out.error_string :
			dos_errstr(werr)));
	}

	TALLOC_FREE(u);
	return werr;
#endif
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrRenameMachineInDomain2(pipes_struct *p, struct wkssvc_NetrRenameMachineInDomain2 *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrValidateName2(pipes_struct *p, struct wkssvc_NetrValidateName2 *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrGetJoinableOus2(pipes_struct *p, struct wkssvc_NetrGetJoinableOus2 *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrAddAlternateComputerName(pipes_struct *p, struct wkssvc_NetrAddAlternateComputerName *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrRemoveAlternateComputerName(pipes_struct *p, struct wkssvc_NetrRemoveAlternateComputerName *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrSetPrimaryComputername(pipes_struct *p, struct wkssvc_NetrSetPrimaryComputername *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}

/********************************************************************
 ********************************************************************/

WERROR _wkssvc_NetrEnumerateComputerNames(pipes_struct *p, struct wkssvc_NetrEnumerateComputerNames *r)
{
	/* FIXME: Add implementation code here */
	p->rng_fault_state = True;
	return WERR_NOT_SUPPORTED;
}
#endif
