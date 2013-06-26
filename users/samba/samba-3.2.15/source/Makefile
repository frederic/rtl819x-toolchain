#########################################################################
# Makefile.in for Samba - rewritten for autoconf support
# Copyright Andrew Tridgell 1992-1998
# Copyright (C) 2001 by Martin Pool <mbp@samba.org>
# Copyright Andrew Bartlett 2002
# Copyright (C) 2003 Jim McDonough <jmcd@us.ibm.com>
# Copyright (C) 2002-2003 Jelmer Vernooij <jelmer@samba.org>
# Copyright (C) 2006 Lars Mueller <lars@samba.org>
# Copyright (C) 2007 James Peach <jpeach@samba.org>
# Copyright (C) 2008 Michael Adam <obnox@samba.org>
###########################################################################

prefix=
exec_prefix=${prefix}
sysconfdir=${prefix}/etc
localstatedir=${prefix}/var
datarootdir=${prefix}/share

selftest_prefix=./st
smbtorture4_path=

LIBS=

CC=mips-linux-gcc
AR=mips-linux-ar
RANLIB=mips-linux-ranlib
SHLD=${CC} ${CFLAGS}
LIB_PATH_VAR=LD_LIBRARY_PATH

## Dynamic shared libraries build settings
DSO_EXPORTS_CMD=-Wl,--version-script,$(srcdir)/exports/`basename $@ | sed 's/so\(.[0-9]\{1,\}\)\{0,1\}$$/syms/'`
DSO_EXPORTS=$(DSO_EXPORTS_CMD)
SHLD_DSO = $(SHLD) $(LDSHFLAGS) $(DSO_EXPORTS) -o $@

# The MODULE_EXPORTS variable contains the platform-specific linker flags
# needed to restrict the exports for VFS, IDMAP, RPC and PASSDB modules.
MODULE_EXPORTS=

CFLAGS+= -Os -D_SAMBA_BUILD_=3
CFLAGS+= -DAVM_NO_PRINTING -DAVM_NO_CHPASSWD
CFLAGS+= -DUSE_READAHEAD #-DUSE_FALLOCATE
CFLAGS+= -DAVM_NO_DEBUG #-DUSE_LOGGING
CFLAGS+= #-DUSE_HOMEDIR
CFLAGS+= -ffunction-sections -fdata-sections
CFLAGS+= -fstrict-aliasing -fstrict-overflow 
CPPFLAGS=-DHAVE_CONFIG_H  -Iinclude -I./include -I. -I. -I./lib/replace -I./lib/talloc -I./lib/tdb/include -I./libaddns -I./librpc -I./popt

EXEEXT=
LDSHFLAGS= -shared -Wl,-Bsymbolic -Wl,-z,relro -L./bin
LDFLAGS= -Wl,-z,relro -L./bin -Wl,--gc-sections

WINBIND_NSS_LDSHFLAGS=-shared -Wl,-Bsymbolic -L./bin
AWK=gawk
PICFLAG=#-fPIC
DYNEXP=#-Wl,--export-dynamic
PERL=/usr/bin/perl

PIDL_ARGS=

TERMLDFLAGS=
TERMLIBS=
PRINT_LIBS=
AUTH_LIBS=
ACL_LIBS=
PASSDB_LIBS= 
IDMAP_LIBS=
KRB5LIBS=
LDAP_LIBS=
NSCD_LIBS=
UUID_LIBS=
WINBIND_LIBS=
WINBIND_NSS_EXTRA_LIBS=
WINBIND_NSS_PTHREAD=
PAM_WINBIND_EXTRA_LIBS=
DNSSD_LIBS=
POPT_LIBS=$(POPT_OBJ)
LIBTALLOC_LIBS=
LIBTDB_LIBS=
LIBNETAPI_LIBS=

INSTALLCMD=/usr/bin/install -c
INSTALLLIBCMD_SH=$(INSTALLCMD)
INSTALLLIBCMD_A=:
UNINSTALLLIBCMD_SH=rm -f
UNINSTALLLIBCMD_A=:


srcdir=`pwd`
builddir=`pwd`
SHELL=/bin/sh
DESTDIR=/

# XXX: Perhaps this should be /bin/sh instead -- apparently autoconf
# will search for a POSIX-compliant shell, and that might not be
# /bin/sh on some platforms.  I guess it's not a big problem -- mbp

# See the autoconf manual "Installation Directory Variables" for a
# discussion of the subtle use of these variables.

BASEDIR= /etc/samba
BINDIR = /bin
LIBDIR = /lib
CONFIGDIR = /etc/samba
VARDIR = /var/samba
SBINDIR = /bin
ROOTSBINDIR = ${SBINDIR}

INCLUDEDIR=${prefix}/include
PAMMODULESDIR = ${LIBDIR}/security
VFSLIBDIR = $(LIBDIR)/vfs
PDBLIBDIR = $(LIBDIR)/pdb
RPCLIBDIR = $(LIBDIR)/rpc
IDMAPLIBDIR = $(LIBDIR)/idmap
NSSINFOLIBDIR = $(LIBDIR)/nss_info
CHARSETLIBDIR = $(LIBDIR)/charset
AUTHLIBDIR = $(LIBDIR)/auth
CONFIGLIBDIR = $(LIBDIR)/config
MANDIR = ${datarootdir}/man
DATADIR = ${datarootdir}

# The permissions to give the executables and other data
INSTALLPERMS_BIN = 0755
INSTALLPERMS_DATA = 0644

# set these to where to find various files
# These can be overridden by command line switches (see smbd(8))
# or in smb.conf (see smb.conf(5))
LOGFILEBASE = ${VARDIR}
CONFIGFILE = $(CONFIGDIR)/smb.conf
LMHOSTSFILE = $(CONFIGDIR)/lmhosts
CTDBDIR = 

# This is where smbpasswd et al go
PRIVATEDIR = ${VARDIR}

SMB_PASSWD_FILE = $(CONFIGDIR)/smbpasswd
PRIVATE_DIR = $(PRIVATEDIR)

# This is where SWAT images and help files go
SWATDIR = ${prefix}/samba

# the directory where lock files go
LOCKDIR = ${VARDIR}/samba

# FHS directories; equal to LOCKDIR if not using --with-fhs
CACHEDIR = ${LOCKDIR}
STATEDIR = ${LOCKDIR}

# Where to look for (and install) codepage databases.
CODEPAGEDIR = ${LIBDIR}

# the directory where pid files go
PIDDIR = ${VARDIR}/samba

FLAGS1 = $(CFLAGS) -I$(srcdir)/popt -I$(srcdir)/iniparser/src -Iinclude -I./include  -I. -I. -I./lib/replace -I./lib/talloc -I./lib/tdb/include -I./libaddns -I./librpc $(CPPFLAGS)
FLAGS2 =
FLAGS3 =
FLAGS4 = -I$(CTDBDIR)/include
FLAGS5 = $(FLAGS1) $(FLAGS2) $(FLAGS3) $(FLAGS4)
FLAGS  = $(ISA) $(FLAGS5) -I$(srcdir)/lib -D_SAMBA_BUILD_=3

PATH_FLAGS = -DSMB_PASSWD_FILE=\"$(SMB_PASSWD_FILE)\" \
	-DPRIVATE_DIR=\"$(PRIVATE_DIR)\" \
	-DCONFIGFILE=\"$(CONFIGFILE)\" \
	-DBINDIR=\"$(BINDIR)\" \
	-DLMHOSTSFILE=\"$(LMHOSTSFILE)\" \
	-DLOCKDIR=\"$(LOCKDIR)\" \
	-DPIDDIR=\"$(PIDDIR)\" \
	-DLIBDIR=\"$(LIBDIR)\" \
	-DLOGFILEBASE=\"$(LOGFILEBASE)\" \
	-DCONFIGDIR=\"$(CONFIGDIR)\" \
	-DCODEPAGEDIR=\"$(CODEPAGEDIR)\" \
	-DCACHEDIR=\"$(CACHEDIR)\" \
	-DSTATEDIR=\"$(STATEDIR)\"

# Note that all executable programs now provide for an optional executable suffix.

BIN_PROGS = bin/smbd bin/nmbd bin/smbpasswd

PAM_MODULES = 

NSS_MODULES = 

SCRIPTS = $(srcdir)/script/smbtar $(builddir)/script/findsmb

VFS_MODULES =  bin/recycle.so bin/audit.so bin/extd_audit.so bin/full_audit.so bin/netatalk.so bin/fake_perms.so bin/default_quota.so bin/readonly.so bin/cap.so bin/expand_msdfs.so bin/shadow_copy.so bin/shadow_copy2.so bin/xattr_tdb.so bin/streams_xattr.so bin/streams_depot.so bin/readahead.so bin/syncops.so bin/smb_traffic_analyzer.so
PDB_MODULES = 
RPC_MODULES = 
IDMAP_MODULES = 
CHARSET_MODULES =  bin/CP850.so bin/CP437.so
AUTH_MODULES =  bin/script.so
NSS_INFO_MODULES = 
MODULES = $(VFS_MODULES) $(PDB_MODULES) $(RPC_MODULES) $(IDMAP_MODULES) \
          $(CHARSET_MODULES) $(AUTH_MODULES) $(NSS_INFO_MODULES)

EXTRA_ALL_TARGETS = 

#####################################################################
# object file lists
######################################################################

TDB_OBJ = lib/util_tdb.o \
	  lib/dbwrap.o lib/dbwrap_tdb.o \
	  lib/dbwrap_rbt.o $(LIBTDB_OBJ0)

SMBLDAP_OBJ =  

VERSION_OBJ = lib/version.o

WBCOMMON_OBJ = nsswitch/wb_common.o

AFS_OBJ = #lib/afs.o

AFS_SETTOKEN_OBJ = #lib/afs_settoken.o

SERVER_MUTEX_OBJ = lib/server_mutex.o

PASSCHANGE_OBJ = #libsmb/passchange.o


LIBNDR_OBJ = librpc/ndr/ndr_basic.o \
	     librpc/ndr/ndr.o \
	     librpc/ndr/ndr_misc.o \
	     librpc/gen_ndr/ndr_misc.o \
	     librpc/gen_ndr/ndr_security.o \
	     librpc/ndr/ndr_sec_helper.o \
	     librpc/ndr/ndr_string.o \
	     librpc/ndr/sid.o \
	     librpc/ndr/uuid.o

RPCCLIENT_NDR_OBJ = #rpc_client/ndr.o

LIBNDR_GEN_OBJ0 = #librpc/gen_ndr/ndr_samr.o \
		  librpc/gen_ndr/ndr_lsa.o

LIBNDR_GEN_OBJ1 = #librpc/gen_ndr/ndr_netlogon.o

#LIBNDR_GEN_OBJ = librpc/gen_ndr/ndr_wkssvc.o \
		 $(LIBNDR_GEN_OBJ0) \
		 librpc/gen_ndr/ndr_dfs.o \
		 librpc/gen_ndr/ndr_echo.o \
		 librpc/gen_ndr/ndr_winreg.o \
		 librpc/gen_ndr/ndr_initshutdown.o \
		 librpc/gen_ndr/ndr_srvsvc.o \
		 librpc/gen_ndr/ndr_eventlog.o \
		 $(LIBNDR_GEN_OBJ1) \
		 librpc/gen_ndr/ndr_dssetup.o \
		 librpc/gen_ndr/ndr_notify.o \
		 librpc/gen_ndr/ndr_xattr.o \
		 librpc/gen_ndr/ndr_ntsvcs.o


LIBNDR_GEN_OBJ = \
		 $(LIBNDR_GEN_OBJ0) \
		 librpc/gen_ndr/ndr_svcctl.o \
		 librpc/gen_ndr/ndr_srvsvc.o \
		 librpc/gen_ndr/ndr_notify.o

RPC_PARSE_OBJ0 = rpc_parse/parse_prs.o rpc_parse/parse_misc.o

# this includes only the low level parse code, not stuff
# that requires knowledge of security contexts
RPC_PARSE_OBJ1 = $(RPC_PARSE_OBJ0) #rpc_parse/parse_sec.o

#RPC_PARSE_OBJ2 = rpc_parse/parse_rpc.o \
		 rpc_client/init_netlogon.o \
		 rpc_client/init_lsa.o \
		 rpc_client/init_srvsvc.o

RPC_PARSE_OBJ2 = rpc_parse/parse_rpc.o \
		 rpc_client/init_srvsvc.o

LIBREPLACE_OBJ =  lib/replace/replace.o lib/replace/snprintf.o lib/replace/getpass.o lib/replace/strptime.o lib/replace/getifaddrs.o

SOCKET_WRAPPER_OBJ = 
NSS_WRAPPER_OBJ = 

LIBSAMBAUTIL_OBJ = $(LIBTALLOC_OBJ0) \
		$(LIBREPLACE_OBJ) \
		$(SOCKET_WRAPPER_OBJ) \
		$(NSS_WRAPPER_OBJ)

LIB_WITHOUT_PROTO_OBJ = $(LIBSAMBAUTIL_OBJ) \
	lib/messages.o librpc/gen_ndr/ndr_messaging.o lib/messages_local.o \
	lib/messages_ctdbd.o lib/packet.o lib/ctdbd_conn.o lib/talloc_stack.o \
	lib/interfaces.o lib/rbtree.o lib/memcache.o \
	lib/util_transfer_file.o lib/async_req.o \
	$(TDB_OBJ)

