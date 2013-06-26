include $(DIR_USERS)/.config
include $(DIR_LINUX)/.config

SCRIPTS = bridge.sh disconnect.sh pptp.sh upnp.sh \
killsh.sh connect.sh  dos.sh  l2tp.sh  \
ddns.sh firewall.sh lan_bridge.sh reload.sh fixedip.sh miniigd.sh \
restart_pppd.sh  dhcpc.sh init.sh  ntp.sh \
dhcpd.sh ip_qos.sh pppoe.sh schedule.sh del_route.sh set_route.sh
#wlan.sh wlan_8196.sh wlanapp.sh wlanapp_8196.sh  wlanapp_kld_repeater.sh
ifeq ($(CONFIG_IPV6),y)
SCRIPTS += ipv6BasicSetting.sh
endif

ifeq ($(CONFIG_APP_RADVD),y)
SCRIPTS += radvd.sh
endif

ifeq ($(CONFIG_APP_DHCPV6),y)
SCRIPTS += dhcp6s.sh
endif

ifeq ($(CONFIG_APP_DNSMASQ),y)
SCRIPTS += dnsv6.sh
endif

ifeq ($(CONFIG_APP_SAMBA),y)
SCRIPTS += samba.sh
endif

ifeq ($(CONFIG_APP_CTORRENT),y)
SCRIPTS += bt.sh
endif

all :
	echo "script"
romfs :
	@cp $(SCRIPTS) $(DIR_ROMFS)/bin/
	$(ROMFSINST) wlan_8196.sh /bin/wlan.sh
	$(ROMFSINST) wlanapp_8196.sh /bin/wlanapp.sh
ifeq ($(CONFIG_APP_WSC),y)
	$(ROMFSINST) simplecfgservice.xml /etc/simplecfgservice.xml
	$(ROMFSINST) -s /var/wps /etc/simplecfg
endif
ifeq ($(CONFIG_APP_MINIIGD),y)
	@rm $(DIR_ROMFS)/bin/upnp.sh
	@mv $(DIR_ROMFS)/bin/miniigd.sh $(DIR_ROMFS)/bin/upnp.sh
endif
	# export CONFIG_RTL_HW_NAPT variable from .config to init.sh/firewall.sh/schedule.sh
	perl -i -pe "s/###CONFIG_RTL_HW_NAPT###/CONFIG_RTL_HW_NAPT=$(CONFIG_RTL_HW_NAPT)/" $(DIR_ROMFS)/bin/init.sh
	perl -i -pe "s/###CONFIG_RTL_HW_NAPT###/CONFIG_RTL_HW_NAPT=$(CONFIG_RTL_HW_NAPT)/" $(DIR_ROMFS)/bin/firewall.sh
	perl -i -pe "s/###CONFIG_RTL_HW_NAPT###/CONFIG_RTL_HW_NAPT=$(CONFIG_RTL_HW_NAPT)/" $(DIR_ROMFS)/bin/schedule.sh
	# export CONFIG_NET_SCHED variable from .config to init.sh/firewall.sh
	perl -i -pe "s/###CONFIG_NET_SCHED###/CONFIG_NET_SCHED=$(CONFIG_NET_SCHED)/" $(DIR_ROMFS)/bin/init.sh
	perl -i -pe "s/###CONFIG_NET_SCHED###/CONFIG_NET_SCHED=$(CONFIG_NET_SCHED)/" $(DIR_ROMFS)/bin/firewall.sh
	## export CONFIG_RTL_WAPI_SUPPORT variable from .config to init.sh
	perl -i -pe "s/###CONFIG_RTL_WAPI_SUPPORT###/CONFIG_RTL_WAPI_SUPPORT=$(CONFIG_RTL_WAPI_SUPPORT)/" $(DIR_ROMFS)/bin/init.sh
	## export CONFIG_RTL_WAPI_SUPPORT variable from .config to wlanapp_8196.sh
	perl -i -pe "s/###CONFIG_RTL_WAPI_SUPPORT###/CONFIG_RTL_WAPI_SUPPORT=$(CONFIG_RTL_WAPI_SUPPORT)/" $(DIR_ROMFS)/bin/wlanapp_8196.sh
	## export CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT variable from .config to init.sh
	perl -i -pe "s/###CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT###/CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT=$(CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT)/" $(DIR_ROMFS)/bin/init.sh
	## export CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT variable from .config to wlanapp_8196.sh
	perl -i -pe "s/###CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT###/CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT=$(CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT)/" $(DIR_ROMFS)/bin/wlanapp_8196.sh
	## export CONFIG_RTK_VLAN_SUPPORT variable from .config to init.sh and bridge.sh
	perl -i -pe "s/###CONFIG_RTK_VLAN_SUPPORT###/CONFIG_RTK_VLAN_SUPPORT=$(CONFIG_RTK_VLAN_SUPPORT)/" $(DIR_ROMFS)/bin/init.sh
	perl -i -pe "s/###CONFIG_RTK_VLAN_SUPPORT###/CONFIG_RTK_VLAN_SUPPORT=$(CONFIG_RTK_VLAN_SUPPORT)/" $(DIR_ROMFS)/bin/bridge.sh
	
	## export CONFIG_MULTI_LAN_DEV variable from .config to init.sh and bridge.sh
	perl -i -pe "s/###CONFIG_RTL_MULTI_LAN_DEV###/CONFIG_RTL_MULTI_LAN_DEV=$(CONFIG_RTL_MULTI_LAN_DEV)/" $(DIR_ROMFS)/bin/init.sh
	perl -i -pe "s/###CONFIG_RTL_MULTI_LAN_DEV###/CONFIG_RTL_MULTI_LAN_DEV=$(CONFIG_RTL_MULTI_LAN_DEV)/" $(DIR_ROMFS)/bin/bridge.sh
	
	## export CONFIG_APP_SAMBA variable from .config to init.sh 
	perl -i -pe "s/###CONFIG_APP_SAMBA###/CONFIG_APP_SAMBA=$(CONFIG_APP_SAMBA)/" $(DIR_ROMFS)/bin/init.sh

	# export CONFIG_RTL_8198_GW variable form .config to ip_qos.sh
	perl -i -pe "s/###CONFIG_RTL_8198_GW###/CONFIG_RTL_8198_GW=$(CONFIG_RTL_8198_GW)/" $(DIR_ROMFS)/bin/ip_qos.sh

