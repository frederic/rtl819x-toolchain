#!/bin/sh
#
export CONFIG_SITE=config.site

echo "ac_cv_func_prctl=yes" >> config.cache
echo "ac_cv_func_getgrouplist=yes" >> config.cache
./configure \
                        --host=mips-linux \
                        --target=mips-linux \
                        --with-configdir=/etc/samba \
                        --config-cache \
                        --with-sendfile-support \
                        --with-aio-support \
                        --enable-largefile \
                        --disable-cups \
                        --disable-iprint \
                        --disable-cifs \
                        --disable-dnssd \
                        --disable-fam \
                        --disable-pie \
                        --disable-swat \
                        --without-acl-support \
                        --without-dnsupdate \
                        --without-automount \
                        --without-cifsmount \
                        --without-cifsupcall \
                        --without-ads \
                        --without-ldap \
                        --without-nisplus-home \
                        --without-pam \
                        --without-pam_smbpass \
                        --without-readline \
                        --without-utmp \
                        --without-syslog \
                        --without-quotas \
                        --without-sys-quotas \
                        --without-winbind \
			--without-libtalloc \
			--without-libtdb \
                        --without-libnetapi \
                        --without-libsmbclient \
                        --without-libsmbsharemodes \
                        --without-libaddns
sed -i -e 's,#define HAVE_RPC_RPC_H 1,,' include/config.h