#LIB_WITH_PROTO_OBJ = $(VERSION_OBJ) lib/charcnv.o lib/debug.o lib/fault.o \
	  lib/interface.o lib/md4.o \
	  lib/pidfile.o \
	  lib/signal.o lib/system.o lib/sendfile.o lib/recvfile.o lib/time.o \
	  lib/ufc.o lib/genrand.o lib/username.o \
	  lib/util_pw.o lib/access.o lib/smbrun.o \
	  lib/bitmap.o lib/crc32.o lib/dprintf.o \
	  lib/xfile.o lib/wins_srv.o $(UTIL_REG_OBJ) \
	  lib/util_str.o lib/clobber.o lib/util_sid.o lib/util_uuid.o \
	  lib/util_unistr.o lib/util_file.o lib/data_blob.o \
	  lib/util.o lib/util_sock.o lib/sock_exec.o lib/util_sec.o \
	  lib/substitute.o lib/fsusage.o lib/dbwrap_util.o \
	  lib/ms_fnmatch.o lib/select.o lib/errmap_unix.o \
	  lib/tallocmsg.o lib/dmallocmsg.o libsmb/smb_signing.o \
	  lib/md5.o lib/hmacmd5.o lib/arc4.o lib/iconv.o \
	  lib/pam_errors.o intl/lang_tdb.o lib/conn_tdb.o \
	  lib/adt_tree.o lib/gencache.o \
	  lib/module.o lib/events.o \
	  lib/secdesc.o lib/util_seaccess.o lib/secace.o lib/secacl.o \
	  libads/krb5_errs.o lib/system_smbd.o lib/audit.o $(LIBNDR_OBJ) \
	  lib/file_id.o

LIB_WITH_PROTO_OBJ = $(VERSION_OBJ) lib/charcnv.o lib/debug.o lib/fault.o \
	  lib/interface.o lib/md4.o \
	  lib/pidfile.o \
	  lib/signal.o lib/system.o lib/sendfile.o lib/recvfile.o lib/time.o \
	  lib/ufc.o lib/genrand.o lib/username.o \
	  lib/util_pw.o lib/access.o lib/smbrun.o \
	  lib/bitmap.o lib/crc32.o lib/dprintf.o \
	  lib/xfile.o lib/wins_srv.o $(UTIL_REG_OBJ) \
	  lib/util_str.o lib/clobber.o lib/util_sid.o lib/util_uuid.o \
	  lib/util_unistr.o lib/util_file.o lib/data_blob.o \
	  lib/util.o lib/util_sock.o lib/sock_exec.o lib/util_sec.o \
	  lib/substitute.o lib/fsusage.o lib/dbwrap_util.o \
	  lib/ms_fnmatch.o lib/select.o lib/errmap_unix.o \
	  lib/tallocmsg.o lib/dmallocmsg.o libsmb/smb_signing.o \
	  lib/md5.o lib/hmacmd5.o lib/arc4.o lib/iconv.o \
	  intl/lang_tdb.o lib/conn_tdb.o \
	  lib/gencache.o \
	  lib/module.o lib/events.o \
	  lib/secdesc.o lib/util_seaccess.o lib/secace.o lib/secacl.o \
	  lib/system_smbd.o $(LIBNDR_OBJ) \
	  lib/file_id.o

LIB_OBJ = $(LIB_WITHOUT_PROTO_OBJ) $(LIB_WITH_PROTO_OBJ)

LIB_DUMMY_OBJ = lib/dummysmbd.o lib/dummyroot.o
LIB_NONSMBD_OBJ = $(LIB_OBJ) $(LIB_DUMMY_OBJ)

READLINE_OBJ = #lib/readline.o

# Also depends on  $(SECRETS_OBJ) $(LIBSAMBA_OBJ)
# Be sure to include them into your application
POPT_LIB_OBJ = #lib/popt_common.o

PARAM_WITHOUT_REG_OBJ = dynconfig.o param/loadparm.o param/params.o param/util.o \
		        lib/sharesec.o #lib/ldap_debug_handler.o

PARAM_REG_ADD_OBJ = $(REG_SMBCONF_OBJ) $(LIBSMBCONF_OBJ) $(PRIVILEGES_BASIC_OBJ)
PARAM_OBJ = $(PARAM_WITHOUT_REG_OBJ) $(PARAM_REG_ADD_OBJ)

KRBCLIENT_OBJ = libads/ads_status.o #libads/kerberos.o 

LIBADDNS_OBJ0 = libaddns/dnsrecord.o libaddns/dnsutils.o  libaddns/dnssock.o \
	       libaddns/dnsgss.o libaddns/dnsmarshall.o
LIBADDNS_OBJ = $(LIBADDNS_OBJ0) $(SOCKET_WRAPPER_OBJ) $(LIBTALLOC_OBJ0)

LIBGPO_OBJ0 = #libgpo/gpo_ldap.o libgpo/gpo_ini.o libgpo/gpo_util.o \
	      libgpo/gpo_fetch.o libgpo/gpo_filesync.o libgpo/gpo_sec.o
LIBGPO_OBJ = #$(LIBGPO_OBJ0)

LIBADS_OBJ = #libads/ldap.o libads/ldap_printer.o \
	     libads/sasl.o libads/sasl_wrapping.o \
	     libads/krb5_setpw.o libads/ldap_user.o \
	     libads/ads_struct.o libads/kerberos_keytab.o \
             libads/disp_sec.o libads/ads_utils.o libads/ldap_utils.o \
	     libads/ldap_schema.o libads/util.o libads/ndr.o

LIBADS_SERVER_OBJ = #libads/kerberos_verify.o libads/authdata.o \
		    librpc/ndr/ndr_krb5pac.o \
		    librpc/gen_ndr/ndr_krb5pac.o

SECRETS_OBJ = passdb/secrets.o passdb/machine_sid.o

#LIBNBT_OBJ = libcli/nbt/nbtname.o \
	     librpc/gen_ndr/ndr_nbt.o \
	     librpc/gen_ndr/ndr_svcctl.o

LIBNBT_OBJ = #librpc/gen_ndr/ndr_svcctl.o

LIBNMB_OBJ = libsmb/unexpected.o \
	     libsmb/namequery.o libsmb/conncache.o libads/dns.o

LIBSAMBA_OBJ = libsmb/ntlm_check.o \
	       libsmb/ntlmssp.o \
	       libsmb/ntlmssp_sign.o \
               libsmb/nterr.o \
               libsmb/errormap.o \
               libsmb/smbdes.o \
               libsmb/smbencrypt.o \
               libsmb/ntlmssp_parse.o \
               $(RPC_PARSE_OBJ1) \
               $(SECRETS_OBJ)

#LIBSMB_OBJ = libsmb/clientgen.o libsmb/cliconnect.o libsmb/clifile.o \
	     libsmb/clikrb5.o libsmb/clispnego.o libsmb/asn1.o \
	     libsmb/clirap.o libsmb/clierror.o libsmb/climessage.o \
	     libsmb/clireadwrite.o libsmb/clilist.o libsmb/cliprint.o \
	     libsmb/clitrans.o libsmb/clisecdesc.o libsmb/clidgram.o \
	     libsmb/clistr.o libsmb/cliquota.o libsmb/clifsinfo.o libsmb/clidfs.o \
             libsmb/smberr.o libsmb/credentials.o libsmb/pwd_cache.o \
	     libsmb/clioplock.o libsmb/clirap2.o \
	     libsmb/smb_seal.o libsmb/async_smb.o \
	     $(LIBSAMBA_OBJ) \
	     $(LIBNMB_OBJ) \
	     $(LIBNBT_OBJ) \
	     $(CLDAP_OBJ)

LIBSMB_OBJ = libsmb/clientgen.o libsmb/cliconnect.o libsmb/clifile.o \
	     libsmb/clispnego.o libsmb/asn1.o \
	     libsmb/clirap.o libsmb/clierror.o libsmb/climessage.o \
	     libsmb/clireadwrite.o libsmb/clilist.o \
	     libsmb/clitrans.o \
	     libsmb/clistr.o \
             libsmb/smberr.o \
	     libsmb/clioplock.o \
	     libsmb/async_smb.o \
             libsmb/nmblib.o libsmb/namecache.o \
	     $(LIBSAMBA_OBJ) \
	     $(LIBNMB_OBJ)

RPC_CLIENT_OBJ1 = #rpc_client/cli_netlogon.o

#LIBMSRPC_OBJ = rpc_client/cli_lsarpc.o rpc_client/cli_samr.o \
	       $(RPC_CLIENT_OBJ1) rpc_client/cli_reg.o $(RPC_CLIENT_OBJ) \
	       rpc_client/cli_spoolss.o rpc_client/cli_spoolss_notify.o  \
	       rpc_client/cli_svcctl.o \
	       rpc_client/init_samr.o

LIBMSRPC_OBJ = $(RPC_CLIENT_OBJ)

#LIBMSRPC_GEN_OBJ = librpc/gen_ndr/cli_lsa.o \
		   librpc/gen_ndr/cli_dfs.o \
		   librpc/gen_ndr/cli_echo.o \
		   librpc/gen_ndr/cli_srvsvc.o \
		   librpc/gen_ndr/cli_svcctl.o \
		   librpc/gen_ndr/cli_winreg.o \
		   librpc/gen_ndr/cli_initshutdown.o \
		   librpc/gen_ndr/cli_eventlog.o \
		   librpc/gen_ndr/cli_wkssvc.o \
		   librpc/gen_ndr/cli_netlogon.o \
		   librpc/gen_ndr/cli_samr.o \
		   librpc/gen_ndr/cli_dssetup.o \
		   librpc/gen_ndr/cli_ntsvcs.o \
		   $(LIBNDR_GEN_OBJ) \
		   $(RPCCLIENT_NDR_OBJ)

LIBMSRPC_GEN_OBJ = $(LIBNDR_GEN_OBJ)
		   
#
# registry-related objects
#
UTIL_REG_OBJ = #lib/util_reg.o
UTIL_REG_API_OBJ = #lib/util_reg_api.o

REG_INIT_BASIC_OBJ = #registry/reg_init_basic.o
REG_INIT_SMBCONF_OBJ = #registry/reg_init_smbconf.o
REG_INIT_FULL_OBJ = #registry/reg_init_full.o

REGFIO_OBJ = #registry/regfio.o

REGOBJS_OBJ = #registry/reg_objects.o

REG_BACKENDS_BASE_OBJ = #registry/reg_backend_db.o

REG_BACKENDS_SMBCONF_OBJ = #registry/reg_backend_smbconf.o

REG_BACKENDS_EXTRA_OBJ = #registry/reg_backend_printing.o \
			 registry/reg_backend_shares.o \
			 registry/reg_backend_netlogon_params.o \
			 registry/reg_backend_prod_options.o \
			 registry/reg_backend_tcpip_params.o \
			 registry/reg_backend_hkpt_params.o \
			 registry/reg_backend_current_version.o \
			 registry/reg_backend_perflib.o

REG_BASE_OBJ = #registry/reg_api.o \
	       registry/reg_dispatcher.o \
	       registry/reg_cachehook.o \
	       $(REGFIO_OBJ) \
	       $(REGOBJS_OBJ) \
	       registry/reg_util.o \
	       $(UTIL_REG_API_OBJ) \
	       lib/util_nttoken.o \
	       $(REG_BACKENDS_BASE_OBJ) \
	       $(REG_INIT_BASIC_OBJ)

REG_SMBCONF_OBJ = #$(REG_BASE_OBJ) \
		  $(REG_BACKENDS_SMBCONF_OBJ) \
		  $(REG_INIT_SMBCONF_OBJ)

REG_FULL_OBJ = #$(REG_SMBCONF_OBJ) \
	       $(REG_BACKENDS_EXTRA_OBJ) \
	       $(REG_INIT_FULL_OBJ) \
	       registry/reg_eventlog.o \
	       registry/reg_perfcount.o \
	       registry/reg_util_legacy.o


RPC_LSA_OBJ = #rpc_server/srv_lsa_nt.o librpc/gen_ndr/srv_lsa.o

RPC_NETLOG_OBJ = #rpc_server/srv_netlog_nt.o librpc/gen_ndr/srv_netlogon.o

RPC_SAMR_OBJ = #rpc_server/srv_samr_util.o rpc_server/srv_samr_nt.o \
		librpc/gen_ndr/srv_samr.o

RPC_INITSHUTDOWN_OBJ =  #librpc/gen_ndr/srv_initshutdown.o #rpc_server/srv_initshutdown_nt.o

RPC_REG_OBJ =  #rpc_server/srv_winreg_nt.o librpc/gen_ndr/srv_winreg.o

RPC_DSSETUP_OBJ =  #rpc_server/srv_dssetup_nt.o librpc/gen_ndr/srv_dssetup.o

RPC_SVC_OBJ = rpc_server/srv_srvsvc_nt.o librpc/gen_ndr/srv_srvsvc.o 

RPC_WKS_OBJ =  rpc_server/srv_wkssvc_nt.o #librpc/gen_ndr/srv_wkssvc.o 

RPC_SVCCTL_OBJ =  #rpc_server/srv_svcctl.o rpc_server/srv_svcctl_nt.o \
		  librpc/gen_ndr/srv_svcctl.o \
                  services/svc_rcinit.o services/services_db.o \
                  services/svc_netlogon.o services/svc_winreg.o \
                  services/svc_wins.o

RPC_NTSVCS_OBJ = #rpc_server/srv_ntsvcs.o rpc_server/srv_ntsvcs_nt.o \
		 librpc/gen_ndr/srv_ntsvcs.o

RPC_DFS_OBJ =  #librpc/gen_ndr/srv_dfs.o rpc_server/srv_dfs_nt.o

RPC_SPOOLSS_OBJ = #rpc_server/srv_spoolss.o rpc_server/srv_spoolss_nt.o

RPC_EVENTLOG_OBJ = #rpc_server/srv_eventlog.o rpc_server/srv_eventlog_nt.o \
		   rpc_server/srv_eventlog_lib.o librpc/gen_ndr/srv_eventlog.o

RPC_PIPE_OBJ = rpc_server/srv_pipe.o rpc_server/srv_lsa_hnd.o rpc_server/srv_pipe_hnd.o

RPC_ECHO_OBJ = #rpc_server/srv_echo_nt.o librpc/gen_ndr/srv_echo.o

RPC_SERVER_OBJ =  $(RPC_LSA_OBJ) $(RPC_REG_OBJ) $(RPC_INITSHUTDOWN_OBJ) $(RPC_DSSETUP_OBJ) $(RPC_WKS_OBJ) $(RPC_SVCCTL_OBJ) $(RPC_NTSVCS_OBJ) $(RPC_NETLOG_OBJ) $(RPC_DFS_OBJ) $(RPC_SVC_OBJ) $(RPC_SPOOLSS_OBJ) $(RPC_EVENTLOG_OBJ) $(RPC_SAMR_OBJ) $(RPC_PIPE_OBJ)

RPC_PARSE_OBJ = $(RPC_PARSE_OBJ2) \
                #rpc_parse/parse_spoolss.o \
	        rpc_parse/parse_eventlog.o rpc_parse/parse_buffer.o \
                rpc_parse/parse_ntsvcs.o rpc_parse/parse_svcctl.o

RPC_CLIENT_OBJ = #rpc_client/cli_pipe.o

LOCKING_OBJ = locking/locking.o locking/brlock.o locking/posix.o

PRIVILEGES_BASIC_OBJ = lib/privileges_basic.o

PRIVILEGES_OBJ = lib/privileges.o

PASSDB_GET_SET_OBJ = passdb/pdb_get_set.o

#passdb/pdb_tdb.o \

PASSDB_OBJ = $(PASSDB_GET_SET_OBJ) passdb/passdb.o passdb/pdb_interface.o \
		passdb/util_wellknown.o \
		passdb/util_builtin.o passdb/pdb_compat.o \
		passdb/util_unixsids.o passdb/lookup_sid.o \
	        passdb/pdb_smbpasswd.o \
		lib/account_pol.o $(PRIVILEGES_OBJ) \
		lib/winbind_util.o \
		$(SERVER_MUTEX_OBJ)

DEVEL_HELP_WEIRD_OBJ = modules/weird.o
CP850_OBJ = modules/CP850.o
CP437_OBJ = modules/CP437.o
CHARSET_MACOSXFS_OBJ = modules/charset_macosxfs.o

GROUPDB_OBJ = groupdb/mapping.o groupdb/mapping_tdb.o #groupdb/mapping_ldb.o

PROFILE_OBJ = profile/profile.o
PROFILES_OBJ = utils/profiles.o \
	       $(LIBSAMBA_OBJ) \
	       $(PARAM_OBJ) \
               $(LIB_OBJ) $(LIB_DUMMY_OBJ) \
               $(POPT_LIB_OBJ)

OPLOCK_OBJ = smbd/oplock.o smbd/oplock_irix.o smbd/oplock_linux.o

NOTIFY_OBJ = smbd/notify.o smbd/notify_inotify.o smbd/notify_internal.o

VFS_DEFAULT_OBJ = modules/vfs_default.o
VFS_AUDIT_OBJ = modules/vfs_audit.o
VFS_EXTD_AUDIT_OBJ = modules/vfs_extd_audit.o
VFS_FULL_AUDIT_OBJ = modules/vfs_full_audit.o
VFS_FAKE_PERMS_OBJ = modules/vfs_fake_perms.o
VFS_RECYCLE_OBJ = modules/vfs_recycle.o
VFS_NETATALK_OBJ = modules/vfs_netatalk.o
VFS_DEFAULT_QUOTA_OBJ = modules/vfs_default_quota.o
VFS_READONLY_OBJ = modules/vfs_readonly.o modules/getdate.o
VFS_CAP_OBJ = modules/vfs_cap.o
VFS_EXPAND_MSDFS_OBJ = modules/vfs_expand_msdfs.o
VFS_SHADOW_COPY_OBJ = modules/vfs_shadow_copy.o
VFS_SHADOW_COPY2_OBJ = modules/vfs_shadow_copy2.o
VFS_AFSACL_OBJ = modules/vfs_afsacl.o
VFS_XATTR_TDB_OBJ = modules/vfs_xattr_tdb.o
VFS_POSIXACL_OBJ = modules/vfs_posixacl.o
VFS_AIXACL_OBJ = modules/vfs_aixacl.o modules/vfs_aixacl_util.o
VFS_AIXACL2_OBJ = modules/vfs_aixacl2.o modules/vfs_aixacl_util.o modules/nfs4_acls.o
VFS_SOLARISACL_OBJ = modules/vfs_solarisacl.o
VFS_ZFSACL_OBJ = modules/vfs_zfsacl.o modules/nfs4_acls.o
VFS_HPUXACL_OBJ = modules/vfs_hpuxacl.o
VFS_IRIXACL_OBJ = modules/vfs_irixacl.o
VFS_TRU64ACL_OBJ = modules/vfs_tru64acl.o
VFS_CATIA_OBJ = modules/vfs_catia.o
VFS_STREAMS_XATTR_OBJ = modules/vfs_streams_xattr.o
VFS_STREAMS_DEPOT_OBJ = modules/vfs_streams_depot.o
VFS_CACHEPRIME_OBJ = modules/vfs_cacheprime.o
VFS_PREALLOC_OBJ = modules/vfs_prealloc.o
VFS_COMMIT_OBJ = modules/vfs_commit.o
VFS_GPFS_OBJ = modules/vfs_gpfs.o modules/gpfs.o modules/nfs4_acls.o
VFS_NOTIFY_FAM_OBJ = modules/vfs_notify_fam.o
VFS_READAHEAD_OBJ = modules/vfs_readahead.o
VFS_TSMSM_OBJ = modules/vfs_tsmsm.o
VFS_FILEID_OBJ = modules/vfs_fileid.o
VFS_AIO_FORK_OBJ = modules/vfs_aio_fork.o
VFS_SYNCOPS_OBJ = modules/vfs_syncops.o
VFS_SMB_TRAFFIC_ANALYZER_OBJ = modules/vfs_smb_traffic_analyzer.o

PLAINTEXT_AUTH_OBJ = auth/pampass.o auth/pass_check.o

#SLCACHE_OBJ = libsmb/samlogon_cache.o
SLCACHE_OBJ = #libsmb/samlogon_cache.o

#DCUTIL_OBJ  = libsmb/namequery_dc.o libsmb/trustdom_cache.o libsmb/trusts_util.o libsmb/dsgetdcname.o
DCUTIL_OBJ  = libsmb/namequery_dc.o

AUTH_BUILTIN_OBJ = auth/auth_builtin.o
AUTH_DOMAIN_OBJ = auth/auth_domain.o
AUTH_SAM_OBJ = auth/auth_sam.o
AUTH_SERVER_OBJ = auth/auth_server.o
AUTH_UNIX_OBJ = auth/auth_unix.o
AUTH_WINBIND_OBJ = auth/auth_winbind.o
AUTH_SCRIPT_OBJ = auth/auth_script.o

#AUTH_OBJ = auth/auth.o  $(AUTH_SAM_OBJ) $(AUTH_UNIX_OBJ) $(AUTH_WINBIND_OBJ) $(AUTH_SERVER_OBJ) $(AUTH_DOMAIN_OBJ) $(AUTH_BUILTIN_OBJ) auth/auth_util.o auth/token_util.o \
	   auth/auth_compat.o auth/auth_ntlmssp.o \
	   $(PLAINTEXT_AUTH_OBJ) $(SLCACHE_OBJ) $(DCUTIL_OBJ)

AUTH_OBJ = auth/auth.o $(AUTH_SAM_OBJ) $(AUTH_BUILTIN_OBJ) \
           auth/auth_util.o \
           auth/token_util.o \
	   auth/auth_compat.o \
           auth/auth_ntlmssp.o \
	   $(PLAINTEXT_AUTH_OBJ) \
           $(SLCACHE_OBJ) \
           $(DCUTIL_OBJ)

MANGLE_OBJ = smbd/mangle.o smbd/mangle_hash.o smbd/mangle_hash2.o

SMBD_OBJ_MAIN = smbd/server.o

BUILDOPT_OBJ = smbd/build_options.o

#SMBD_OBJ_SRV = smbd/files.o smbd/chgpasswd.o smbd/connection.o \
	       smbd/utmp.o smbd/session.o smbd/map_username.o \
               smbd/dfree.o smbd/dir.o smbd/password.o smbd/conn.o \
	       smbd/share_access.o smbd/fileio.o \
               smbd/ipc.o smbd/lanman.o smbd/negprot.o \
               smbd/message.o smbd/nttrans.o smbd/pipes.o \
               smbd/reply.o smbd/sesssetup.o smbd/trans2.o smbd/uid.o \
	       smbd/dosmode.o smbd/filename.o smbd/open.o smbd/close.o \
	       smbd/blocking.o smbd/sec_ctx.o smbd/srvstr.o \
	       smbd/vfs.o smbd/statcache.o smbd/seal.o \
               smbd/posix_acls.o lib/sysacls.o \
	       smbd/process.o smbd/service.o smbd/error.o \
	       printing/printfsp.o lib/sysquotas.o lib/sysquotas_linux.o \
	       lib/sysquotas_xfs.o lib/sysquotas_4A.o \
	       smbd/change_trust_pw.o smbd/fake_file.o \
	       smbd/quotas.o smbd/ntquotas.o $(AFS_OBJ) smbd/msdfs.o \
	       $(AFS_SETTOKEN_OBJ) smbd/aio.o smbd/statvfs.o \
	       smbd/dmapi.o \
	       smbd/file_access.o \
	       smbd/dnsregister.o \
	       $(MANGLE_OBJ)  $(VFS_DEFAULT_OBJ)

SMBD_OBJ_SRV = smbd/files.o smbd/chgpasswd.o smbd/connection.o \
	       smbd/utmp.o smbd/session.o smbd/map_username.o \
               smbd/dfree.o smbd/dir.o smbd/password.o smbd/conn.o \
	       smbd/share_access.o smbd/fileio.o \
               smbd/ipc.o smbd/lanman.o smbd/negprot.o \
               smbd/message.o smbd/nttrans.o smbd/pipes.o \
               smbd/reply.o smbd/sesssetup.o smbd/trans2.o smbd/uid.o \
	       smbd/dosmode.o smbd/filename.o smbd/open.o smbd/close.o \
	       smbd/blocking.o smbd/sec_ctx.o smbd/srvstr.o \
	       smbd/vfs.o smbd/statcache.o \
               smbd/posix_acls.o lib/sysacls.o \
	       smbd/process.o smbd/service.o smbd/error.o \
	       smbd/change_trust_pw.o smbd/fake_file.o \
	       smbd/msdfs.o \
	       smbd/aio.o smbd/statvfs.o \
	       smbd/file_access.o \
	       $(MANGLE_OBJ)  $(VFS_DEFAULT_OBJ) $(VFS_READAHEAD_OBJ)

#SMBD_OBJ_BASE = $(PARAM_WITHOUT_REG_OBJ) $(SMBD_OBJ_SRV) $(LIBSMB_OBJ) \
		$(RPC_SERVER_OBJ) $(RPC_PARSE_OBJ) \
		$(LOCKING_OBJ) $(PASSDB_OBJ) $(PRINTING_OBJ) $(PROFILE_OBJ) \
		$(LIB_OBJ) $(PRINTBACKEND_OBJ) $(OPLOCK_OBJ) \
		$(NOTIFY_OBJ) $(GROUPDB_OBJ) $(AUTH_OBJ) \
		$(LIBMSRPC_OBJ) $(LIBMSRPC_GEN_OBJ) \
		$(LIBADS_OBJ) $(KRBCLIENT_OBJ) $(LIBADS_SERVER_OBJ) \
		$(REG_FULL_OBJ) $(POPT_LIB_OBJ) $(BUILDOPT_OBJ) \
		$(SMBLDAP_OBJ) $(LDB_OBJ) $(LIBNET_OBJ) bin/libwbclient.a \
		$(LIBSMBCONF_OBJ) \

SMBD_OBJ_BASE = $(PARAM_WITHOUT_REG_OBJ) $(SMBD_OBJ_SRV) $(LIBSMB_OBJ) \
		$(RPC_SERVER_OBJ) $(RPC_PARSE_OBJ) \
		$(LOCKING_OBJ) $(PASSDB_OBJ) $(PROFILE_OBJ) \
		$(LIB_OBJ) $(OPLOCK_OBJ) \
		$(NOTIFY_OBJ) $(GROUPDB_OBJ) $(AUTH_OBJ) \
		$(LIBMSRPC_OBJ) $(LIBMSRPC_GEN_OBJ) \
		libads/ads_status.o \
                lib/util_nttoken.o \
		$(POPT_LIB_OBJ) \
		$(LIBSMBCONF_OBJ) \
		$(PRIVILEGES_BASIC_OBJ)


PRINTING_OBJ = printing/pcap.o printing/print_svid.o printing/print_aix.o \
               printing/print_cups.o printing/print_generic.o \
               printing/lpq_parse.o printing/load.o \
               printing/print_iprint.o

PRINTBASE_OBJ = printing/notify.o printing/printing_db.o
PRINTBACKEND_OBJ = printing/printing.o printing/nt_printing.o $(PRINTBASE_OBJ)

SMBD_OBJ = $(SMBD_OBJ_BASE) $(SMBD_OBJ_MAIN)

NMBD_OBJ1 = nmbd/asyncdns.o nmbd/nmbd.o nmbd/nmbd_become_dmb.o \
            nmbd/nmbd_become_lmb.o nmbd/nmbd_browserdb.o \
            nmbd/nmbd_browsesync.o nmbd/nmbd_elections.o \
            nmbd/nmbd_incomingdgrams.o nmbd/nmbd_incomingrequests.o \
            nmbd/nmbd_lmhosts.o nmbd/nmbd_logonnames.o nmbd/nmbd_mynames.o \
            nmbd/nmbd_namelistdb.o nmbd/nmbd_namequery.o \
            nmbd/nmbd_nameregister.o nmbd/nmbd_namerelease.o \
            nmbd/nmbd_nodestatus.o nmbd/nmbd_packets.o \
            nmbd/nmbd_processlogon.o nmbd/nmbd_responserecordsdb.o \
            nmbd/nmbd_sendannounce.o nmbd/nmbd_serverlistdb.o \
            nmbd/nmbd_subnetdb.o nmbd/nmbd_winsproxy.o nmbd/nmbd_winsserver.o \
            nmbd/nmbd_workgroupdb.o nmbd/nmbd_synclists.o smbd/connection.o

NMBD_OBJ = $(NMBD_OBJ1) $(PARAM_OBJ) $(LIBSMB_OBJ) \
           $(PROFILE_OBJ) $(LIB_NONSMBD_OBJ) $(POPT_LIB_OBJ) \
	   $(LIBNDR_GEN_OBJ0) \
	   libads/ads_status.o

PASSWD_UTIL_OBJ = utils/passwd_util.o

DISPLAY_SEC_OBJ= lib/display_sec.o

RPCCLIENT_OBJ1 = #rpcclient/rpcclient.o rpcclient/cmd_lsarpc.o \
	         rpcclient/cmd_samr.o rpcclient/cmd_spoolss.o \
		 rpcclient/cmd_netlogon.o rpcclient/cmd_srvsvc.o \
		 rpcclient/cmd_dfs.o \
		 rpcclient/cmd_dssetup.o rpcclient/cmd_echo.o \
		 rpcclient/cmd_shutdown.o rpcclient/cmd_test.o \
		 rpcclient/cmd_wkssvc.o rpcclient/cmd_ntsvcs.o \
		 $(DISPLAY_SEC_OBJ)

RPCCLIENT_OBJ = #$(RPCCLIENT_OBJ1) \
             $(PARAM_OBJ) $(LIBSMB_OBJ) $(LIB_NONSMBD_OBJ) \
             $(RPC_PARSE_OBJ) $(PASSDB_OBJ) bin/libwbclient.a $(LIBMSRPC_GEN_OBJ) $(LIBMSRPC_OBJ) \
             $(READLINE_OBJ) $(GROUPDB_OBJ) $(KRBCLIENT_OBJ) \
	     $(LIBADS_OBJ) $(POPT_LIB_OBJ) \
	     $(SMBLDAP_OBJ) $(DCUTIL_OBJ) $(LDB_OBJ) 

LIBSMBCLIENT_OBJ0 = #\
		    libsmb/libsmb_cache.o \
		    libsmb/libsmb_compat.o \
		    libsmb/libsmb_context.o \
		    libsmb/libsmb_dir.o \
		    libsmb/libsmb_file.o \
		    libsmb/libsmb_misc.o \
		    libsmb/libsmb_path.o \
		    libsmb/libsmb_printjob.o \
		    libsmb/libsmb_server.o \
		    libsmb/libsmb_stat.o \
		    libsmb/libsmb_xattr.o \
		    libsmb/libsmb_setget.o

LIBSMBCLIENT_OBJ1 = #$(LIBSMBCLIENT_OBJ0) \
		    $(PARAM_OBJ) $(LIB_NONSMBD_OBJ) \
		    $(LIBSMB_OBJ) $(KRBCLIENT_OBJ) \
		    $(LIBMSRPC_OBJ) $(LIBMSRPC_GEN_OBJ) $(RPC_PARSE_OBJ) \
		    $(PASSDB_OBJ) $(SMBLDAP_OBJ) $(GROUPDB_OBJ) $(LDB_OBJ)

LIBSMBCLIENT_OBJ = #$(LIBSMBCLIENT_OBJ1) bin/libwbclient.a

# This shared library is intended for linking with unit test programs
# to test Samba internals.  It's called libbigballofmud.so to
# discourage casual usage.

LIBBIGBALLOFMUD_MAJOR = 0

LIBBIGBALLOFMUD_OBJ = $(PARAM_OBJ) $(LIB_NONSMBD_OBJ) \
	$(LIBSMB_OBJ) $(LIBMSRPC_OBJ) $(LIBMSRPC_GEN_OBJ) $(RPC_PARSE_OBJ) $(PASSDB_OBJ) bin/libwbclient.a \
	$(GROUPDB_OBJ) $(KRBCLIENT_OBJ) $(SMBLDAP_OBJ) $(LDB_OBJ)

CLIENT_OBJ1 = client/client.o client/clitar.o rpc_client/cli_pipe.o \
	      client/dnsbrowse.o \
	      $(RPC_CLIENT_OBJ1) \
	      $(RPC_PARSE_OBJ2)

CLIENT_OBJ = $(CLIENT_OBJ1) $(PARAM_OBJ) $(LIBSMB_OBJ) \
	     $(LIB_NONSMBD_OBJ) $(KRBCLIENT_OBJ) $(LIBMSRPC_GEN_OBJ) \
             $(READLINE_OBJ) $(POPT_LIB_OBJ) \
             $(PASSDB_OBJ) bin/libwbclient.a $(SMBLDAP_OBJ) $(GROUPDB_OBJ) $(LDB_OBJ) \
	     $(DISPLAY_SEC_OBJ) 

#LIBSMBCONF_OBJ = lib/smbconf/smbconf.o lib/smbconf/smbconf_util.o \
		 lib/smbconf/smbconf_reg.o \
		 lib/smbconf/smbconf_txt.o \
		 lib/smbconf/smbconf_init.o

LIBSMBCONF_OBJ = lib/smbconf/smbconf.o \
                 lib/smbconf/smbconf_util.o \
		 lib/smbconf/smbconf_txt.o \
		 lib/smbconf/smbconf_init.o

LIBNET_OBJ = #libnet/libnet_join.o \
	     libnet/libnet_samsync.o \
	     librpc/gen_ndr/ndr_libnet_join.o

NET_OBJ1 = #utils/net.o utils/net_ads.o utils/net_help.o \
	   utils/net_rap.o utils/net_rpc.o utils/net_rpc_samsync.o \
	   utils/net_rpc_join.o utils/net_time.o utils/net_lookup.o \
	   utils/net_cache.o utils/net_groupmap.o utils/net_idmap.o \
	   utils/net_status.o utils/net_rpc_printer.o utils/net_rpc_rights.o \
	   utils/net_rpc_service.o utils/net_rpc_registry.o utils/net_usershare.o \
	   utils/netlookup.o utils/net_sam.o utils/net_rpc_shell.o \
	   utils/net_util.o utils/net_rpc_sh_acct.o utils/net_rpc_audit.o \
	   $(PASSWD_UTIL_OBJ) utils/net_dns.o utils/net_ads_gpo.o \
	   utils/net_conf.o \
	   utils/net_registry.o \
	   auth/token_util.o utils/net_dom.o

# these are not processed by make proto
NET_OBJ2 = #utils/net_registry_util.o

NET_OBJ = #$(NET_OBJ1) \
	  $(NET_OBJ2) \
	  $(PARAM_WITHOUT_REG_OBJ) $(LIBSMB_OBJ) \
	  $(RPC_PARSE_OBJ) $(PASSDB_OBJ) $(GROUPDB_OBJ) \
	  $(KRBCLIENT_OBJ) $(LIB_NONSMBD_OBJ) $(LIBADDNS_OBJ0) \
	  $(LIBMSRPC_OBJ) $(LIBMSRPC_GEN_OBJ) \
	  $(LIBADS_OBJ) $(LIBADS_SERVER_OBJ) $(POPT_LIB_OBJ) \
	  $(SMBLDAP_OBJ) $(DCUTIL_OBJ) \
	  $(AFS_OBJ) $(AFS_SETTOKEN_OBJ) $(READLINE_OBJ) \
	  $(LDB_OBJ) $(LIBGPO_OBJ) $(INIPARSER_OBJ) $(DISPLAY_SEC_OBJ) \
	  $(REG_SMBCONF_OBJ) $(LIBNETAPI_OBJ0) $(LIBNET_OBJ) \
	  $(LIBSMBCONF_OBJ) \
	  bin/libwbclient.a \
	  $(PRIVILEGES_BASIC_OBJ)

SMBICONV_OBJ = $(PARAM_OBJ) torture/smbiconv.o $(LIB_NONSMBD_OBJ) $(POPT_LIB_OBJ) $(LIBSAMBA_OBJ)

LOG2PCAP_OBJ = utils/log2pcaphex.o

LOCKTEST2_OBJ = torture/locktest2.o $(PARAM_OBJ) $(LOCKING_OBJ) $(LIBSMB_OBJ) \
		$(KRBCLIENT_OBJ) $(LIB_NONSMBD_OBJ) \
		$(LIBNDR_GEN_OBJ0)

SMBCACLS_OBJ = utils/smbcacls.o $(PARAM_OBJ) $(LIBSMB_OBJ) \
		$(KRBCLIENT_OBJ) $(LIB_NONSMBD_OBJ) $(RPC_PARSE_OBJ) \
		$(PASSDB_OBJ) bin/libwbclient.a $(GROUPDB_OBJ) $(LIBMSRPC_OBJ) $(LIBMSRPC_GEN_OBJ) \
		$(POPT_LIB_OBJ) $(DCUTIL_OBJ) $(LIBADS_OBJ) $(SMBLDAP_OBJ) $(LDB_OBJ)

SHARESEC_OBJ0 = utils/sharesec.o
SHARESEC_OBJ  = $(SHARESEC_OBJ0) $(PARAM_OBJ) $(LIB_NONSMBD_OBJ) \
		$(LIBSAMBA_OBJ) \
                $(POPT_LIB_OBJ)

TALLOCTORT_OBJ = lib/talloc/testsuite.o $(PARAM_OBJ) $(LIB_NONSMBD_OBJ) \
                 $(LIBSAMBA_OBJ)

REPLACETORT_OBJ = lib/replace/test/testsuite.o \
		lib/replace/test/getifaddrs.o \
		lib/replace/test/os2_delete.o \
		lib/replace/test/strptime.o \
		$(LIBREPLACE_OBJ)

NDRDUMP_OBJ = #librpc/tools/ndrdump.o \
	      $(PARAM_OBJ) $(LIBNDR_GEN_OBJ) \
	      $(LIBSAMBA_OBJ) $(LIB_NONSMBD_OBJ) $(POPT_LIB_OBJ) \
	      librpc/gen_ndr/ndr_svcctl.o

SMBFILTER_OBJ = utils/smbfilter.o $(PARAM_OBJ) $(LIBSMB_OBJ) \
                 $(LIB_NONSMBD_OBJ) $(KRBCLIENT_OBJ) \
		 $(LIBNDR_GEN_OBJ0)

PROTO_OBJ = $(SMBD_OBJ_MAIN) $(LIBNDR_OBJ) $(LIBNDR_GEN_OBJ) \
	    $(SMBD_OBJ_SRV) $(NMBD_OBJ1) $(LIBSMB_OBJ) \
	    $(SMBTORTURE_OBJ1) $(RPCCLIENT_OBJ1) \
	    $(LIBMSRPC_OBJ) \
	    $(LIB_WITH_PROTO_OBJ) \
	    $(RPC_PIPE_OBJ) $(RPC_PARSE_OBJ) $(KRBCLIENT_OBJ) \
	    $(AUTH_OBJ) $(PARAM_WITHOUT_REG_OBJ) $(LOCKING_OBJ) \
	    $(PRINTING_OBJ) $(PRINTBACKEND_OBJ) $(OPLOCK_OBJ) $(NOTIFY_OBJ) \
	    $(PASSDB_OBJ) $(GROUPDB_OBJ) \
	    $(READLINE_OBJ) $(PROFILE_OBJ) $(LIBADS_OBJ) $(LIBADS_SERVER_OBJ) \
	    $(AUTH_SAM_OBJ) $(REG_FULL_OBJ) $(POPT_LIB_OBJ) \
	    $(RPC_LSA_OBJ) $(RPC_NETLOG_OBJ) $(RPC_SAMR_OBJ) $(RPC_REG_OBJ) $(RPC_DSSETUP_OBJ) \
	    $(RPC_SVC_OBJ) $(RPC_WKS_OBJ) $(RPC_DFS_OBJ) $(RPC_SPOOLSS_OBJ) \
	    $(RPC_ECHO_OBJ) $(RPC_SVCCTL_OBJ) $(RPC_EVENTLOG_OBJ) $(SMBLDAP_OBJ) \
            $(IDMAP_OBJ) libsmb/spnego.o $(PASSCHANGE_OBJ) \
	    $(RPC_NTSVCS_OBJ) $(RPC_INITSHUTDOWN_OBJ) \
	    utils/passwd_util.o $(LIBGPO_OBJ) $(NSS_INFO_OBJ) \
	    $(RPCCLIENT_NDR_OBJ) \
	    $(PRIVILEGES_BASIC_OBJ) \
            lib/util_nttoken.o \
            libsmb/clidfs.o

WINBIND_WINS_NSS_OBJ = nsswitch/wins.o $(PARAM_OBJ) \
	$(LIBSMB_OBJ) $(LIB_NONSMBD_OBJ) $(NSSWINS_OBJ) $(KRBCLIENT_OBJ) \
	$(LIBNDR_GEN_OBJ0)

LDB_COMMON_OBJ=#lib/ldb/common/ldb.o lib/ldb/common/ldb_ldif.o \
          lib/ldb/common/ldb_parse.o lib/ldb/common/ldb_msg.o lib/ldb/common/ldb_utf8.o \
          lib/ldb/common/ldb_debug.o lib/ldb/common/ldb_modules.o \
          lib/ldb/common/ldb_dn.o lib/ldb/common/ldb_match.o lib/ldb/common/ldb_attributes.o \
          lib/ldb/common/attrib_handlers.o lib/ldb/common/ldb_controls.o lib/ldb/common/qsort.o

LDB_TDB_OBJ=#lib/ldb/ldb_tdb/ldb_tdb.o \
       lib/ldb/ldb_tdb/ldb_pack.o lib/ldb/ldb_tdb/ldb_search.o lib/ldb/ldb_tdb/ldb_index.o \
       lib/ldb/ldb_tdb/ldb_cache.o lib/ldb/ldb_tdb/ldb_tdb_wrap.o

LDB_MODULES_OBJ=#lib/ldb/modules/operational.o lib/ldb/modules/rdn_name.o \
          lib/ldb/modules/objectclass.o \
          lib/ldb/modules/paged_results.o lib/ldb/modules/sort.o lib/ldb/modules/asq.o

# enabled in configure.in
#LDB_LDAP_OBJ=
#
#LDB_OBJ = #${LDB_COMMON_OBJ} ${LDB_TDB_OBJ} ${LDB_LDAP_OBJ} ${LDB_MODULES_OBJ}
#
#LDB_CMDLINE_OBJ = $(PARAM_OBJ) \
#	  $(POPT_LIB_OBJ) $(LIB_OBJ) $(LIB_DUMMY_OBJ) $(LIBSMB_ERR_OBJ0) \
#	  $(RPC_PARSE_OBJ1) $(SECRETS_OBJ) \
#	  $(LDB_OBJ) lib/ldb/tools/cmdline.o 


WINBIND_KRB5_LOCATOR_OBJ1 = nsswitch/winbind_krb5_locator.o
WINBIND_KRB5_LOCATOR_OBJ = $(WINBIND_KRB5_LOCATOR_OBJ1) $(WBCOMMON_OBJ) $(LIBREPLACE_OBJ)

POPT_OBJ= #popt/findme.o popt/popt.o popt/poptconfig.o \
          popt/popthelp.o popt/poptparse.o

INIPARSER_OBJ = iniparser_build/iniparser.o iniparser_build/dictionary.o \
		iniparser_build/strlib.o

TDBBACKUP_OBJ = lib/tdb/tools/tdbbackup.o $(LIBREPLACE_OBJ) \
	$(LIBTDB_OBJ0) $(SOCKET_WRAPPER_OBJ)

TDBTOOL_OBJ = lib/tdb/tools/tdbtool.o $(LIBTDB_OBJ0) $(LIBREPLACE_OBJ) \
	$(SOCKET_WRAPPER_OBJ)

TDBDUMP_OBJ = lib/tdb/tools/tdbdump.o $(LIBTDB_OBJ0) $(LIBREPLACE_OBJ) \
	$(SOCKET_WRAPPER_OBJ)

TDBTORTURE_OBJ = lib/tdb/tools/tdbtorture.o $(LIBTDB_OBJ0) $(LIBREPLACE_OBJ) \
	$(SOCKET_WRAPPER_OBJ)


NTLM_AUTH_OBJ1 = utils/ntlm_auth.o utils/ntlm_auth_diagnostics.o

NTLM_AUTH_OBJ = ${NTLM_AUTH_OBJ1} $(LIBSAMBA_OBJ) $(POPT_LIB_OBJ) \
		libsmb/asn1.o libsmb/spnego.o libsmb/clikrb5.o libads/kerberos.o \
		$(LIBADS_SERVER_OBJ) \
		$(PASSDB_OBJ) $(GROUPDB_OBJ) \
		$(SMBLDAP_OBJ) $(LIBNMB_OBJ) \
		$(LDB_OBJ) $(WBCOMMON_OBJ) bin/libwbclient.a \
		$(LIBNDR_GEN_OBJ0) $(LIBNDR_GEN_OBJ1)


######################################################################
# now the rules...
######################################################################
all:: SHOWFLAGS libs $(BIN_PROGS) 

modules:: SHOWFLAGS $(MODULES)

#####################################################################
## Perl IDL Compiler
IDL_FILES = lsa.idl dfs.idl echo.idl winreg.idl initshutdown.idl \
	srvsvc.idl svcctl.idl eventlog.idl wkssvc.idl netlogon.idl notify.idl \
	epmapper.idl messaging.idl xattr.idl misc.idl samr.idl security.idl \
	dssetup.idl krb5pac.idl ntsvcs.idl libnetapi.idl

PIDL = "$(srcdir)/pidl/pidl"

idl::
	@IDL_FILES="$(IDL_FILES)" CPP="$(CPP)" PERL="$(PERL)" PIDL="$(PIDL)"\
	 srcdir="$(srcdir)" $(srcdir)/script/build_idl.sh $(PIDL_ARGS)


#####################################################################


everything:: all libtalloc libsmbclient libnetapi debug2html smbfilter talloctort replacetort smbconftort modules torture \
	$(EVERYTHING_PROGS)

.SUFFIXES:
.SUFFIXES: .c .o .lo

SHOWFLAGS::
	@echo "Using FLAGS      = $(FLAGS)"
	@echo "      PICFLAG    = $(PICFLAG)"
	@echo "      LIBS       = $(LIBS)"
	@echo "      LDFLAGS    = $(LDFLAGS)"
	@echo "      DYNEXP     = $(DYNEXP)"

MAKEDIR = || exec false; \
	  if test -d "$$dir"; then :; else \
	  echo mkdir "$$dir"; \
	  mkdir -p "$$dir" >/dev/null 2>&1 || \
	  test -d "$$dir" || \
	  mkdir "$$dir" || \
	  exec false; fi || exec false

# Run a static checker.
CHECK_CC = $(CC_CHECKER) -I. -I$(srcdir) $(FLAGS) $(PICFLAG) -c $< -o $@
# Compile a source file.
COMPILE_CC = $(CC) -I. -I$(srcdir) $(FLAGS) $(PICFLAG) -c $< -o $@
# Compile a source file with the installation paths defined.
COMPILE_CC_PATH = $(CC) -I. -I$(srcdir) $(PATH_FLAGS) $(FLAGS) $(PICFLAG) -c $< -o $@

COMPILE = $(COMPILE_CC)

.c.o:
	@if (: >> $@ || : > $@) >/dev/null 2>&1; then rm -f $@; else \
	 dir=`echo $@ | sed 's,/[^/]*$$,,;s,^$$,.,'` $(MAKEDIR); fi
	@if test -n "$(CC_CHECKER)"; then \
	  echo "Checking  $*.c with '$(CC_CHECKER)'";\
	  $(CHECK_CC); \
	 fi
	@echo Compiling $*.c
	@$(COMPILE_CC_PATH) && exit 0;\
		echo "The following command failed:" 1>&2;\
		echo "$(COMPILE_CC_PATH)" 1>&2;\
		$(COMPILE_CC_PATH) >/dev/null 2>&1
#	-mv `echo $@ | sed 's%^.*/%%g'` $@

PRECOMPILED_HEADER = $(builddir)/include/includes.h.gch

# this adds support for precompiled headers. To use it, install a snapshot
# of gcc-3.4 and run 'make pch' before you do the main build.
pch:: proto_exists
	rm -f $(PRECOMPILED_HEADER)
	$(MAKE) $(PRECOMPILED_HEADER)

$(PRECOMPILED_HEADER): $(srcdir)/include/includes.h
	$(COMPILE)

BINARY_PREREQS = proto_exists bin/.dummy

# These dependencies are only approximately correct: we want to make
# sure Samba's paths are updated if ./configure is re-run.  Really it
# would be nice if "make prefix=/opt/samba all" also rebuilt things,
# but since we also require "make install prefix=/opt/samba" *not* to
# rebuild it's a bit hard.

dynconfig.o: dynconfig.c Makefile
	@echo Compiling $*.c
	@$(COMPILE_CC_PATH) && exit 0;\
		echo "The following command failed:" 1>&2;\
		echo "$(COMPILE_CC_PATH)" 1>&2;\
		$(COMPILE_CC_PATH) >/dev/null 2>&1

lib/pidfile.o: lib/pidfile.c
	@echo Compiling $*.c
	@$(COMPILE_CC_PATH) && exit 0;\
		echo "The following command failed:" 1>&2;\
		echo "$(COMPILE_CC_PATH)" 1>&2;\
		$(COMPILE_CC_PATH) >/dev/null 2>&1

lib/version.o: lib/version.c include/version.h
	@echo Compiling $*.c
	@$(COMPILE_CC_PATH) && exit 0;\
		echo "The following command failed:" 1>&2;\
		echo "$(COMPILE_CC_PATH)" 1>&2;\
		$(COMPILE_CC_PATH) >/dev/null 2>&1

smbd/build_options.o: smbd/build_options.c Makefile include/config.h include/build_env.h include/proto.h
	@echo Compiling $*.c
	@$(COMPILE_CC_PATH) && exit 0;\
		echo "The following command failed:" 1>&2;\
		echo "$(COMPILE_CC_PATH)" 1>&2;\
		$(COMPILE_CC_PATH) >/dev/null 2>&1

smbd/build_options.c: script/mkbuildoptions.awk
	@echo Generating $@

bin/.dummy:
	@if (: >> $@ || : > $@) >/dev/null 2>&1; then :; else \
	  dir=bin $(MAKEDIR); fi
	@: >> $@ || : > $@ # what a fancy emoticon!

bin/libsmbd.a: $(SMBD_OBJ_BASE) $(POPT_OBJ)
	$(AR) cru bin/libsmbd.a $(SMBD_OBJ_BASE) $(POPT_OBJ)
	$(RANLIB) bin/libsmbd.a

#bin/smbd: $(BINARY_PREREQS) bin/libsmbd.a $(SMBD_OBJ_MAIN)
#	@echo Linking $@
#	$(CC) $(FLAGS)  -o $@ $(SMBD_OBJ_MAIN) $(LDFLAGS) \
#		$(LIBS) -Lbin -lsmbd

bin/smbd: $(BINARY_PREREQS) $(SMBD_OBJ) $(POPT_OBJ)
	@echo Linking $@
	@$(CC) $(FLAGS) -o $@ $(SMBD_OBJ) $(LDFLAGS) \
		$(LIBS) $(POPT_LIBS)  

bin/nmbd: $(BINARY_PREREQS) $(NMBD_OBJ) $(POPT_OBJ)  
	@echo Linking $@
	@$(CC) $(FLAGS) -o $@ $(NMBD_OBJ) $(LDFLAGS) $(LIBS) \
		$(LIBTALLOC_LIBS) $(LIBTDB_LIBS) $(POPT_LIBS)

MY_PASS_OBJ = libsmb/smbdes.o lib/md4.o lib/arc4.o lib/replace/snprintf.o
bin/smbpasswd: utils/owrt_smbpasswd.o $(MY_PASS_OBJ)
	@echo Linking $@
	@$(CC) $(FLAGS)  -o $@ utils/owrt_smbpasswd.o $(MY_PASS_OBJ)

#bin/smbpasswd: $(BINARY_PREREQS) $(SMBPASSWD_OBJ) $(POPT_OBJ)   
#	@echo Linking $@
#	@$(CC) $(FLAGS) -o $@ $(SMBPASSWD_OBJ) $(LDFLAGS) $(PASSDB_LIBS) \
#		$(DYNEXP) $(LIBS) $(POPT_LIBS) $(KRB5LIBS) $(LDAP_LIBS) \
#		$(LIBTALLOC_LIBS) $(LIBTDB_LIBS) $(WINBIND_LIBS)

#####################################################################
#
# libraries
#
#####################################################################

MKSYMS_SH = $(srcdir)/script/mksyms.sh


#-------------------------------------------------------------------
#
# libtalloc
#
#-------------------------------------------------------------------

TALLOC_OBJ =  lib/talloc/talloc.o

LIBTALLOC_OBJ0 = $(TALLOC_OBJ)
LIBTALLOC_OBJ = $(LIBTALLOC_OBJ0) $(LIBREPLACE_OBJ)

LIBTALLOC_SHARED_TARGET=bin/libtalloc.so
LIBTALLOC_SOVER=1
LIBTALLOC_SHARED_TARGET_SONAME=$(LIBTALLOC_SHARED_TARGET).$(LIBTALLOC_SOVER)
LIBTALLOC_STATIC_TARGET=bin/libtalloc.a
LIBTALLOC=$(LIBTALLOC_STATIC_TARGET) 
LIBTALLOC_SYMS=$(srcdir)/exports/libtalloc.syms
LIBTALLOC_HEADERS=$(srcdir)/lib/talloc/talloc.h

$(LIBTALLOC_SYMS): $(LIBTALLOC_HEADERS)
	@$(MKSYMS_SH) $(AWK) $@ $(LIBTALLOC_HEADERS)

$(LIBTALLOC_SHARED_TARGET_SONAME): $(BINARY_PREREQS) $(LIBTALLOC_OBJ) $(LIBTALLOC_SYMS)
	@echo Linking shared library $@
	@$(SHLD_DSO) $(LIBTALLOC_OBJ) -Wl,-soname=`basename $@`

$(LIBTALLOC_SHARED_TARGET): $(LIBTALLOC_SHARED_TARGET_SONAME)
	@rm -f $@
	@ln -f -s `basename $(LIBTALLOC_SHARED_TARGET_SONAME)` $@

$(LIBTALLOC_STATIC_TARGET): $(BINARY_PREREQS) $(LIBTALLOC_OBJ0)
	@echo Linking non-shared library $@
	@-$(AR) -rc $@ $(LIBTALLOC_OBJ0)

libtalloc: $(LIBTALLOC)

cleanlibtalloc::
	-rm -f \
		$(LIBTALLOC_SYMS) \
		$(LIBTALLOC_STATIC_TARGET) \
		$(LIBTALLOC_SHARED_TARGET) \
		$(LIBTALLOC_SHARED_TARGET_SONAME)

installlibtalloc:: installdirs libtalloc
	@$(SHELL) $(srcdir)/script/installdirs.sh $(INSTALLPERMS_BIN) $(DESTDIR) $(LIBDIR)
	-$(INSTALLLIBCMD_SH) $(LIBTALLOC_SHARED_TARGET_SONAME) $(DESTDIR)$(LIBDIR)
	@rm -f $(DESTDIR)$(LIBDIR)/`basename $(LIBTALLOC_SHARED_TARGET)`
	-if test -r $(LIBTALLOC_SHARED_TARGET_SONAME) ; then \
		ln -f -s `basename $(LIBTALLOC_SHARED_TARGET_SONAME)` \
			$(DESTDIR)$(LIBDIR)/`basename $(LIBTALLOC_SHARED_TARGET)` ; \
	fi
	-$(INSTALLLIBCMD_A) $(LIBTALLOC_STATIC_TARGET) $(DESTDIR)$(LIBDIR)
	@$(SHELL) $(srcdir)/script/installdirs.sh $(INSTALLPERMS_BIN) $(DESTDIR) ${prefix}/include
	-$(INSTALLCMD) -m $(INSTALLPERMS_DATA) $(LIBTALLOC_HEADERS) $(DESTDIR)${prefix}/include

uninstalllibtalloc::
	-rm -f $(DESTDIR)$(LIBDIR)/`basename $(LIBTALLOC_SHARED_TARGET)`
	-$(UNINSTALLLIBCMD_SH) $(DESTDIR)$(LIBDIR)/`basename $(LIBTALLOC_SHARED_TARGET_SONAME)`
	-$(UNINSTALLLIBCMD_A) $(DESTDIR)$(LIBDIR)/`basename $(LIBTALLOC_STATIC_TARGET)`
	-rm -f $(DESTDIR)${prefix}/include/talloc.h

libs:: libtalloc
installlibs:: 
uninstalllibs:: 
cleanlibs:: cleanlibtalloc
shlibs test_shlibs: 

#-------------------------------------------------------------------
#
# libtdb
#
#-------------------------------------------------------------------

LIBTDB_OBJ0 =  lib/tdb/common/tdb.o lib/tdb/common/dump.o lib/tdb/common/transaction.o lib/tdb/common/error.o lib/tdb/common/traverse.o lib/tdb/common/freelist.o lib/tdb/common/freelistcheck.o lib/tdb/common/io.o lib/tdb/common/lock.o lib/tdb/common/open.o
LIBTDB_OBJ = $(LIBTDB_OBJ0) $(LIBREPLACE_OBJ)

LIBTDB_SHARED_TARGET=bin/libtdb.so
LIBTDB_SOVER=1
LIBTDB_SHARED_TARGET_SONAME=$(LIBTDB_SHARED_TARGET).$(LIBTDB_SOVER)
LIBTDB_STATIC_TARGET=bin/libtdb.a
LIBTDB=$(LIBTDB_STATIC_TARGET) 
LIBTDB_SYMS=$(srcdir)/exports/libtdb.syms
LIBTDB_HEADERS=$(srcdir)/lib/tdb/include/tdb.h

$(LIBTDB_SYMS): $(LIBTDB_HEADERS)
	@$(MKSYMS_SH) $(AWK) $@ $(LIBTDB_HEADERS)

$(LIBTDB_SHARED_TARGET_SONAME): $(BINARY_PREREQS) $(LIBTDB_OBJ) $(LIBTDB_SYMS)
	@echo Linking shared library $@
	@$(SHLD_DSO) $(LIBTDB_OBJ) \
		-Wl,-soname=`basename $@`

$(LIBTDB_SHARED_TARGET): $(LIBTDB_SHARED_TARGET_SONAME)
	@rm -f $@
	@ln -f -s `basename $(LIBTDB_SHARED_TARGET_SONAME)` $@

$(LIBTDB_STATIC_TARGET): $(BINARY_PREREQS) $(LIBTDB_OBJ0)
	@echo Linking non-shared library $@
	@-$(AR) -rc $@ $(LIBTDB_OBJ0)

libtdb: $(LIBTDB)

cleanlibtdb::
	-rm -f \
		$(LIBTDB_SYMS) \
		$(LIBTDB_STATIC_TARGET) \
		$(LIBTDB_SHARED_TARGET) \
		$(LIBTDB_SHARED_TARGET_SONAME)


installlibtdb:: installdirs libtdb
	@$(SHELL) $(srcdir)/script/installdirs.sh $(INSTALLPERMS_BIN) $(DESTDIR) $(LIBDIR)
	-$(INSTALLLIBCMD_SH) $(LIBTDB_SHARED_TARGET_SONAME) $(DESTDIR)$(LIBDIR)
	@rm -f $(DESTDIR)$(LIBDIR)/`basename $(LIBTDB_SHARED_TARGET)`
	-if test -r $(LIBTDB_SHARED_TARGET_SONAME) ; then \
		ln -f -s `basename $(LIBTDB_SHARED_TARGET_SONAME)` \
			$(DESTDIR)$(LIBDIR)/`basename $(LIBTDB_SHARED_TARGET)` ; \
	fi
	-$(INSTALLLIBCMD_A) $(LIBTDB_STATIC_TARGET) $(DESTDIR)$(LIBDIR)
	@$(SHELL) $(srcdir)/script/installdirs.sh $(INSTALLPERMS_BIN) $(DESTDIR) ${prefix}/include
	-$(INSTALLCMD) -m $(INSTALLPERMS_DATA) $(LIBTDB_HEADERS) $(DESTDIR)${prefix}/include

uninstalllibtdb::
	-rm -f $(DESTDIR)$(LIBDIR)/`basename $(LIBTDB_SHARED_TARGET)`
	-$(UNINSTALLLIBCMD_SH) $(DESTDIR)$(LIBDIR)/`basename $(LIBTDB_SHARED_TARGET_SONAME)`
	-$(UNINSTALLLIBCMD_A) $(DESTDIR)$(LIBDIR)/`basename $(LIBTDB_STATIC_TARGET)`
	-rm -f $(DESTDIR)${prefix}/include/tdb.h

libs:: libtdb
installlibs:: 
uninstalllibs:: 
cleanlibs:: cleanlibtdb
shlibs test_shlibs: 

#-------------------------------------------------------------------
#
# libwbclient
#
#-------------------------------------------------------------------

#LIBWBCLIENT_OBJ0 = nsswitch/libwbclient/wbclient.o \
		  nsswitch/libwbclient/wbc_util.o \
		  nsswitch/libwbclient/wbc_pwd.o \
		  nsswitch/libwbclient/wbc_idmap.o \
		  nsswitch/libwbclient/wbc_sid.o \
		  nsswitch/libwbclient/wbc_pam.o

LIBWBCLIENT_OBJ0 = nsswitch/libwbclient/wbclient.o

LIBWBCLIENT_OBJ = $(LIBWBCLIENT_OBJ0) \
		  $(WBCOMMON_OBJ) \
		  $(LIBTALLOC_OBJ0) $(LIBREPLACE_OBJ)

LIBWBCLIENT_SHARED_TARGET=bin/libwbclient.so
LIBWBCLIENT_SOVER=0
LIBWBCLIENT_SHARED_TARGET_SONAME=$(LIBWBCLIENT_SHARED_TARGET).$(LIBWBCLIENT_SOVER)
LIBWBCLIENT_STATIC_TARGET=bin/libwbclient.a
LIBWBCLIENT=bin/libwbclient.a 
LIBWBCLIENT_SYMS=$(srcdir)/exports/libwbclient.syms
LIBWBCLIENT_HEADERS=$(srcdir)/nsswitch/libwbclient/wbclient.h

$(LIBWBCLIENT_SYMS): $(LIBWBCLIENT_HEADERS)
	@$(MKSYMS_SH) $(AWK) $@ $(LIBWBCLIENT_HEADERS)

$(LIBWBCLIENT_SHARED_TARGET_SONAME): $(BINARY_PREREQS) $(LIBWBCLIENT_OBJ) $(LIBWBCLIENT_SYMS) 
	@echo Linking shared library $@
	@$(SHLD_DSO) $(LIBTALLOC_LIBS) $(LIBWBCLIENT_OBJ) \
		-Wl,-soname=`basename $@`

$(LIBWBCLIENT_SHARED_TARGET): $(LIBWBCLIENT_SHARED_TARGET_SONAME)
	@rm -f $@
	@ln -f -s `basename $(LIBWBCLIENT_SHARED_TARGET_SONAME)` $@

$(LIBWBCLIENT_STATIC_TARGET): $(BINARY_PREREQS) $(LIBWBCLIENT_OBJ0) $(WBCOMMON_OBJ)
	@echo Linking non-shared library $@
	@-$(AR) -rc $@ $(LIBWBCLIENT_OBJ0) $(WBCOMMON_OBJ)

libwbclient: $(LIBWBCLIENT)

cleanlibwbclient::
	-rm -f \
		$(LIBWBCLIENT_SYMS) \
		$(LIBWBCLIENT_STATIC_TARGET) \
		$(LIBWBCLIENT_SHARED_TARGET) \
		$(LIBWBCLIENT_SHARED_TARGET_SONAME)

installlibwbclient:: installdirs libwbclient
	@$(SHELL) $(srcdir)/script/installdirs.sh $(INSTALLPERMS_BIN) $(DESTDIR) $(LIBDIR)
	-$(INSTALLLIBCMD_SH) $(LIBWBCLIENT_SHARED_TARGET_SONAME) $(DESTDIR)$(LIBDIR)
	@rm -f $(DESTDIR)$(LIBDIR)/`basename $(LIBWBCLIENT_SHARED_TARGET)`
	-if test -r $(LIBWBCLIENT_SHARED_TARGET_SONAME) ; then \
		ln -f -s `basename $(LIBWBCLIENT_SHARED_TARGET_SONAME)` \
			$(DESTDIR)$(LIBDIR)/`basename $(LIBWBCLIENT_SHARED_TARGET)` ; \
	fi
	@$(SHELL) $(srcdir)/script/installdirs.sh $(INSTALLPERMS_BIN) $(DESTDIR) ${prefix}/include
	-$(INSTALLCMD) -m $(INSTALLPERMS_DATA) $(LIBWBCLIENT_HEADERS) $(DESTDIR)${prefix}/include

uninstalllibwbclient::
	-rm -f $(DESTDIR)$(LIBDIR)/`basename $(LIBWBCLIENT_SHARED_TARGET)`
	-$(UNINSTALLLIBCMD_SH) $(DESTDIR)$(LIBDIR)/`basename $(LIBWBCLIENT_SHARED_TARGET_SONAME)`
	-rm -f $(DESTDIR)${prefix}/include/wbclient.h

#libs:: libwbclient
#installlibs:: 
#uninstalllibs:: uninstalllibwbclient
#cleanlibs:: cleanlibwbclient
#shlibs test_shlibs: 


#-------------------------------------------------------------------
#
# libaddns
#
#-------------------------------------------------------------------


LIBADDNS_SHARED_TARGET=bin/libaddns.so
LIBADDNS_SOVER=0
LIBADDNS_SHARED_TARGET_SONAME=$(LIBADDNS_SHARED_TARGET).$(LIBADDNS_SOVER)
LIBADDNS_STATIC_TARGET=bin/libaddns.a
LIBADDNS=$(LIBADDNS_STATIC_TARGET) 
#LIBADDNS_SYMS=$(srcdir)/exports/libaddns.syms
LIBADDNS_HEADERS=$(srcdir)/libaddns/addns.h

$(LIBADDNS_SHARED_TARGET_SONAME): $(BINARY_PREREQS) $(LIBADDNS_OBJ) 
	@echo Linking shared library $@
	@$(SHLD_DSO) $(LIBADDNS_OBJ) $(LIBS) \
		$(KRB5LIBS) $(UUID_LIBS) $(LIBTALLOC_LIBS) \
		-Wl,-soname=`basename $@`

$(LIBADDNS_SHARED_TARGET): $(LIBADDNS_SHARED_TARGET_SONAME)
	@rm -f $@
	@ln -f -s `basename $(LIBADDNS_SHARED_TARGET_SONAME)` $@

$(LIBADDNS_STATIC_TARGET): $(BINARY_PREREQS) $(LIBADDNS_OBJ0)
	@echo Linking non-shared library $@
	@-$(AR) -rc $@ $(LIBADDNS_OBJ0)

libaddns: $(LIBADDNS)

cleanlibaddns::
	-rm -f \
		$(LIBADDNS_SHARED_TARGET) \
		$(LIBADDNS_SHARED_TARGET_SONAME) \
		$(LIBADDNS_STATIC_TARGET)

installlibaddns:: installdirs libaddns
	@$(SHELL) $(srcdir)/script/installdirs.sh $(INSTALLPERMS_BIN) $(DESTDIR) $(LIBDIR)
	-$(INSTALLLIBCMD_SH) $(LIBADDNS_SHARED_TARGET_SONAME) $(DESTDIR)$(LIBDIR)
	@rm -f $(DESTDIR)$(LIBDIR)/`basename $(LIBADDNS_SHARED_TARGET)`
	-if test -r $(LIBADDNS_SHARED_TARGET_SONAME) ; then \
		ln -f -s `basename $(LIBADDNS_SHARED_TARGET_SONAME)` \
			$(DESTDIR)$(LIBDIR)/`basename $(LIBADDNS_SHARED_TARGET)` ; \
	fi
	-$(INSTALLLIBCMD_A) $(LIBADDNS_STATIC_TARGET) $(DESTDIR)$(LIBDIR)

uninstalllibaddns::
	-$(UNINSTALLLIBCMD_SH) $(DESTDIR)$(LIBDIR)/`basename $(LIBADDNS_SHARED_TARGET_SONAME)`
	-rm -f $(DESTDIR)$(LIBDIR)/`basename $(LIBADDNS_SHARED_TARGET)`
	-$(UNINSTALLLIBCMD_A) $(DESTDIR)$(LIBDIR)/`basename $(LIBADDNS_STATIC_TARGET)`

#libs:: libaddns
#installlibs:: 
#uninstalllibs:: 
#cleanlibs:: cleanlibaddns
#shlibs test_shlibs: 

#-------------------------------------------------------------------
#
# libnetapi
#
#-------------------------------------------------------------------

LIBNETAPI_OBJ0 = #lib/netapi/netapi.o \
		 lib/netapi/cm.o \
		 librpc/gen_ndr/ndr_libnetapi.o \
		 lib/netapi/libnetapi.o \
		 lib/netapi/joindomain.o \
		 lib/netapi/serverinfo.o \
		 lib/netapi/getdc.o \
		 lib/netapi/user.o

LIBNETAPI_OBJ  = #$(LIBNETAPI_OBJ0) $(LIBNET_OBJ) \
		 $(LIBSMBCONF_OBJ) \
		 $(REG_SMBCONF_OBJ) \
		 $(PARAM_WITHOUT_REG_OBJ) $(LIB_NONSMBD_OBJ) \
		 $(LIBSMB_OBJ) $(KRBCLIENT_OBJ) \
		 $(LIBMSRPC_OBJ) $(LIBMSRPC_GEN_OBJ) $(RPC_PARSE_OBJ) \
		 $(PASSDB_OBJ) bin/libwbclient.a $(SMBLDAP_OBJ) $(GROUPDB_OBJ) $(LDB_OBJ) \
		 $(DCUTIL_OBJ) $(LIBADS_OBJ) $(PRIVILEGES_BASIC_OBJ)

LIBNETAPI_SHARED_TARGET=bin/libnetapi.so
LIBNETAPI_SOVER=0
LIBNETAPI_SHARED_TARGET_SONAME=$(LIBNETAPI_SHARED_TARGET).$(LIBNETAPI_SOVER)
LIBNETAPI_STATIC_TARGET=bin/libnetapi.a
LIBNETAPI=$(LIBNETAPI_STATIC_TARGET) 
LIBNETAPI_SYMS=$(srcdir)/exports/libnetapi.syms
LIBNETAPI_HEADERS=$(srcdir)/lib/netapi/netapi.h

$(LIBNETAPI_SYMS): $(LIBNETAPI_HEADERS)
	@$(MKSYMS_SH) $(AWK) $@ $(LIBNETAPI_HEADERS)

$(LIBNETAPI_SHARED_TARGET_SONAME): $(BINARY_PREREQS) $(LIBNETAPI_OBJ) $(LIBNETAPI_SYMS)   
	@echo Linking shared library $@
	@$(SHLD_DSO) $(LIBNETAPI_OBJ) \
		$(LIBTALLOC_LIBS) $(LIBTDB_LIBS) $(WINBIND_LIBS) $(LIBS) \
		$(LDAP_LIBS) $(KRB5LIBS) $(NSCD_LIBS) \
		-Wl,-soname=`basename $@`

$(LIBNETAPI_SHARED_TARGET): $(LIBNETAPI_SHARED_TARGET_SONAME)
	@rm -f $@
	@ln -f -s `basename $(LIBNETAPI_SHARED_TARGET_SONAME)` $@

$(LIBNETAPI_STATIC_TARGET): $(BINARY_PREREQS) $(LIBNETAPI_OBJ0)
	@echo Linking non-shared library $@
	@-$(AR) -rc $@ $(LIBNETAPI_OBJ0)

libnetapi: $(LIBNETAPI)

cleanlibnetapi::
	-rm -f \
		$(LIBNETAPI_SYMS) \
		$(LIBNETAPI_STATIC_TARGET) \
		$(LIBNETAPI_SHARED_TARGET) \
		$(LIBNETAPI_SHARED_TARGET_SONAME)

installlibnetapi:: installdirs libnetapi
	@$(SHELL) $(srcdir)/script/installdirs.sh $(INSTALLPERMS_BIN) $(DESTDIR) $(LIBDIR)
	-$(INSTALLLIBCMD_SH) $(LIBNETAPI_SHARED_TARGET_SONAME) $(DESTDIR)$(LIBDIR)
	@rm -f $(DESTDIR)$(LIBDIR)/`basename $(LIBNETAPI_SHARED_TARGET)`
	-if test -r $(LIBNETAPI_SHARED_TARGET_SONAME) ; then \
		ln -f -s `basename $(LIBNETAPI_SHARED_TARGET_SONAME)` \
			$(DESTDIR)$(LIBDIR)/`basename $(LIBNETAPI_SHARED_TARGET)` ; \
	fi
	-$(INSTALLLIBCMD_A) $(LIBNETAPI_STATIC_TARGET) $(DESTDIR)$(LIBDIR)
	@$(SHELL) $(srcdir)/script/installdirs.sh $(INSTALLPERMS_BIN) $(DESTDIR) ${prefix}/include
	-$(INSTALLCMD) -m $(INSTALLPERMS_DATA) $(srcdir)/lib/netapi/netapi.h $(DESTDIR)${prefix}/include

uninstalllibnetapi::
	-rm -f $(DESTDIR)$(LIBDIR)/`basename $(LIBNETAPI_SHARED_TARGET)`
	-$(UNINSTALLLIBCMD_SH) $(DESTDIR)$(LIBDIR)/`basename $(LIBNETAPI_SHARED_TARGET_SONAME)`
	-$(UNINSTALLLIBCMD_A) $(DESTDIR)$(LIBDIR)/`basename $(LIBNETAPI_STATIC_TARGET)`
	-rm -f $(DESTDIR)${prefix}/include/netapi.h

#libs:: libnetapi
#installlibs:: 
#uninstalllibs:: 
#cleanlibs:: cleanlibnetapi
#shlibs test_shlibs: 

#-------------------------------------------------------------------
#
# libsmbclient
#
#-------------------------------------------------------------------

LIBSMBCLIENT_SHARED_TARGET=bin/libsmbclient.so
LIBSMBCLIENT_SOVER=0
LIBSMBCLIENT_SHARED_TARGET_SONAME=$(LIBSMBCLIENT_SHARED_TARGET).$(LIBSMBCLIENT_SOVER)
LIBSMBCLIENT_STATIC_TARGET=bin/libsmbclient.a
LIBSMBCLIENT=$(LIBSMBCLIENT_STATIC_TARGET) 
#LIBSMBCLIENT_SYMS=$(srcdir)/exports/libsmbclient.syms
LIBSMBCLIENT_HEADERS=$(srcdir)/include/libsmbclient.h

$(LIBSMBCLIENT_SHARED_TARGET_SONAME): $(BINARY_PREREQS) $(LIBSMBCLIENT_OBJ)   
	@echo Linking shared library $@
	@$(SHLD_DSO) $(LIBSMBCLIENT_OBJ) \
		$(LIBTALLOC_LIBS) $(LIBTDB_LIBS) $(WINBIND_LIBS) $(LIBS) \
		$(KRB5LIBS) $(LDAP_LIBS) $(NSCD_LIBS) \
		-Wl,-soname=`basename $@`

$(LIBSMBCLIENT_SHARED_TARGET): $(LIBSMBCLIENT_SHARED_TARGET_SONAME)
	@rm -f $@
	@ln -s `basename $(LIBSMBCLIENT_SHARED_TARGET_SONAME)` $@

$(LIBSMBCLIENT_STATIC_TARGET): $(BINARY_PREREQS) $(LIBSMBCLIENT_OBJ1)
	@echo Linking non-shared library $@
	@-$(AR) -rc $@ $(LIBSMBCLIENT_OBJ1)

libsmbclient: $(LIBSMBCLIENT)

cleanlibsmbclient::
	-rm -f \
		$(LIBSMBCLIENT_SHARED_TARGET) \
		$(LIBSMBCLIENT_SHARED_TARGET_SONAME) \
		$(LIBSMBCLIENT_STATIC_TARGET)

installlibsmbclient:: installdirs libsmbclient
	@$(SHELL) $(srcdir)/script/installdirs.sh $(INSTALLPERMS_BIN) $(DESTDIR) $(LIBDIR)
	-$(INSTALLLIBCMD_SH) $(LIBSMBCLIENT_SHARED_TARGET_SONAME) $(DESTDIR)$(LIBDIR)
	@rm -f $(DESTDIR)$(LIBDIR)/`basename $(LIBSMBCLIENT_SHARED_TARGET)`
	-if test -r $(LIBSMBCLIENT_SHARED_TARGET_SONAME) ; then \
		ln -f -s `basename $(LIBSMBCLIENT_SHARED_TARGET_SONAME)` \
			$(DESTDIR)$(LIBDIR)/`basename $(LIBSMBCLIENT_SHARED_TARGET)` ; \
	fi
	-$(INSTALLLIBCMD_A) $(LIBSMBCLIENT_STATIC_TARGET) $(DESTDIR)$(LIBDIR)
	@$(SHELL) $(srcdir)/script/installdirs.sh $(INSTALLPERMS_BIN) $(DESTDIR) ${prefix}/include
	-$(INSTALLCMD) -m $(INSTALLPERMS_DATA) $(LIBSMBCLIENT_HEADERS) $(DESTDIR)${prefix}/include

installclientlib:: installlibsmbclient

uninstalllibsmbclient::
	-rm -f $(DESTDIR)$(LIBDIR)/`basename $(LIBSMBCLIENT_SHARED_TARGET)`
	-$(UNINSTALLLIBCMD_SH) $(DESTDIR)$(LIBDIR)/`basename $(LIBSMBCLIENT_SHARED_TARGET_SONAME)`
	-$(UNINSTALLLIBCMD_A) $(DESTDIR)$(LIBDIR)/`basename $(LIBSMBCLIENT_STATIC_TARGET)`
	-rm -f $(DESTDIR)${prefix}/include/libsmbclient.h

uninstallclientlib:: uninstalllibsmbclient

#libs:: libsmbclient
#installlibs:: 
#uninstalllibs:: 
#cleanlibs:: cleanlibsmbclient
#shlibs test_shlibs: 

#-------------------------------------------------------------------
#
# libsmbsharemodes
#
#-------------------------------------------------------------------

LIBSMBSHAREMODES_OBJ0 = libsmb/smb_share_modes.o

LIBSMBSHAREMODES_OBJ = $(LIBSMBSHAREMODES_OBJ0) $(LIBTDB_OBJ0)

LIBSMBSHAREMODES_SHARED_TARGET=bin/libsmbsharemodes.so
LIBSMBSHAREMODES_SOVER=0
LIBSMBSHAREMODES_SHARED_TARGET_SONAME=$(LIBSMBSHAREMODES_SHARED_TARGET).$(LIBSMBSHAREMODES_SOVER)
LIBSMBSHAREMODES_STATIC_TARGET=bin/libsmbsharemodes.a
LIBSMBSHAREMODES=$(LIBSMBSHAREMODES_STATIC_TARGET) 
LIBSMBSHAREMODES_SYMS=$(srcdir)/exports/libsmbsharemodes.syms
LIBSMBSHAREMODES_HEADERS=$(srcdir)/include/smb_share_modes.h

LIBSMBSHAREMODES=bin/libsmbsharemodes.a 

$(LIBSMBSHAREMODES_SHARED_TARGET_SONAME): $(BINARY_PREREQS) $(LIBSMBSHAREMODES_OBJ)  
	@echo Linking shared library $@
	@$(SHLD_DSO) $(LIBSMBSHAREMODES_OBJ) \
		$(LIBS) $(LIBTALLOC_LIBS) $(LIBTDB_LIBS) \
		$(KRB5LIBS) $(LDAP_LIBS) \
		-Wl,-soname=`basename $@`

$(LIBSMBSHAREMODES_SHARED_TARGET): $(LIBSMBSHAREMODES_SHARED_TARGET_SONAME)
	@rm -f $@
	@ln -f -s `basename $(LIBSMBSHAREMODES_SHARED_TARGET_SONAME)` $@

$(LIBSMBSHAREMODES_STATIC_TARGET): $(BINARY_PREREQS) $(LIBSMBSHAREMODES_OBJ0)
	@echo Linking non-shared library $@
	@-$(AR) -rc $@ $(LIBSMBSHAREMODES_OBJ0)

libsmbsharemodes: $(LIBSMBSHAREMODES)

cleanlibsmbsharemodes::
	-rm -f \
		$(LIBSMBSHAREMODES_SHARED_TARGET) \
		$(LIBSMBSHAREMODES_SHARED_TARGET_SONAME) \
		$(LIBSMBSHAREMODES_STATIC_TARGET)

installlibsmbsharemodes:: installdirs libsmbsharemodes
	@$(SHELL) $(srcdir)/script/installdirs.sh $(INSTALLPERMS_BIN) $(DESTDIR) $(LIBDIR)
	-$(INSTALLLIBCMD_SH) $(LIBSMBSHAREMODES_SHARED_TARGET_SONAME) $(DESTDIR)$(LIBDIR)
	@rm -f $(DESTDIR)$(LIBDIR)/`basename $(LIBSMBSHAREMODES_SHARED_TARGET)`
	-if test -r $(LIBSMBSHAREMODES_SHARED_TARGET_SONAME) ; then \
		ln -f -s `basename $(LIBSMBSHAREMODES_SHARED_TARGET_SONAME)` \
			$(DESTDIR)$(LIBDIR)/`basename $(LIBSMBSHAREMODES_SHARED_TARGET)` ; \
	fi
	-$(INSTALLLIBCMD_A) $(LIBSMBSHAREMODES_STATIC_TARGET) $(DESTDIR)$(LIBDIR)
	@$(SHELL) $(srcdir)/script/installdirs.sh $(INSTALLPERMS_BIN) $(DESTDIR) ${prefix}/include
	-$(INSTALLCMD) -m $(INSTALLPERMS_DATA) $(LIBSMBSHAREMODES_HEADERS) $(DESTDIR)${prefix}/include

uninstalllibsmbsharemodes::
	-rm -f $(DESTDIR)$(LIBDIR)/`basename $(LIBSMBSHAREMODES_SHARED_TARGET)`
	-$(UNINSTALLLIBCMD_SH) $(DESTDIR)$(LIBDIR)/`basename $(LIBSMBSHAREMODES_SHARED_TARGET_SONAME)`
	-$(UNINSTALLLIBCMD_A) $(DESTDIR)$(LIBDIR)/`basename $(LIBSMBSHAREMODES_STATIC_TARGET)`
	-rm -f $(DESTDIR)${prefix}/include/smb_share_modes.h

#libs:: libsmbsharemodes
#installlibs:: 
#uninstalllibs:: 
#cleanlibs:: cleanlibsmbsharemodes
#shlibs test_shlibs: 

#-------------------------------------------------------------------
#
# libbigballofmud
#
#-------------------------------------------------------------------

# This is probably wrong for anything other than the GNU linker.
bin/libbigballofmud.so: $(BINARY_PREREQS) $(LIBBIGBALLOFMUD_OBJ) 
	@echo Linking shared library $@
	@$(SHLD) $(LDSHFLAGS) -o $@ $(LIBBIGBALLOFMUD_OBJ) \
		$(LIBS) $(LIBTALLOC_LIBS) \
		$(PASSDB_LIBS) $(IDMAP_LIBS) $(KRB5LIBS) $(LDAP_LIBS) \
		-Wl,-soname=`basename $@`.$(LIBBIGBALLOFMUD_MAJOR)
	ln -snf libbigballofmud.so bin/libbigballofmud.so.0

# It would be nice to build a static bigballofmud too, but when I try
# I get linker errors about dl_open and similar things.  I'm not sure if
# it can be fixed or if they just can't be called from a static
# library.



#########################################################
## IdMap NSS plugins

## None here right now
#########################################################

.PHONY: showlayout

showlayout::
	@echo "Samba will be installed into:"
	@echo "  prefix:      $(prefix)"
	@echo "  bindir:      $(BINDIR)"
	@echo "  sbindir:     $(SBINDIR)"
	@echo "  libdir:      $(LIBDIR)"
	@echo "  vardir:      $(VARDIR)"
	@echo "  mandir:      $(MANDIR)"
	@echo "  privatedir:  $(PRIVATE_DIR)"
	@echo "  configdir:   $(CONFIGDIR)"
	@echo "  lockdir:     $(LOCKDIR)"
	@echo "  piddir:      $(PIDDIR)"
	@echo "  swatdir:     $(SWATDIR)"


# Toplevel clean files
TOPFILES=dynconfig.o

clean:: delheaders cleanlibs
	-rm -f $(PRECOMPILED_HEADER)
	-rm -f core */*~ *~ \
		*/*.o */*/*.o */*/*/*.o \
		../testsuite/*/*.o \
		*/*.so */*/*.so */*/*/*.so \
		$(TOPFILES) $(BIN_PROGS) \
		.headers.stamp */src/*.o proto_exists
	-rm -rf t_dir

# Making this target will just make sure that the prototype files
# exist, not necessarily that they are up to date.  Since they're
# removed by "make clean" this will always be run when you do anything
# afterwards.
proto_exists:: include/proto.h include/build_env.h \
	client/client_proto.h utils/ntlm_auth_proto.h
	@touch proto_exists

delheaders::
	@echo Removing prototype headers
	@rm -f include/proto.h include/build_env.h \
		winbindd/winbindd_proto.h \
		client/client_proto.h utils/net_proto.h \
		smbd/build_options.c utils/ntlm_auth_proto.h \
		utils/passwd_proto.h libnet/libnet_proto.h

MKPROTO_SH = $(srcdir)/script/mkproto.sh

include/proto.h: smbd/build_options.c
	@echo Building include/proto.h
	@cd $(srcdir) && $(SHELL) $(MKPROTO_SH) $(AWK) \
	  -h _PROTO_H_ $(builddir)/include/proto.h \
	  $(PROTO_OBJ)

include/build_env.h: script/build_env.sh
	@echo Building include/build_env.h
	@$(SHELL) $(srcdir)/script/build_env.sh $(srcdir) $(builddir) $(CC) \
	  > $(builddir)/include/build_env.h

winbindd/winbindd_proto.h:
	@cd $(srcdir) && $(SHELL) $(MKPROTO_SH) $(AWK) \
	  -h _WINBINDD_PROTO_H_ $(builddir)/winbindd/winbindd_proto.h \
	  $(WINBINDD_OBJ1)

web/swat_proto.h:
	@cd $(srcdir) && $(SHELL) $(MKPROTO_SH) $(AWK) \
	  -h _SWAT_PROTO_H_ $(builddir)/web/swat_proto.h \
	  $(SWAT_OBJ1)

client/client_proto.h:
	@cd $(srcdir) && $(SHELL) $(MKPROTO_SH) $(AWK) \
	  -h _CLIENT_PROTO_H_ $(builddir)/client/client_proto.h \
	  $(CLIENT_OBJ1)

utils/net_proto.h:
	@cd $(srcdir) && $(SHELL) $(MKPROTO_SH) $(AWK) \
	  -h _NET_PROTO_H_ $(builddir)/utils/net_proto.h \
	  $(NET_OBJ1)

utils/passwd_proto.h:
	@cd $(srcdir) && $(SHELL) $(MKPROTO_SH) $(AWK) \
	  -h _PASSWD_PROTO_H_ $(builddir)/utils/passwd_proto.h \
	  $(PASSWD_UTIL_OBJ)

utils/ntlm_auth_proto.h:
	@cd $(srcdir) && $(SHELL) $(MKPROTO_SH) $(AWK) \
	  -h _NTLM_AUTH_PROTO_H_ $(builddir)/utils/ntlm_auth_proto.h \
	  $(NTLM_AUTH_OBJ1)

libnet/libnet_proto.h:
	@cd $(srcdir) && $(SHELL) $(MKPROTO_SH) $(AWK) \
	  -h _LIBNET_PROTO_H_ $(builddir)/libnet/libnet_proto.h \
	  $(LIBNET_OBJ)

# "make headers" or "make proto" calls a subshell because we need to
# make sure these commands are executed in sequence even for a
# parallel make.
headers::
	$(MAKE) delheaders; \
	$(MAKE) smbd/build_options.c; \
	$(MAKE) include/proto.h; \
	$(MAKE) include/build_env.h; \
	$(MAKE) winbindd/winbindd_proto.h; \
	$(MAKE) web/swat_proto.h; \
	$(MAKE) client/client_proto.h; \
	$(MAKE) utils/ntlm_auth_proto.h; \
	$(MAKE) utils/net_proto.h; \
	$(MAKE) utils/passwd_proto.h; \
	$(MAKE) libnet/libnet_proto.h;

proto:: headers

.PHONY: headers proto

etags:
	etags `find $(srcdir) -name "*.[ch]" | grep -v /.svn/`

ctags:
	ctags `find $(srcdir) -name "*.[ch]" | grep -v /.svn/`

realclean:: clean delheaders
	-rm -f config.log bin/.dummy script/findsmb script/gen-8bit-gap.sh
	-rm -f script/installbin.sh script/uninstallbin.sh

distclean:: realclean
	-rm -f smbadduser
	-rm -f include/config.h Makefile
	-rm -f config.status config.cache so_locations
	-rm -rf .deps TAGS

realdistclean:: distclean
	-rm -f include/config.h.in
	-rm -f include/version.h
	-rm -f configure

# this target is really just for my use. It only works on a limited
# range of machines and is used to produce a list of potentially
# dead (ie. unused) functions in the code. (tridge)
finddead::
	nm */*.o |grep 'U ' | awk '{print $$2}' | sort -u > nmused.txt
	nm */*.o |grep 'T ' | awk '{print $$3}' | sort -u > nmfns.txt
	comm -13 nmused.txt nmfns.txt


######################################################################
# Samba Testing Framework

# Check shared libs for unresolved symbols
test_shlibs:
	@echo "Testing $? "
	@$(LIB_PATH_VAR)=./bin && \
	export $(LIB_PATH_VAR) && \
	for module in $?; do \
		./script/tests/dlopen.sh $${module} \
			|| exit 1; \
	done

# Check for NSS module problems.
test_nss_modules:: nss_modules
	@echo "Testing $(NSS_MODULES) "
	@$(LIB_PATH_VAR)=./bin && \
	export $(LIB_PATH_VAR) && \
	for module in $(NSS_MODULES); do \
		./script/tests/dlopen.sh $${module} \
			|| exit 1; \
	done

# Check for PAM module problems.  Specifically, check that every module we
# built can actually be loaded by a minimal PAM-aware application.
test_pam_modules:: pam_modules
	@echo "Testing $(PAM_MODULES) "
	@$(LIB_PATH_VAR)=./bin && \
	export $(LIB_PATH_VAR) && \
	for module in $(PAM_MODULES); do \
		./script/tests/dlopen.sh -lpam -ldl bin/$${module}.so \
			|| exit 1; \
	done


##
## Targets for 'make test'
##
test:: all torture timelimit
	@echo Running Test suite
	@LIB_PATH_VAR=$(LIB_PATH_VAR) PERL="$(PERL)" $(srcdir)/script/tests/selftest.sh ${selftest_prefix} all "${smbtorture4_path}"

valgrindtest:: all torture timelimit
	@echo Running Test suite with valgrind
	@LIB_PATH_VAR=$(LIB_PATH_VAR) \
	 NMBD_VALGRIND="xterm -n nmbd -e valgrind -q --db-attach=yes --num-callers=30" \
	 WINBINDD_VALGRIND="xterm -n winbindd -e valgrind -q --db-attach=yes --num-callers=30" \
	 SMBD_VALGRIND="xterm -n smbd -e valgrind -q --db-attach=yes --num-callers=30" \
	 VALGRIND="valgrind -q --num-callers=30 --log-file=${selftest_prefix}/valgrind.log" \
	 PERL="$(PERL)" $(srcdir)/script/tests/selftest.sh ${selftest_prefix} all "${smbtorture4_path}"
