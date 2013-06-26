/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /tmp/CN_VoIP/cvsroot/VoIP-rtl865x/linux-2.4.18/rtk_voip/voip_dsp/include/asicRegs.h,v 1.3 2008-01-15 06:33:23 kennylin Exp $
*
* Abstract: ASIC specific definitions.
*
* $Author: kennylin $
*
* $Log: not supported by cvs2svn $
* Revision 1.1  2006/07/19 08:01:12  qwert
* Qwert : new voip_dsp paths
*
* Revision 1.1  2006/06/20 13:13:06  kennylin
* reorganize rtk_voip basing on Branch: merged-3way 2006-06-13
*
* Revision 1.1.1.1  2005/10/25 05:50:15  bruce
* SD4 VoIP ATA project! initial version!
*
* Revision 1.1  2005/09/12 15:00:15  sandro
*
*  Committing in .
*
*  Added Files:
*  	Makefile asicRegs.h assert.h dsp_main.c dsp_main.h
*  	voip_dsp.h mbuf.h myTypes.h pcm865x.c pcm_interface.h
*  	rtl_queue.h rtl_types.h types.h voip_support.h
*
* Revision 1.1  2005/07/21 10:57:18  sandro
*  Added Files:
*  	Makefile asicRegs.h assert.h dsp_main.c dsp_main.h dspapi.c
*  	dspapi.h voip_dsp.h mbuf.h myTypes.h pcm.h pcm865x.c
*  	rtl_queue.h rtl_types.h types.h voip_support.h
*
* Revision 1.1.1.1  2005/06/29 03:46:33  sandro
* Initial import.
*
* Revision 1.16  2005/01/26 08:22:06  cfliu
* +: Commit Generic DMA engine code
*
* Revision 1.15  2004/11/29 07:35:43  chenyl
* *: fix bug : multiple-PPPoE MTU setting bug
* *: fix bug : Never add second session's ip into ASIC in multiple-PPPoE WAN type.
* *: fix bug : Tunekey abnormal behavior when multiple-PPPoE Dial-on-demand Dynamically turn ON/OFF
* *: in forwarding engine Init function, the parameter == 0xffffffff means user want to set as default value
* *: add Mutex-Lock/Unlock checking in testing code
*
* Revision 1.14  2004/09/29 08:36:31  yjlou
* *: redefine 0xBD01____ registers as SWCORE_BASE style.
*
* Revision 1.13  2004/07/28 13:41:41  yjlou
* -: EN_TCPUDP_4WAY_HASH and others no more need CONFIG_RTL865XB flag.
*
* Revision 1.12  2004/07/26 08:40:42  yjlou
* +: add code to check BIST Queue Memory and Packet Buffer when initializing Switch Core
*
* Revision 1.11  2004/05/13 07:23:58  yjlou
* *: change setting, including:
*    100M TxD latch clock phase
*    config LED mode
*    normal backoff
*
* Revision 1.10  2004/05/03 14:37:36  cfliu
* define CHIPID and CRMR
*
* Revision 1.9  2004/04/21 11:53:06  yjlou
* +: add protocol-based VLAN related functions:
*    rtl8651_defineProtocolBasedVLAN()
*    rtl8651_setProtocolBasedVLAN()
*    rtl8651_getProtocolBasedVLAN()
*
* Revision 1.8  2004/04/21 07:06:51  danwu
* change flag CONFIG_RTL8650BBASIC to CONFIG_RTL865XB
*
* Revision 1.7  2004/04/05 10:38:30  yjlou
* +: enable runtime code to detect CPU low-speed lexra bus speed.
*
* Revision 1.6  2004/04/03 07:39:03  jzchen
* Change to B version port mask
*
* Revision 1.5  2004/04/01 06:04:52  chhuang
* enable SWTECR
*
* Revision 1.4  2004/03/26 07:32:35  danwu
* change CONFIG_RTL8650B flag to CONFIG_RTL8650BBASIC temporarily
*
* Revision 1.3  2004/03/18 09:40:08  danwu
* change flag RTL8651B to CONFIG_RTL8650B
*
* Revision 1.2  2004/03/03 10:40:38  yjlou
* *: commit for mergence the difference in rtl86xx_tbl/ since 2004/02/26.
*
* Revision 1.1  2004/02/25 14:26:33  chhuang
* *** empty log message ***
*
* Revision 1.3  2004/02/18 07:02:04  chhuang
* *** empty log message ***
*
* Revision 1.2  2004/02/16 09:47:03  yjlou
* *: make rtl8651_tblDrv.c to fit the new testing model
*
* Revision 1.1  2004/02/16 01:50:49  chhuang
* *** empty log message ***
*
* Revision 1.1  2003/12/30 14:16:23  cfliu
* initial version
*
* Revision 1.1  2003/12/26 05:26:40  chhuang
* init
*
* Revision 1.53  2003/12/19 01:20:52  danwu
* add ACTION_CPU_FOR_LOGGING definition
*
* Revision 1.52  2003/12/18 02:31:54  danwu
* debug OCR register field definitions
*
* Revision 1.51  2003/11/26 00:25:20  cfliu
* disable checking Tx all done, mbuf desc runout intrs for swNic2.c.
* If you still use original swNic.c, you must turn them on.
*
* Revision 1.50  2003/10/28 03:49:25  jzchen
* Fix IRR2 offset typo error
*
* Revision 1.49  2003/10/21 02:52:49  danwu
* add new MIB counters, HSB_RDY, SWTECR, IRR2 registers of 51B
*
* Revision 1.48  2003/10/15 12:37:38  jzchen
* Add GPIO register definition
*
* Revision 1.47  2003/09/23 09:54:48  danwu
* add ACL table new rule and action definitions
*
* Revision 1.46  2003/09/10 09:41:37  jzchen
* Define HSA and HSB addresses
*
* Revision 1.45  2003/09/09 11:28:06  jzchen
* Remove useless crypto-engine definition
*
* Revision 1.44  2003/09/09 07:17:28  jzchen
* Add authentication bit definition
*
* Revision 1.43  2003/09/08 10:25:27  jzchen
* Define authenticaiton register and change (1<<31) to 0x80000000
*
* Revision 1.42  2003/09/04 17:50:18  cfliu
* no message
*
* Revision 1.41  2003/09/03 09:22:19  danwu
* modify for 51b
*
* Revision 1.40  2003/08/28 14:27:07  jzchen
* crypto engine CBC/ECB mode definition error
*
* Revision 1.39  2003/08/28 02:53:21  jzchen
* Define crypto engine part register definition
*
* Revision 1.38  2003/08/18 05:44:42  cfliu
* add BIST_TRXRDY_PATTERN
*
* Revision 1.37  2003/08/15 07:06:15  hiwu
* change STR to STAR
*
* Revision 1.36  2003/08/15 05:15:28  cfliu
* Rename TTL and exponential backoff bits
*
* Revision 1.35  2003/08/05 09:04:47  danwu
* add RTC registers
*
* Revision 1.34  2003/07/21 06:28:23  cfliu
* fix compile warning
*
* Revision 1.33  2003/07/03 01:54:20  jzchen
* Tune register definition to correct one
*
* Revision 1.32  2003/06/18 06:46:14  danwu
* add RTL8650B nic definitions
*
* Revision 1.31  2003/06/16 09:18:34  jzchen
* No parallel port but the same interrupt is PCI interrupt
*
* Revision 1.30  2003/06/16 03:41:35  danwu
* move table definitions to asictabs.h
*
* Revision 1.29  2003/05/26 05:52:04  danwu
* no message
*
* Revision 1.28  2003/04/29 10:10:51  jzchen
* Add layer4 hashing bit0 decision definition
*
* Revision 1.27  2003/04/18 02:14:28  danwu
* invert UART0 and UART1 definitions
*
* Revision 1.26  2003/04/04 04:09:09  cfliu
* REG32 and REG8 type should be unsigned
*
* Revision 1.25  2003/03/07 06:06:42  jzchen
* Fix checksum option flag bug
*
* Revision 1.24  2003/03/05 13:33:32  jzchen
* Fix protocol trapping definition bug and add some definition
*
* Revision 1.23  2003/03/04 13:39:38  danwu
* modify PHY enable fields of MAC control register
*
* Revision 1.22  2003/02/27 03:23:16  danwu
* no message
*
* Revision 1.21  2003/02/21 10:19:51  jzchen
* Correct typo error from spanning tree register to flow cotnrol primary threshold
*
* Revision 1.20  2003/01/14 07:34:18  danwu
* Modify BISTCR register defintions.
*
* Revision 1.19  2002/11/19 10:18:59  henryCho
* Add definitions of lexra bus timeout control register.
*
* Revision 1.18  2002/10/24 09:39:09  danwu
* Modify GPIO definitions.
*
* Revision 1.17  2002/10/23 06:40:41  danwu
* Modify watchdog definitions.
*
* Revision 1.16  2002/10/09 14:30:23  henryCho
* Using TX ALL DONE interrupt
*
* Revision 1.15  2002/10/04 06:24:19  henryCho
* Modify ACTION_S_CPU/S_DROP/N_DROP/N_CPU definitions.
*
* Revision 1.14  2002/09/24 01:30:53  henryCho
* no message
*
* Revision 1.13  2002/09/17 14:05:40  henryCho
* no message
*
* Revision 1.12  2002/09/13 06:17:00  danwu
* Modify TYPE_TRAP_* definitions.
*
* Revision 1.11  2002/09/13 06:09:55  henryCho
* Update PROCESS_* definitions.
*
* Revision 1.10  2002/09/04 02:32:20  danwu
* Add MCAST_PORT_EXT_MODE_MASK.
*
* Revision 1.9  2002/09/03 12:55:40  danwu
* Add ifsel rule of acl table.
*
* Revision 1.8  2002/09/03 10:27:02  delta
* no message
*
* Revision 1.7  2002/09/03 09:33:27  danwu
* Change RESULT_* to TABSTS_*.
*
* Revision 1.6  2002/08/28 11:22:06  danwu
* Add OCR.
*
* Revision 1.5  2002/08/23 06:05:03  danwu
* no message
*
* Revision 1.4  2002/08/21 06:42:08  danwu
* Modify register defintions.
*
* Revision 1.3  2002/08/20 05:57:06  danwu
* Modify table misc control reg definitions.
*
* Revision 1.2  2002/08/19 11:46:28  danwu
* Change to RTL8650.
*
*
*
* ---------------------------------------------------------------
*/

#ifndef _ASICREGS_H
#define _ASICREGS_H



#undef SWTABLE_NO_HW

/* Basic features
*/
#define MAX_PORT_NUMBER           6
#define PORT_NUM_MASK               7
#define ALL_PORT_MASK                 0x3F



/* Register access macro
*/
#define REG32(reg) 			(*((volatile uint32 *)(reg)))
#define REG16(reg) 			(*((volatile uint16 *)(reg)))
#define REG8(reg) 				(*((volatile uint8 *)(reg)))

#define HSB_BASE				0xBC0F0000
#define HSA_BASE				0xBC0F0040

#ifdef RTL865X_TEST
#define SWCORE_BASE			((uint32)pVirtualSWReg)
#define BD01_BASE			((uint32)pVirtualSWReg+0x00100000)	/* SWCORE_BASE must NOT larger than 1MB */
#else
#define SWCORE_BASE			0xBC800000
#define BD01_BASE			0xBD010000
#endif /* RTL865X_TEST */
/* Table access and CPU interface control registers
*/
#define TACI_BASE                 		(SWCORE_BASE + 0x00000000)
#define SWTACR                    		(0x000 + TACI_BASE)     /* Table Access Control */
#define SWTASR                     		(0x004 + TACI_BASE)     /* Table Access Status */
#define SWTAA                       		(0x008 + TACI_BASE)     /* Table Access Address */
#define TCR0                        		(0x020 + TACI_BASE)     /* Table Access Control 0 */
#define TCR1                        		(0x024 + TACI_BASE)     /* Table Access Control 1 */
#define TCR2                        		(0x028 + TACI_BASE)     /* Table Access Control 2 */
#define TCR3                        		(0x02C + TACI_BASE)     /* Table Access Control 3 */
#define TCR4                        		(0x030 + TACI_BASE)     /* Table Access Control 4 */
#define TCR5                        		(0x034 + TACI_BASE)     /* Table Access Control 5 */
#define TCR6                        		(0x038 + TACI_BASE)     /* Table Access Control 6 */
#define TCR7					(0x03C + TACI_BASE)     /* Table Access Control 7 */
/* Table access control register field definitions
*/
#define ACTION_MASK                 	1
#define ACTION_DONE                 	0
#define ACTION_START                	1
#define CMD_MASK                    	(7 << 1)
#define CMD_ADD                     	(1 << 1)
#define CMD_MODIFY                  	(2 << 1)
#define CMD_FORCE                   	(4 << 1)
/* Table access status register field definitions 
*/
#define TABSTS_MASK                 	1
#define TABSTS_SUCCESS              	0
#define TABSTS_FAIL                 	1
/* Vlan table access definitions 
*/
#define STP_DISABLE                 	0
#define STP_BLOCK                   	1
#define STP_LEARN                   	2
#define STP_FORWARD                 	3
/* Protocol trapping table access definitions
*/
#define TYPE_TRAP_ETHERNET     	0x00
#define TYPE_TRAP_IP                	0x02
#define TYPE_TRAP_TCP               	0x05
#define TYPE_TRAP_UDP               	0x06
/* L3 Routing table access definitions
*/
#define PROCESS_PPPOE               	0x00
#define PROCESS_DIRECT              	0x01
#define PROCESS_INDIRECT          	0x02
#define PROCESS_S_CPU             	0x04
#define PROCESS_N_CPU              	0x05
#ifdef CONFIG_RTL865XB
#define PROCESS_NXT_HOP         	0x05
#endif /*CONFIG_RTL865XB*/
#define PROCESS_S_DROP           	0x06
#define PROCESS_N_DROP           	0x07
/* ACL table access definitions
*/
#define RULE_ETHERNET               		0
#define RULE_IP                     			2
#define RULE_IFSEL                  		3
#define RULE_ICMP                   		4
#define RULE_IGMP                  		5
#define RULE_TCP                    		6
#define RULE_UDP                    		7
#define RULE_SRC_FILTER             		8
#define RULE_DST_FILTER             		9
#define ACTION_PERMIT               		0
#define ACTION_REDIRECT             		1
#define ACTION_S_DROP               		2
#define ACTION_S_CPU                		3
#define ACTION_N_DROP               		4
#define ACTION_N_CPU                		5
#define ACTION_CPU_FOR_LOGGING     	5
#define ACTION_MIRROR               		6
#define ACTION_PPPOE_REDIRECT       	7
#define ACTION_DEFAULT_REDIRECT     	8
#define ACTION_DEFAULT_MIRROR       	9
#define ACTION_DROP_RATE_PPS        	10
#define ACTION_LOG2CPU_RATE_PPS    11
#define ACTION_DROP_RATE_BPS        	12
#define ACTION_LOG2CPU_RATE_BPS    13



/* MIB counter registers
*/
#define MIB_COUNTER_BASE                            (SWCORE_BASE + 0x00001000)
#define ETHER_STATS_OCTETS                          (0x000 + MIB_COUNTER_BASE)
#define ETHER_STATS_DROP_EVENTS                     (0x004 + MIB_COUNTER_BASE)
#define ETHER_STATS_CRC_ALIGN_ERRORS                (0x008 + MIB_COUNTER_BASE)
#define ETHER_STATS_FRAGMENTS                       (0x00C + MIB_COUNTER_BASE)
#define ETHER_STATS_JABBERS                         (0x010 + MIB_COUNTER_BASE)
#define IF_IN_UCAST_PKTS                            (0x014 + MIB_COUNTER_BASE)
#define ETHER_STATS_MULTICAST_PKTS                  (0x018 + MIB_COUNTER_BASE)
#define ETHER_STATS_BROADCAST_PKTS                  (0x01C + MIB_COUNTER_BASE)
#define ETHER_STATS_UNDERSIZE_PKTS                  (0x020 + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_64_OCTETS                  (0x024 + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_65_TO_127_OCTETS           (0x028 + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_128_TO_255_OCTETS          (0x02C + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_256_TO_511_OCTETS          (0x030 + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_512_TO_1023_OCTETS         (0x034 + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_1024_TO_1518_OCTETS        (0x038 + MIB_COUNTER_BASE)
#define ETHER_STATS_OVERSIZE_PKTS                   (0x03C + MIB_COUNTER_BASE)
#define DOT3_CONTROL_IN_UNKNOWN_OPCODES             (0x040 + MIB_COUNTER_BASE)
#define DOT3_IN_PAUSE_FRAMES                        (0x044 + MIB_COUNTER_BASE)
#define IF_OUT_OCTETS                               (0x048 + MIB_COUNTER_BASE)
#define IF_OUT_UCAST_PKTS                           (0x04C + MIB_COUNTER_BASE)
#define IF_OUT_MULTICASTCAST_PKTS                   (0x050 + MIB_COUNTER_BASE)
#define IF_OUT_BROADCASTCAST_PKTS                   (0x054 + MIB_COUNTER_BASE)
#define DOT3_STATS_LATE_COLLISIONS                  (0x05C + MIB_COUNTER_BASE)
#define DOT3_STATS_DEFERRED_TRANSMISSIONS           (0x05C + MIB_COUNTER_BASE)
#define ETHER_STATS_COLLISIONS                      (0x060 + MIB_COUNTER_BASE)
#define DOT3_STATS_SINGLE_COLLISION_FRAMES          (0x064 + MIB_COUNTER_BASE)
#define DOT3_STATS_MULTIPLE_COLLISION_FRAMES        (0x068 + MIB_COUNTER_BASE)
#define DOT3_OUT_PAUSE_FRAMES                       (0x06C + MIB_COUNTER_BASE)
#define MIB_CONTROL                                 (0x070 + MIB_COUNTER_BASE)
/* MIB control register field definitions 
*/
#define IN_COUNTER_RESTART                          (1 << 31)
#define OUT_COUNTER_RESTART                         (1 << 30)
#define PORT_FOR_COUNTING_MASK                      0x3FE00000
#define PORT_FOR_COUNTING_OFFSET                    24	//8650 offset while 8650B offset is 21

#define CPU_EVENTS                                  (0x074 + MIB_COUNTER_BASE)
#define MIB_CONTROL_1                               (0x078 + MIB_COUNTER_BASE)
#define MIB_CONTROL_2                               (0x07C + MIB_COUNTER_BASE)
#define EXT_PORT_FOR_COUNTING_MASK                  0x00E00000
#define EXT_PORT_FOR_COUNTING_OFFSET                21
#define RX_BYTES_1                                  (0x080 + MIB_COUNTER_BASE)
#define RX_PKTS_1                                   (0x084 + MIB_COUNTER_BASE)
#define DROP_EVENTS_1                               (0x088 + MIB_COUNTER_BASE)
#define CPU_EVENTS_1                                (0x08C + MIB_COUNTER_BASE)
#define RX_BYTES_2                                  (0x090 + MIB_COUNTER_BASE)
#define RX_PKTS_2                                   (0x094 + MIB_COUNTER_BASE)
#define DROP_EVENTS_2                               (0x098 + MIB_COUNTER_BASE)
#define CPU_EVENTS_2                                (0x09C + MIB_COUNTER_BASE)
#define TX_BYTES_1                                  (0x0A0 + MIB_COUNTER_BASE)
#define TX_PKTS_1                                   (0x0A4 + MIB_COUNTER_BASE)
#define TX_BYTES_2                                  (0x0A8 + MIB_COUNTER_BASE)
#define TX_PKTS_2                                   (0x0AC + MIB_COUNTER_BASE)



/* PHY control registers 
*/
#define PHY_BASE                                    (SWCORE_BASE + 0x00002000)
#define PORT0_PHY_CONTROL                           (0x000 + PHY_BASE)
#define PORT0_PHY_STATUS                            (0x004 + PHY_BASE)
#define PORT0_PHY_IDENTIFIER_1                      (0x008 + PHY_BASE)
#define PORT0_PHY_IDENTIFIER_2                      (0x00C + PHY_BASE)
#define PORT0_PHY_AUTONEGO_ADVERTISEMENT            (0x010 + PHY_BASE)
#define PORT0_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x014 + PHY_BASE)
#define PORT1_PHY_CONTROL                           (0x020 + PHY_BASE)
#define PORT1_PHY_STATUS                            (0x024 + PHY_BASE)
#define PORT1_PHY_IDENTIFIER_1                      (0x028 + PHY_BASE)
#define PORT1_PHY_IDENTIFIER_2                      (0x02C + PHY_BASE)
#define PORT1_PHY_AUTONEGO_ADVERTISEMENT            (0x030 + PHY_BASE)
#define PORT1_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x034 + PHY_BASE)
#define PORT2_PHY_CONTROL                           (0x040 + PHY_BASE)
#define PORT2_PHY_STATUS                            (0x044 + PHY_BASE)
#define PORT2_PHY_IDENTIFIER_1                      (0x048 + PHY_BASE)
#define PORT2_PHY_IDENTIFIER_2                      (0x04C + PHY_BASE)
#define PORT2_PHY_AUTONEGO_ADVERTISEMENT            (0x050 + PHY_BASE)
#define PORT2_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x054 + PHY_BASE)
#define PORT3_PHY_CONTROL                           (0x060 + PHY_BASE)
#define PORT3_PHY_STATUS                            (0x064 + PHY_BASE)
#define PORT3_PHY_IDENTIFIER_1                      (0x068 + PHY_BASE)
#define PORT3_PHY_IDENTIFIER_2                      (0x06C + PHY_BASE)
#define PORT3_PHY_AUTONEGO_ADVERTISEMENT            (0x070 + PHY_BASE)
#define PORT3_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x074 + PHY_BASE)
#define PORT4_PHY_CONTROL                           (0x080 + PHY_BASE)
#define PORT4_PHY_STATUS                            (0x084 + PHY_BASE)
#define PORT4_PHY_IDENTIFIER_1                      (0x088 + PHY_BASE)
#define PORT4_PHY_IDENTIFIER_2                      (0x08C + PHY_BASE)
#define PORT4_PHY_AUTONEGO_ADVERTISEMENT            (0x090 + PHY_BASE)
#define PORT4_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x094 + PHY_BASE)
#define PORT5_PHY_CONTROL                           (0x0A0 + PHY_BASE)
#define PORT5_PHY_STATUS                            (0x0A4 + PHY_BASE)
#define PORT5_PHY_IDENTIFIER_1                      (0x0A8 + PHY_BASE)
#define PORT5_PHY_IDENTIFIER_2                      (0x0AC + PHY_BASE)
#define PORT5_PHY_AUTONEGO_ADVERTISEMENT            (0x0B0 + PHY_BASE)
#define PORT5_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x0B4 + PHY_BASE)
#define PORT6_PHY_CONTROL                           (0x0C0 + PHY_BASE)
#define PORT6_PHY_STATUS                            (0x0C4 + PHY_BASE)
#define PORT6_PHY_IDENTIFIER_1                      (0x0C8 + PHY_BASE)
#define PORT6_PHY_IDENTIFIER_2                      (0x0CC + PHY_BASE)
#define PORT6_PHY_AUTONEGO_ADVERTISEMENT            (0x0D0 + PHY_BASE)
#define PORT6_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x0D4 + PHY_BASE)
/* PHY control register field definitions 
*/
#define PHY_RESET                                   (1 << 15)
#define ENABLE_LOOPBACK                             (1 << 14)
#define SPEED_SELECT_100M                           (1 << 13)
#define SPEED_SELECT_10M                            0
#define ENABLE_AUTONEGO                             (1 << 12)
#define POWER_DOWN                                  (1 << 11)
#define ISOLATE_PHY                                 (1 << 10)
#define RESTART_AUTONEGO                            (1 << 9)
#define SELECT_FULL_DUPLEX                          (1 << 8)
#define SELECT_HALF_DUPLEX                          0
/* PHY status register field definitions 
*/
#define STS_CAPABLE_100BASE_T4                      (1 << 15)
#define STS_CAPABLE_100BASE_TX_FD                   (1 << 14)
#define STS_CAPABLE_100BASE_TX_HD                   (1 << 13)
#define STS_CAPABLE_10BASE_TX_FD                    (1 << 12)
#define STS_CAPABLE_10BASE_TX_HD                    (1 << 11)
#define STS_MF_PREAMBLE_SUPPRESSION                 (1 << 6)
#define STS_AUTONEGO_COMPLETE                       (1 << 5)
#define STS_REMOTE_FAULT                            (1 << 4)
#define STS_CAPABLE_NWAY_AUTONEGO                   (1 << 3)
#define STS_LINK_ESTABLISHED                        (1 << 2)
#define STS_JABBER_DETECTED                         (1 << 1)
#define STS_CAPABLE_EXTENDED                        (1 << 0)
/* PHY identifier 1 
*/
#define OUT_3_18_MASK                               (0xFFFF << 16)
#define OUT_3_18_OFFSET                             16
#define OUT_19_24_MASK                              (0x3F << 10)
#define OUT_19_24_OFFSET                            10
#define MODEL_NUMBER_MASK                           (0x3F << 4)
#define MODEL_NUMBER_OFFSET                         4
#define REVISION_NUMBER_MASK                        0x0F
#define REVISION_NUMBER_OFFSET                      0
/* PHY auto-negotiation advertisement and 
link partner ability registers field definitions
*/
#define NEXT_PAGE_ENABLED                           (1 << 15)
#define ACKNOWLEDGE                                 (1 << 14)
#define REMOTE_FAULT                                (1 << 13)
#define CAPABLE_PAUSE                               (1 << 10)
#define CAPABLE_100BASE_T4                          (1 << 9)
#define CAPABLE_100BASE_TX_FD                       (1 << 8)
#define CAPABLE_100BASE_TX_HD                       (1 << 7)
#define CAPABLE_10BASE_TX_FD                        (1 << 6)
#define CAPABLE_10BASE_TX_HD                        (1 << 5)
#define SELECTOR_MASK                               0x1F
#define SELECTOR_OFFSET                             0



/* CPU interface Tx/Rx packet registers 
*/
#define CPU_IFACE_BASE                      (SWCORE_BASE + 0x00004000)
#define CPUICR                              (0x000 + CPU_IFACE_BASE)    /* Interface control */
#define CPURPDCR                            (0x004 + CPU_IFACE_BASE)    /* Rx pkthdr descriptor control */
#define CPURMDCR                            (0x008 + CPU_IFACE_BASE)    /* Rx mbuf descriptor control */
#define CPUTPDCR                            (0x00C + CPU_IFACE_BASE)    /* Tx pkthdr descriptor control */
#define CPUIIMR                             (0x010 + CPU_IFACE_BASE)    /* Interrupt mask control */
#define CPUIISR                             (0x014 + CPU_IFACE_BASE)    /* Interrupt status control */
/* CPU interface control register field definitions 
*/
#define TXCMD                               (1 << 31)       /* Enable Tx */
#define RXCMD                               (1 << 30)       /* Enable Rx */
#define BUSBURST_32WORDS                    0
#define BUSBURST_64WORDS                    (1 << 28)
#define BUSBURST_128WORDS                   (2 << 28)
#define BUSBURST_256WORDS                   (3 << 28)
#define MBUF_128BYTES                       0
#define MBUF_256BYTES                       (1 << 24)
#define MBUF_512BYTES                       (2 << 24)
#define MBUF_1024BYTES                      (3 << 24)
#define MBUF_2048BYTES                      (4 << 24)
#define TXFD                                (1 << 23)       /* Notify Tx descriptor fetch */
#define SOFTRST                             (1 << 22)       /* Re-initialize all descriptors */
#define STOPTX                              (1 << 21)       /* Stop Tx */
#define SWINTSET                            (1 << 20)       /* Set software interrupt */
#define LBMODE                              (1 << 19)       /* Loopback mode */
#define LB10MHZ                             (1 << 18)       /* LB 10MHz */
#define LB100MHZ                            (1 << 18)       /* LB 100MHz */
#ifdef CONFIG_RTL865XB
#define MITIGATION                          (1 << 17)       /* Mitigation with timer1 */
#define EXCLUDE_CRC                         (1 << 16)       /* Exclude CRC from length */
#define RX_SHIFT_OFFSET                     0
#endif /*CONFIG_RTL865XB*/
/* CPU interface descriptor field defintions 
*/
#define DESC_OWNED_BIT                      1
#define DESC_RISC_OWNED                     0
#define DESC_SWCORE_OWNED                   1
#define DESC_ENG_OWNED				1
#define DESC_WRAP                           (1 << 1)
/* CPU interface interrupt mask register field definitions 
*/
#define LINK_CHANG_IE                       (1 << 31)    /* Link change interrupt enable */
#define RX_ERR_IE                           (1 << 30)    /* Rx error interrupt enable */
#define TX_ERR_IE                           (1 << 29)    /* Tx error interrupt enable */
#define SW_INT_IE                           (1 << 28)    /* Software interrupt enable */
#define L4_COL_REMOVAL_IE                   (1 << 27)    /* L4 collision removal interrupt enable */
#define PKTHDR_DESC_RUNOUT_IE               (1 << 23)    /* Run out pkthdr descriptor interrupt enable */
#define MBUF_DESC_RUNOUT_IE                 (1 << 22)    /* Run out mbuf descriptor interrupt enable */
#define TX_DONE_IE                          (1 << 21)    /* Tx one packet done interrupt enable */
#define RX_DONE_IE                          (1 << 20)    /* Rx one packet done interrupt enable */
#define TX_ALL_DONE_IE                      (1 << 19)    /* Tx all packets done interrupt enable */
/* CPU interface interrupt status register field definitions 
*/
#define LINK_CHANG_IP                       (1 << 31)   /* Link change interrupt pending */
#define RX_ERR_IP                           (1 << 30)   /* Rx error interrupt pending */
#define TX_ERR_IP                           (1 << 29)   /* Tx error interrupt pending */
#define SW_INT_IP                           (1 << 28)   /* Software interrupt pending */
#define L4_COL_REMOVAL_IP                   (1 << 27)   /* L4 collision removal interrupt pending */
#define PKTHDR_DESC_RUNOUT_IP               (1 << 23)   /* Run out pkthdr descriptor interrupt pending */
#define MBUF_DESC_RUNOUT_IP                 (1 << 22)   /* Run out mbuf descriptor interrupt pending */
#define TX_DONE_IP                          (1 << 21)   /* Tx one packet done interrupt pending */
#define RX_DONE_IP                          (1 << 20)   /* Rx one packet done interrupt pending */
#define TX_ALL_DONE_IP                      (1 << 19)   /* Tx all packets done interrupt pending */
#define INTPENDING_NIC_MASK     (RX_ERR_IP | TX_ERR_IP |  RX_DONE_IP|PKTHDR_DESC_RUNOUT_IP)
#define INTPENDING_RUN_OUT		(PKTHDR_DESC_RUNOUT_IP)


/* System control registers 
*/
#define SYSTEM_BASE                         (SWCORE_BASE + 0x00003000)
#define MACCR                               (0x000 + SYSTEM_BASE)   /* MAC control */
#define MACMR                               (0x004 + SYSTEM_BASE)   /* MAC monitor */
#define VLANTCR                             (0x008 + SYSTEM_BASE)   /* Vlan tag control */
#define DSCR0                               (0x00C + SYSTEM_BASE)   /* Qos by DS control */
#define DSCR1                               (0x010 + SYSTEM_BASE)   /* Qos by DS control */
#define QOSCR                               (0x014 + SYSTEM_BASE)   /* Qos control */
#define MISCCR                              (0x018 + SYSTEM_BASE)   /* Switch core misc control */
#define SWTMCR                              (0x01C + SYSTEM_BASE)   /* Switch table misc control */
#define TMRMR                               (0x020 + SYSTEM_BASE)   /* Test mode Rx mii-like */
#define TMTMR                               (0x024 + SYSTEM_BASE)   /* Test mode Tx mii-like */
#define TMCR                                (0x028 + SYSTEM_BASE)   /* Test mode control */
#define MDCIOCR                             (0x02C + SYSTEM_BASE)   /* MDC/MDIO Command */
#define MDCIOSR                             (0x030 + SYSTEM_BASE)   /* MDC/MDIO Status */
/* MAC control register field definitions 
*/
#define DIS_IPG                             (1 << 31)   /* Set IFG */
#define EN_INT_CAM                          (1 << 30)   /* Enable internal CAM */
#define NORMAL_BACKOFF                      (1 << 29)   /* Normal back off slot timer */
#define BACKOFF_EXPONENTIAL_3	              (1 << 28)   /* Set back off exponential parameter 9 */
#define DIS_BACKOFF_BIST                    (1 << 27)   /* Disable back off BIST */
#define ACPT_MAXLEN_1552                    (1 << 26)   /* Set acceptable max length of packet 1552 */
#define FULL_RST                            (1 << 25)   /* Reset all tables & queues */
#define SEMI_RST                            (1 << 24)   /* Reset queues */
#define LONG_TXE                            (1 << 23)   /* Back pressure, carrier based */
#define BYPASS_TCRC                         (1 << 22)   /* Not recalculate CRC error */
#define INFINITE_PAUSE_FRAMES               (1 << 21)   /* Infinite pause frames */
#define IPG_SEL                             (1 << 20)   /* Fixed IPG */
#define EN_48_DROP                          (1 << 19)
#define DIS_MASK_CGST                       (1 << 18)
#define EN_BACK_PRESSURE                    (1 << 17)   /* Enable back pressure */
#define PCI_SRST                            (1 << 15)   /* PCI software reset */
#define PCI2LEXBCR				(0x7<<12)
#define EN_PHY_P4                           (1 << 9)
#define EN_PHY_P3                           (1 << 8)
#define EN_PHY_P2                           (1 << 7)
#define EN_PHY_P1                           (1 << 6)
#define EN_PHY_P0                           (1 << 5)
#define EN_FX_P4                           (1 << 4)
#define EN_FX_P3                           (1 << 3)
#define EN_FX_P2                           (1 << 2)
#define EN_FX_P1                           (1 << 1)
#define EN_FX_P0                           (1 << 0)
/* MAC monitor register field definitions 
*/
#define SYS_CLK_MASK                        (0x7 << 16)
#define SYS_CLK_100M                        (0 << 16)
#define SYS_CLK_90M                         (1 << 16)
#define SYS_CLK_85M                         (2 << 16)
#define SYS_CLK_96M                         (3 << 16)
#define SYS_CLK_80M                         (4 << 16)
#define SYS_CLK_75M                         (5 << 16)
#define SYS_CLK_70M                         (6 << 16)
#define SYS_CLK_50M                         (7 << 16)
/* VLAN tag control register field definitions 
*/
#define VLAN_TAG_ONLY                       (1 << 19)   /* Only accept tagged packets */
/* Qos by DS control register 
*/
/* Qos control register 
*/
#define QWEIGHT_MASK                        (3 << 30)
#define QWEIGHT_ALWAYS_H                    (3 << 30)   /* Weighted round robin of priority always high first */
#define QWEIGHT_16TO1                       (2 << 30)   /* Weighted round robin of priority queue 16:1 */
#define QWEIGHT_8O1                         (1 << 30)   /* Weighted round robin of priority queue 8:1 */
#define QWEIGHT_4TO1                        0           /* Weighted round robin of priority queue 4:1 */
#define EN_FCA_AUTOOFF                      (1 << 29)   /* Enable flow control auto off */
#define DIS_DS_PRI                          (1 << 28)   /* Disable DS priority */
#define DIS_VLAN_PRI                        (1 << 27)   /* Disable 802.1p priority */
#define PORT5_H_PRI                         (1 << 26)   /* Port 5 high priority */
#define PORT4_H_PRI                         (1 << 25)   /* Port 4 high priority */
#define PORT3_H_PRI                         (1 << 24)   /* Port 3 high priority */
#define PORT2_H_PRI                         (1 << 23)   /* Port 2 high priority */
#define PORT1_H_PRI                         (1 << 22)   /* Port 1 high priority */
#define PORT0_H_PRI                         (1 << 21)   /* Port 0 high priority */
#define EN_QOS                              (1 << 20)   /* Enable QoS */
/* Switch core misc control register field definitions 
*/
#define DIS_P5_LOOPBACK                     (1 << 30)   /* Disable port 5 loopback */
#define P5_LINK_MII_MAC                     0           /* Port 5 MII MAC type */
#define P5_LINK_MII_PHY                     1           /* Port 5 MII PHY type */
#define P5_LINK_SNI_MAC                     2           /* Port 5 SNI MAC type */
#define P5_LINK_PCMCIA				3
#define P5_LINK_OFFSET                      28          /* Port 5 link type offset */
//#define P4_USB_SEL                          (1 << 25)   /* Select port USB interface */
#define EN_P5_LINK_PHY                      (1 << 26)   /* Enable port 5 PHY provides link status to MAC */
#define EN_P4_LINK_PHY                      (1 << 25)   /* Enable port 4 PHY provides link status to MAC */
#define EN_P3_LINK_PHY                      (1 << 24)   /* Enable port 3 PHY provides link status to MAC */
#define EN_P2_LINK_PHY                      (1 << 23)   /* Enable port 2 PHY provides link status to MAC */
#define EN_P1_LINK_PHY                      (1 << 22)   /* Enable port 1 PHY provides link status to MAC */
#define EN_P0_LINK_PHY                      (1 << 21)   /* Enable port 0 PHY provides link status to MAC */

#define L4_HASH						(1<<12)
#define FRAG2CPU						(1<<11)
#define MULTICAST_L2_MTU_MASK		0x7ff
/* Switch table misc control register field definitions 
*/
#define NAPTR_NOT_FOUND_DROP                (1 << 0)    /* Reverse NAPT not found to S_DROP */
#define EN_NAPT_AUTO_LEARN                  (1 << 1)    /* Enable NAPT auto learn */
#define EN_NAPT_AUTO_DELETE                 (1 << 2)    /* Enable NAPT auto delete */
#define EN_VLAN_INGRESS_FILTER              (1 << 3)    /* Enable Vlan ingress filtering */

#define WAN_ROUTE_MASK                      (3 << 5)
#define WAN_ROUTE_FORWARD                   0           /* Route WAN packets */
#define WAN_ROUTE_TO_CPU                    (1 << 5)    /* Forward WAN packets to CPU */
#define WAN_ROUTE_DROP                      (2 << 5)    /* Drop WAN packets */
#define MCAST_STP_STS_MASK                  (0x3ff << 7)
#define MCAST_STP_STS_P0_OFFSET             7           /* Multicast spanning tree status port 0 */
#define MCAST_STP_STS_P1_OFFSET             9           /* Multicast spanning tree status port 1 */
#define MCAST_STP_STS_P2_OFFSET             11          /* Multicast spanning tree status port 2 */
#define MCAST_STP_STS_P3_OFFSET             13          /* Multicast spanning tree status port 3 */
#define MCAST_STP_STS_P4_OFFSET             15          /* Multicast spanning tree status port 4 */
#define MCAST_STP_STS_P5_OFFSET             17          /* Multicast spanning tree status port 5 */
#define MCAST_PORT_EXT_MODE_OFFSET          19          /* Multicast port mode offset */
#define MCAST_PORT_EXT_MODE_MASK            (0x3f << 19)    /* Multicast port mode mask */
#define NAPTF2CPU                           (1 << 25)   /* Trap packets not in TCP/UDP/ICMP format and 
                                                        destined to the interface required to do NAPT */
#define EN_MCAST                            (1 << 26)   /* Enable Multicast Table */
#define BRIDGE_PKT_TO_CPU			(1<<27)	//802.1d packet to CPU
#define BCAST_TO_CPU                        (1 << 28)   /* If EN_BCAST is not set, trap broadcast packets 
                                                        to CPU */
#define MCAST_TO_CPU                        (1 << 29)   /* If EN_MCAST is not set, trap multicast packets 
                                                        to CPU */
#define EN_BCAST                            (1 << 30)   /* Enable Broadcast Handling */
#define EN_PPPOE					(1<<31)		/* Enable PPPoE auto-encapsulation and auto-decapsulation */
/* Test mode Rx MII-like register field definitions 
*/
#define P5_RXDV                             (1 << 30)   /* Enable port 0 MII RXDV signal */
#define P4_RXDV                             (1 << 29)   /* Enable port 1 MII RXDV signal */
#define P3_RXDV                             (1 << 28)   /* Enable port 2 MII RXDV signal */
#define P2_RXDV                             (1 << 27)   /* Enable port 3 MII RXDV signal */
#define P1_RXDV                             (1 << 26)   /* Enable port 4 MII RXDV signal */
#define P0_RXDV                             (1 << 25)   /* Enable port 5 MII RXDV signal */
#define NIBBLE_MASK                         0x0F        /* Mask for a nibble */
#define P5_RXD_OFFSET                       20          /* Port 0 RXD MII signal */
#define P4_RXD_OFFSET                       16          /* Port 1 RXD MII signal */
#define P3_RXD_OFFSET                       12          /* Port 2 RXD MII signal */
#define P2_RXD_OFFSET                       8           /* Port 3 RXD MII signal */
#define P1_RXD_OFFSET                       4           /* Port 4 RXD MII signal */
#define P0_RXD_OFFSET                       0           /* Port 5 RXD MII signal */
/* Test mode Tx MII-like register field definitions 
*/
#define P5_TXDV                             (1 << 30)   /* Enable port 0 MII TXDV signal */
#define P4_TXDV                             (1 << 29)   /* Enable port 1 MII TXDV signal */
#define P3_TXDV                             (1 << 28)   /* Enable port 2 MII TXDV signal */
#define P2_TXDV                             (1 << 27)   /* Enable port 3 MII TXDV signal */
#define P1_TXDV                             (1 << 26)   /* Enable port 4 MII TXDV signal */
#define P0_TXDV                             (1 << 25)   /* Enable port 5 MII TXDV signal */
#define P5_TXD_OFFSET                       20          /* Port 0 TXD MII signal */
#define P4_TXD_OFFSET                       16          /* Port 1 TXD MII signal */
#define P3_TXD_OFFSET                       12          /* Port 2 TXD MII signal */
#define P2_TXD_OFFSET                       8           /* Port 3 TXD MII signal */
#define P1_TXD_OFFSET                       4           /* Port 4 TXD MII signal */
#define P0_TXD_OFFSET                       0           /* Port 5 TXD MII signal */
/* Test mode enable register 
*/
#define TX_TEST_PORT_OFFSET                 26          /* Tx test mode enable port offset */
#define RX_TEST_PORT_OFFSET                 18          /* Rx test mode enable port offset */
#define HSB_RDY 	   	   	   	   	   	   	(1 << 0)   	/* HSB ready */
/* Gateway MAC low register 
*/
#define GMACL_OFFSET                        20          /* Gateway MAC[15:4] offset */
#define GMACL_MASK                          (0xFFFFFFFF << 20)  /* Gateway MAC[15:4] mask */



/* Miscellaneous control registers 
*/
#define MISC_BASE                           (SWCORE_BASE + 0x00005000)
#define LEDCR                               (0x000 + MISC_BASE)     /* LED control */
#define BISTCR                              (0x004 + MISC_BASE)     /* BIST control */
#define BCR0                                (0x008 + MISC_BASE)     /* Input bandwidth control */
#define BCR1                                (0x00C + MISC_BASE)     /* Ouput bandwidth control */
#define CSCR                                (0x010 + MISC_BASE)     /* Checksum control */
#define FCREN                               (0x014 + MISC_BASE)     /* Flow control enable control */
#define FCRTH                               (0x018 + MISC_BASE)     /* Flow control threshold */
#define FCPTR                                (0x028 + MISC_BASE)     /* Flow control prime threshold register */
#define PTCR                                (0x01C + MISC_BASE)     /* Port trunk control */
//#ifdef CONFIG_RTL865XB
#define SWTECR                              (0x020 + MISC_BASE)     /* Switch table extended control */
//#endif /*CONFIG_RTL865XB*/
#define PTRAPCR                             (0x024 + MISC_BASE)     /* Protocol trapping control */
#define TTLCR                               (0x02C + MISC_BASE)     /* TTL control */
#define MSCR                                (0x030 + MISC_BASE)     /* Module switch control */
#define BSCR                                (0x038 + MISC_BASE)     /* Broadcast storm control */
#define TEATCR                              (0x03C + MISC_BASE)     /* Table entry aging time control */
#define PMCR                                (0x040 + MISC_BASE)     /* Port mirror control */
#define PPMAR                               (0x044 + MISC_BASE)     /* Per port matching action */
#define PATP0                               (0x048 + MISC_BASE)     /* Pattern for port 0 */
#define PATP1                               (0x04C + MISC_BASE)     /* Pattern for port 1 */
#define PATP2                               (0x050 + MISC_BASE)     /* Pattern for port 2 */
#define PATP3                               (0x054 + MISC_BASE)     /* Pattern for port 3 */
#define PATP4                               (0x058 + MISC_BASE)     /* Pattern for port 4 */
#define PATP5                               (0x05C + MISC_BASE)     /* Pattern for port 5 */
#define MASKP0                              (0x060 + MISC_BASE)     /* Mask for port 0 */
#define MASKP1                              (0x064 + MISC_BASE)     /* Mask for port 1 */
#define MASKP2                              (0x068 + MISC_BASE)     /* Mask for port 2 */
#define MASKP3                              (0x06C + MISC_BASE)     /* Mask for port 3 */
#define MASKP4                              (0x070 + MISC_BASE)     /* Mask for port 4 */
#define MASKP5                              (0x074 + MISC_BASE)     /* Mask for port 5 */
#define PVCR                                (0x078 + MISC_BASE)     /* Port based vlan config */
#define GIDXMCR                             (0x07C + MISC_BASE)     /* GIDX mapping control */
#define OCR                                 (0x080 + MISC_BASE)     /* Offset control */
#define PBVCR								(0x090 + MISC_BASE)		/* Protocol-Based VLAN Control Register        */
#define PBVCR2								(0x094 + MISC_BASE)		/* Protocol-Based VLAN Control Register 2      */
#define PBVR1								(0x098 + MISC_BASE)		/* Protocol-Based VLAN Rule 1 -- IPX           */
#define PBVR1E								(0x09C + MISC_BASE)		/* Protocol-Based VLAN Rule 1 Extended         */
#define PBVR2								(0x0A0 + MISC_BASE)		/* Protocol-Based VLAN Rule 2 -- NetBIOS       */
#define PBVR2E								(0x0A4 + MISC_BASE)		/* Protocol-Based VLAN Rule 2 Extended         */
#define PBVR3								(0x0A8 + MISC_BASE)		/* Protocol-Based VLAN Rule 3 -- PPPoE Control */
#define PBVR3E								(0x0AC + MISC_BASE)		/* Protocol-Based VLAN Rule 3 Extended         */
#define PBVR4								(0x0B0 + MISC_BASE)		/* Protocol-Based VLAN Rule 4 -- PPPoE Session */
#define PBVR4E								(0x0B4 + MISC_BASE)		/* Protocol-Based VLAN Rule 4 Extended         */
#define PBVR5								(0x0B8 + MISC_BASE)		/* Protocol-Based VLAN Rule 5 -- User-defined 1*/
#define PBVR5E								(0x0BC + MISC_BASE)		/* Protocol-Based VLAN Rule 5 Extended         */
#define PBVR6								(0x0C0 + MISC_BASE)		/* Protocol-Based VLAN Rule 6 -- User-defined 2*/
#define PBVR6E								(0x0C4 + MISC_BASE)		/* Protocol-Based VLAN Rule 6 Extended         */
#define CHIPID                               (0x100 + MISC_BASE)     /* Chip version ID */
#define CRMR                                (0x0104 + MISC_BASE)    /* Chip Revision Management Register */
/* LED control register field definitions 
*/
#define LED_P0_SPEED                        (1 << 0)    /* LED port 0 collision */
#define LED_P0_ACT                          (1 << 1)    /* LED port 0 active */
#define LED_P0_COL                          (1 << 2)    /* LED port 0 speed 100M */
#define LED_P1_SPEED                        (1 << 3)    /* LED port 1 collision */
#define LED_P1_ACT                          (1 << 4)    /* LED port 1 active */
#define LED_P1_COL                          (1 << 5)    /* LED port 1 speed 100M */
#define LED_P2_SPEED                        (1 << 6)    /* LED port 2 collision */
#define LED_P2_ACT                          (1 << 7)    /* LED port 2 active */
#define LED_P2_COL                          (1 << 8)    /* LED port 2 speed 100M */
#define LED_P3_SPEED                        (1 << 9)    /* LED port 3 collision */
#define LED_P3_ACT                          (1 << 10)   /* LED port 3 active */
#define LED_P3_COL                          (1 << 11)   /* LED port 3 speed 100M */
#define LED_P4_SPEED                        (1 << 12)   /* LED port 4 collision */
#define LED_P4_ACT                          (1 << 13)   /* LED port 4 active */
#define LED_P4_COL                          (1 << 14)   /* LED port 4 speed 100M */
#define LED_P5_SPEED                        (1 << 15)   /* LED port 5 collision */
#define LED_P5_ACT                          (1 << 16)   /* LED port 5 active */
#define LED_P5_COL                          (1 << 17)   /* LED port 5 speed 100M */
#define EN_LED_CPU_CTRL                     (1 << 18)   /* Enable CPU control LED */
/* BIST control register field definitions 
*/
#define BIST_READY_PATTERN                  0x018F0000
#define BIST_TRXRDY_PATTERN					0x318f0000
#define BIST_QUEUE_MEMORY_FAIL_PATTERN      0x00700000
#define BIST_PACKET_BUFFER_FAIL_PATTERN     0x0E000000
#define TRXRDY                              (1 << 1)    /* Start normal TX and RX */
/* Bandwidth control register field definitions 
*/
#define OUT_BC_P0_OFFSET                    0           /* Output bandwidth control port 0 offset */
#define IN_BC_P0_OFFSET                     4           /* Input bandwidth control port 0 offset */
#define OUT_BC_P1_OFFSET                    8           /* Output bandwidth control port 1 offset */
#define IN_BC_P1_OFFSET                     12          /* Input bandwidth control port 1 offset */
#define OUT_BC_P2_OFFSET                    16          /* Output bandwidth control port 2 offset */
#define IN_BC_P2_OFFSET                     20          /* Input bandwidth control port 2 offset */
#define OUT_BC_P3_OFFSET                    24          /* Output bandwidth control port 3 offset */
#define IN_BC_P3_OFFSET                     48          /* Input bandwidth control port 3 offset */
#define OUT_BC_P4_OFFSET                    0           /* Output bandwidth control port 4 offset */
#define IN_BC_P4_OFFSET                     4           /* Input bandwidth control port 4 offset */
#define OUT_BC_P5_OFFSET                    8           /* Output bandwidth control port 5 offset */
#define IN_BC_P5_OFFSET                     12          /* Input bandwidth control port 5 offset */
#define BW_FULL_RATE                        0
#define BW_128K                             1
#define BW_256K                             2
#define BW_512K                             3
#define BW_1M                               4
#define BW_2M                               5
#define BW_4M                               6
#define BW_8M                               7
/* Checksum control register field definitions 
*/
#define ALLOW_L2_CHKSUM_ERR                 (1 << 0)    /* Allow L2 checksum error */
#define ALLOW_L3_CHKSUM_ERR                 (1 << 1)    /* Allow L3 checksum error */
#define ALLOW_L4_CHKSUM_ERR                 (1 << 2)    /* Allow L4 checksum error */
#define EN_ETHER_L3_CHKSUM_REC              (1 << 3)    /* Enable L3 checksum recalculation*/
#define EN_ETHER_L4_CHKSUM_REC              (1 << 4)    /* Enable L4 checksum recalculation*/
/* Flow control enable register field defintions 
*/
#define EN_INQ_FC_CPU                       (1 << 31)   /* Enable Flow Control on CPU Port */
#define EN_INQ_FC_5                         (1 << 30)   /* Enable Flow Control on Port 5 */
#define EN_INQ_FC_4                         (1 << 29)   /* Enable Flow Control on Port 4 */
#define EN_INQ_FC_3                         (1 << 28)   /* Enable Flow Control on Port 3 */
#define EN_INQ_FC_2                         (1 << 27)   /* Enable Flow Control on Port 2 */
#define EN_INQ_FC_1                         (1 << 26)   /* Enable Flow Control on Port 1 */
#define EN_INQ_FC_0                         (1 << 25)   /* Enable Flow Control on Port 0 */
#define EN_OUTQ_FC_CPU                      (1 << 24)   /* Enable Flow Control on CPU Port */
#define EN_OUTQ_FC_5                        (1 << 23)   /* Enable Flow Control on Port 5 */
#define EN_OUTQ_FC_4                        (1 << 22)   /* Enable Flow Control on Port 4 */
#define EN_OUTQ_FC_3                        (1 << 21)   /* Enable Flow Control on Port 3 */
#define EN_OUTQ_FC_2                        (1 << 20)   /* Enable Flow Control on Port 2 */
#define EN_OUTQ_FC_1                        (1 << 19)   /* Enable Flow Control on Port 1 */
#define EN_OUTQ_FC_0                        (1 << 18)   /* Enable Flow Control on Port 0 */
#define CPU_LAUNCH_FC_P5                    (1 << 17)   /* CPU launch flow control of Port 5 */
#define CPU_LAUNCH_FC_P4                    (1 << 16)   /* CPU launch flow control of Port 4 */
#define CPU_LAUNCH_FC_P3                    (1 << 15)   /* CPU launch flow control of Port 3 */
#define CPU_LAUNCH_FC_P2                    (1 << 14)   /* CPU launch flow control of Port 2 */
#define CPU_LAUNCH_FC_P1                    (1 << 13)   /* CPU launch flow control of Port 1 */
#define CPU_LAUNCH_FC_P0                    (1 << 12)   /* CPU launch flow control of Port 0 */
#define EN_MDC_MDIO_FC                      (1 << 10)   /* Enable MDC/MDIO Flow Control */
#define DSC_TH_OFFSET                       0           /* Descriptor Initial threshold */
/* Flow control threshold register field defintions 
*/
#define IN_Q_PER_PORT_BUF_FC_THH_OFFSET     24          /* InQ per port buffer page flow control high threshold offset */
#define IN_Q_PER_PORT_BUF_FC_THL_OFFSET     16          /* InQ per port buffer page flow control low threshold offset */
#define OUT_Q_PER_PORT_BUF_FC_THH_OFFSET    8           /* OutQ per port buffer page flow control high threshold offset */
#define OUT_Q_PER_PORT_BUF_FC_THL_OFFSET    0           /* OutQ per port buffer page flow control low threshold offset */
/* Flow control prime threshold register field defintions 
*/
#define IN_Q_PTH_OFFSET                     16          /* InQ Prime flow control threshold */
#define OUT_Q_PTH_OFFSET                    0           /* OutQ Prime flow control threshold */
/* Port trunking control register field definitions 
*/
#define LMPR7_OFFSET                        27          /* Physical port index for logical port 7 */
#define LMPR6_OFFSET                        24          /* Physical port index for logical port 6 */
#define LMPR5_OFFSET                        21          /* Physical port index for logical port 5 */
#define LMPR4_OFFSET                        18          /* Physical port index for logical port 4 */
#define LMPR3_OFFSET                        15          /* Physical port index for logical port 3 */
#define LMPR2_OFFSET                        12          /* Physical port index for logical port 2 */
#define LMPR1_OFFSET                        9           /* Physical port index for logical port 1 */
#define LMPR0_OFFSET                        6           /* Physical port index for logical port 0 */
#define TRUNK1_PORT_MASK_OFFSET             0           /* Physical port mask of trunk 1 */
/* Port trunking control register field definitions 
*/
#define EN_PPP_OP                           (1 << 31)   /* Enable PPPoE auto insert and remove */
/* Protocol trapping control register field definitions 
*/
#define EN_ARP_TRAP                         (1 << 24)   /* Enable trapping ARP packets */
#define EN_RARP_TRAP                        (1 << 25)   /* Enable trapping RARP packets */
#define EN_PPPOE_TRAP                       (1 << 26)   /* Enable trapping PPPoE packets */
#define EN_IGMP_TRAP                        (1 << 27)   /* Enable trapping IGMP packets */
#define EN_DHCP_TRAP1                       (1 << 28)   /* Enable trapping DHCP 67 packets */
#define EN_DHCP_TRAP2                       (1 << 29)   /* Enable trapping DHCP 68 packets */
#define EN_OSPF_TRAP                        (1 << 30)   /* Enable trapping OSPF packets */
#define EN_RIP_TRAP                         (1 << 31)   /* Enable trapping RIP packets */
/* Spanning tree control register field definitions 
*/
#define EN_ESTP_S_DROP                      (1 << 31)   /* Enable egress spanning tree forward S_Drop */
/* TTL control register field definitions 
*/
#define EN_TTL1                            (1 << 31)   /* Enable TTL-1 operation */
#define EN_MEM2MEM_DMA           (1 << 28)   /* Enable mem-to-mem DMA with Crypto engine operation */
/* Module switch control register field definitions 
*/
#define MOD_MASK                            7
#define EN_L2                               (1 << 0)    /* Enable L2 module */
#define EN_L3                               (1 << 1)    /* Enable L3 module */
#define EN_L4                               (1 << 2)    /* Enable L4 module */
#define EN_OUT_ACL                          (1 << 3)    /* Enable egress ACL */
#define EN_IN_ACL                           (1 << 4)    /* Enable ingress ACL */
#define EN_STP                              (1 << 7)    /* Enable spanning tree */
#define FORCE_TO_CPU                        (1 << 8)
#define L2_COL_BCAST                        (1 << 9)    /* L2 collision broadcast */
#define I8021D_TO_CPU                       (1 << 10)   /* 802.1D trap to CPU */
#define GARP_TO_CPU                         (1 << 11)   /* GARP trap to CPU */
/* Broadcast storm control register field definitions 
*/
#define EN_BCAST_STORM                      (1 << 0)    /* Enable broadcast storm control */
#define BCAST_TH_MASK                       (0xFF << 1) /* Threshold within broadcast interval mask */
#define BCAST_TH_OFFSET                     1           /* Threshold within broadcast interval offset */
#define TI_100M_MASK                        (0x3F << 9) /* Time interval for 100M mask */
#define TI_100M_OFFSET                      9           /* Time interval for 100M offset */
#define TI_10M_MASK                         (0x3F << 15)/* Time interval for 10M mask */
#define TI_10M_OFFSET                       15          /* Time interval for 10M offset */
/* Table entry aging time control register field definitions 
*/
#define ICMP_TH_OFFSET                      26          /* ICMP timeout threshold offset */
#define ICMP_TH_MASK                        (0x3f << ICMP_TH_OFFSET)
#define UDP_TH_OFFSET                       20          /* UDP timeout threshold offset */
#define UDP_TH_MASK                         (0x3f << UDP_TH_OFFSET)
#define TCP_LONG_TH_OFFSET                  14          /* TCP long timeout threshold offset */
#define TCP_LONG_TH_MASK                    (0x3f << TCP_LONG_TH_OFFSET)
#define TCP_MED_TH_OFFSET                   8           /* TCP medium timeout threshold offset */
#define TCP_MED_TH_MASK                     (0x3f << TCP_MED_TH_OFFSET)
#define TCP_FAST_TH_OFFSET                  2           /* TCP fast timeout threshold offset */
#define TCP_FAST_TH_MASK                    (0x3f << TCP_FAST_TH_OFFSET)
/* Port mirror control register field definitions 
*/
#define MIRROR_TO_PORT_OFFSET               26          /* Port receiving the mirrored traffic offset */
#define MIRROR_FROM_PORT_RX_OFFSET          20          /* Rx port to be mirrored offset */
#define MIRROR_FROM_PORT_TX_OFFSET          14          /* Tx port to be mirrored offset */
/* Per port matching action register field definitions 
*/
#define EN_PMATCH_PORT_LIST_OFFSET          26          /* Enable pattern match port list offset */
#define MATCH_OP_P5_OFFSET                  24          /* Offset of operation if matched on port 5 */
#define MATCH_OP_P4_OFFSET                  22          /* Offset of operation if matched on port 4 */
#define MATCH_OP_P3_OFFSET                  20          /* Offset of operation if matched on port 3 */
#define MATCH_OP_P2_OFFSET                  18          /* Offset of operation if matched on port 2 */
#define MATCH_OP_P1_OFFSET                  16          /* Offset of operation if matched on port 1 */
#define MATCH_OP_P0_OFFSET                  14          /* Offset of operation if matched on port 0 */
#define MATCH_DROP                          0           /* Drop if matched */
#define MATCH_MIRROR_TO_CPU                 1           /* Mirror to CPU if matched */
#define MATCH_FORWARD_TO_CPU                2           /* Forward to CPU if matched */
#define MATCH_TO_MIRROR_PORT                3           /* To mirror port if matched */
/* Port based vlan config register field definitions 
*/
#define PVID_MASK                           7           /* MASK for PVID */
#define VIDP0_OFFSET                        0           /* Vlan table index for port 0 */
#define VIDP1_OFFSET                        3           /* Vlan table index for port 1 */
#define VIDP2_OFFSET                        6           /* Vlan table index for port 2 */
#define VIDP3_OFFSET                        9           /* Vlan table index for port 3 */
#define VIDP4_OFFSET                        12          /* Vlan table index for port 4 */
#define VIDP5_OFFSET                        15          /* Vlan table index for port 5 */
/* GIDX mapping control register field definitions 
*/
#define GIDX_MASK                           7           /* MASK for GIDX */
#define GIDX0_OFFSET                        0           /* Index to IP table */
#define GIDX1_OFFSET                        3           /* Index to IP table */
#define GIDX2_OFFSET                        6           /* Index to IP table */
#define GIDX3_OFFSET                        9           /* Index to IP table */
#define GIDX4_OFFSET                        12          /* Index to IP table */
#define GIDX5_OFFSET                        15          /* Index to IP table */
#define GIDX6_OFFSET                        18          /* Index to IP table */
#define GIDX7_OFFSET                        21          /* Index to IP table */
/* Offset control register field definitions 
*/
#define OCR_START_MASK                      (0x1f << 26)    /* Starting value of offset mask */
#define OCR_START_OFFSET                    26              /* Starting value of offset offset */
#define OCR_END_MASK                        (0x1f << 20)    /* End value of offset mask */
#define OCR_END_OFFSET                      20              /* End value of offset offset */
/* Protocol-based VLAN Control Register field definitions
*/
#define PBVCR_PROTO_TYPE_OFFSET				0			/* Protocol-Based VLAN Control Register Protocol Type */
#define PBVCR_PROTO_TYPE_MASK				(0x3 << PBVCR_PROTO_TYPE_OFFSET)
#define PBVCR_PROTO_VALUE_OFFSET			2			/* Protocol-Based VLAN Control Register Protocol Value */
#define PBVCR_PROTO_VALUE_MASK				(0xffff << PBVCR_PROTO_VALUE_OFFSET)
#define PBVCR_VIDXP5_OFFSET					21			/* VLAN index for Port 5 */
#define PBVCR_VIDXP5_MASK					(0x7 << PBVCR_VIDXP5_OFFSET)
#define PBVCR_VP5_OFFSET					20			/* Valid bit for Port 5 */
#define PBVCR_VP5_MASK						(0x1 << PBVCR_VP5_OFFSET)
#define PBVCR_VIDXP4_OFFSET					17			/* VLAN index for Port 4 */
#define PBVCR_VIDXP4_MASK					(0x7 << PBVCR_VIDXP4_OFFSET)
#define PBVCR_VP4_OFFSET					16			/* Valid bit for Port 4 */
#define PBVCR_VP4_MASK						(0x1 << PBVCR_VP4_OFFSET)
#define PBVCR_VIDXP3_OFFSET					13			/* VLAN index for Port 3 */
#define PBVCR_VIDXP3_MASK					(0x7 << PBVCR_VIDXP3_OFFSET)
#define PBVCR_VP3_OFFSET					12			/* Valid bit for Port 3 */
#define PBVCR_VP3_MASK						(0x1 << PBVCR_VP3_OFFSET)
#define PBVCR_VIDXP2_OFFSET					9			/* VLAN index for Port 2 */
#define PBVCR_VIDXP2_MASK					(0x7 << PBVCR_VIDXP2_OFFSET)
#define PBVCR_VP2_OFFSET					8			/* Valid bit for Port 2 */
#define PBVCR_VP2_MASK						(0x1 << PBVCR_VP2_OFFSET)
#define PBVCR_VIDXP1_OFFSET					5			/* VLAN index for Port 1 */
#define PBVCR_VIDXP1_MASK					(0x7 << PBVCR_VIDXP1_OFFSET)
#define PBVCR_VP1_OFFSET					4			/* Valid bit for Port 1 */
#define PBVCR_VP1_MASK						(0x1 << PBVCR_VP1_OFFSET)
#define PBVCR_VIDXP0_OFFSET					3			/* VLAN index for Port 0 */
#define PBVCR_VIDXP0_MASK					(0x7 << PBVCR_VIDXP0_OFFSET)
#define PBVCR_VP0_OFFSET					0			/* Valid bit for Port 0 */
#define PBVCR_VP0_MASK						(0x1 << PBVCR_VP0_OFFSET)
#define PBVCR_VIDXEP2_OFFSET				9			/* VLAN index for Extented Port 2 */
#define PBVCR_VIDXEP2_MASK					(0x7 << PBVCR_VIDXEP2_OFFSET)
#define PBVCR_VEP2_OFFSET					8			/* Valid bit for Extented Port 2 */
#define PBVCR_VEP2_MASK						(0x1 << PBVCR_VEP2_OFFSET)
#define PBVCR_VIDXEP1_OFFSET				5			/* VLAN index for Extented Port 1 */
#define PBVCR_VIDXEP1_MASK					(0x7 << PBVCR_VIDXEP1_OFFSET)
#define PBVCR_VEP1_OFFSET					4			/* Valid bit for Extented Port 1 */
#define PBVCR_VEP1_MASK						(0x1 << PBVCR_VEP1_OFFSET)
#define PBVCR_VIDXEP0_OFFSET				1			/* VLAN index for Extented Port 0 */
#define PBVCR_VIDXEP0_MASK					(0x7 << PBVCR_VIDXEP0_OFFSET)
#define PBVCR_VEP0_OFFSET					0			/* Valid bit for Extented Port 0 */
#define PBVCR_VEP0_MASK						(0x1 << PBVCR_VEP0_OFFSET)
/* Chip version ID register field definitions 
*/
#define RTL8650_CVID                        0x86500000
/* Switch table extended control register field definitions 
*/
#define EN_RTL8650B                         (1 << 31)   /* Enable 51B mode */
#define EN_FRAG_TO_ACLPT                    (1 << 30)   /* Enable fragment packet checked by ACL and protocol trapper */
#define EN_ATTACK_TO_LOG                    (1 << 29)   /* Enable trapping attack packets for logging */
#define EN_SPI6_WAN_SRVPRT                  (1 << 28)   /* Enable SPI-6 between WAN and server port */
#define EN_SPI5_WAN_NI                      (1 << 27)   /* Enable SPI-5 between WAN and NI */
#define EN_SPI4_WAN_DMZ                     (1 << 26)   /* Enable SPI-4 between WAN and DMZ */
#define EN_SPI3_DMZ_RLAN                    (1 << 25)   /* Enable SPI-3 between DMZ and RLAN */
#define EN_SPI2_DMZ_LAN                     (1 << 24)   /* Enable SPI-2 between DMZ and LAN */
#define EN_SPI1_WAN_RLAN                    (1 << 23)   /* Enable SPI-1 between WAN and RLAN */
#define EN_TCPUDP_4WAY_HASH                 (1 << 22)   /* Enable 4-way hash on L4 TCP/UDP table */
#define EN_51B_CPU_REASON                   (1 << 21)   /* Enable 51B CPU reason coding */
#define EN_FRAG_PPPOE                       (1 << 20)   /* Enable auto-fragment for PPPoE */
#define EN_MCAST_TBL_AGING  	   	   	   	(1 << 19)  	   	/* Enable aging of IP multicast table */
#define EN_PPPOE_TBL_AGING  	   	   	   	(1 << 18)  	   	/* Enable aging of PPPoE table */
#define EN_DROP_SPIR_NOT_FOUND              (1 << 17)   /* Enable dropping packets not found by reverse SPI */
#define EN_SPI_TRAP_UNKNOWN                 (1 << 16)   /* Enable SPI trapping non-TCP/UDP/ICMP packets */
#define EN_HSB_WR   	   	   	   	   	   	(1 << 0)   	   	/* Enable HSB writing */



/* PCMCIA registers 
*/
#define PCMCIA_BASE					(BD01_BASE+0x0000)	/* 0xBD010000 */
#define PCMCIA_CFR					(0x000 + PCMCIA_BASE)      /* PCMCIA Command FIFO Register */
#define PCMCIA_IER					(0x004 + PCMCIA_BASE)      /* PCMCIA Interrupt enable register */
#define PCMCIA_FCR					(0x008 + PCMCIA_BASE)      /* PCMCIA FIFO control register */
#define PCMCIA_ISR					(0x00C + PCMCIA_BASE)      /* PCMCIA Interrupt Status register */
#define PCTCR						(0x010 + PCMCIA_BASE)      /* PCMCIA Timing Control Register */

//PCMCIA CFG
#define RD_PCMCIA_ATTR		0x10000000
#define WR_PCMCIA_ATTR		0x00000000
#define RD_PCMCIA_IO			0x14000000
#define WR_PCMCIA_IO			0x04000000

//PCMCIA IER
#define ERDAI					0x80000000
#define ERFEI					(1<<30)
#define ERFFI					(1<<29)
#define ECFDI					(1<<28)
#define ECFEI					(1<<27)
#define ECFFI					(1<<26)

//PCMCIA FCR
#define EFIFO					0x80000000
#define RFRST				(1<<30)
#define TFRST				(1<<29)
#define RTRG_MASK			(0x3<<27)
#define RTRG_01				(0x0<<27)
#define RTRG_12				(0x1<<27)
#define RTRG_16				(0x2<<27)
#define RTRG_26				(0x3<<27)
#define RFRC_MASK			(0x3f<<21)
#define CFRC_MASK			(0x3f<<15)

//PCMCIA LSR
#define DR					0x80000000
#define RxEMPTY				(1<<30)
#define RxFULL				(1<<29)
#define CMOK					(1<<28)
#define CMEMPTY				(1<<27)
#define CMFULL				(1<<26)

/* UART registers 
*/
#define UART0_BASE                          (BD01_BASE+0x1100)	/* 0xBD011100 */
#define UART1_BASE                          (BD01_BASE+0x1000)	/* 0xBD011000 */




/* System Control Registers */

/* Global interrupt control registers */
#define GICR_BASE                           (BD01_BASE+0x2000)	/* 0xBD012000 */
#define GIMR                                (0x000 + GICR_BASE)       /* Global interrupt mask */
#define GISR                                (0x004 + GICR_BASE)       /* Global interrupt status */
#define IRR                                 (0x008 + GICR_BASE)       /* Interrupt routing */
#ifdef CONFIG_RTL865XB
#define IRR2                                (0x0A0 + GICR_BASE)       /* Interrupt routing 2 */
#endif /*CONFIG_RTL865XB*/
/* Global interrupt mask register field definitions 
*/
#define TCIE                                (1 << 31)       /* Timer/Counter interrupt enable */
#define USBIE                             (1 << 30)       /* USB interrupt enable */
#define PCMCIAIE                             (1 << 29)       /* PCMCIA interrupt enable */
#define UART1IE                             (1 << 28)       /* UART 1 interrupt enable */
#define UART0IE                             (1 << 27)       /* UART 0 interrupt enable */
#define PCIIE                               (1 << 26)       /* Parallel port interrupt enable */
#define SWIE                                (1 << 25)       /* Switch core interrupt enable */
#define PABCIE                              (1 << 24)       /* GPIO port ABC interrupt enable */
#define IREQ0IE                             (1 << 23)       /* External interrupt 0 enable */
#define LBCTMOIE                            (1 << 21)       /* LBC time-out interrupt enable */
#ifdef CONFIG_RTL865XB
#define CRYPTOIE                            (1 << 20)       /* Crypto engine interrupt enable */
#define AUTHIE                              (1 << 19)       /* Authentication engine interupt enable */
#define PCMIE                               (1 << 18)       /* PCM interface interrupt enable */
#define PDEIE                               (1 << 17)       /* GPIO port D,E,F,G,H,I interrupt enable */
#define PCIBTMOIE                           (1 << 16)       /* PCI bridge time-out interrupt enable */
#endif /*CONFIG_RTL865XB*/
/* Global interrupt status register field definitions 
*/
#define TCIP                                 (1 << 31)       /* Timer/Counter interrupt pending */
#define USBIP                               (1 << 30)       /* USB host interrupt pending */
#define PCMCIAIP                          (1 << 29)       /* PCMCIA interrupt pending */
#define UART1IP                            (1 << 28)       /* UART 1 interrupt pending */
#define UART0IP                            (1 << 27)       /* UART 0 interrupt pending */
#define PCIIP                                (1 << 26)       /* PCI host interrupt pending */
#define SWIP                                (1 << 25)       /* Switch core interrupt pending */
#define PABCIP                              (1 << 24)       /* GPIO port ABC interrupt pending */
#define IREQIP                               (1 << 23)       /* External interrupt 0 pending */
#define LBCTMOIP                          (1 << 21)       /* LBC time-out interrupt pending */
#ifdef CONFIG_RTL865XB
#define CRYPTOIP                            (1 << 20)       /* Crypto engine interrupt pending */
#define AUTHIP                              (1 << 19)       /* Authentication engine interupt pending */
#define PCMIP                               (1 << 18)       /* PCM interface interrupt pending */
#define PDEIP                               (1 << 17)       /* GPIO port D,E,F,G,H,I interrupt pending */
#define PCIBTMOIP                           (1 << 16)       /* PCI bridge time-out interrupt pending */
#endif /*CONFIG_RTL865XB*/
/* Interrupt routing register field definitions 
*/
#define TCIRS_OFFSET                       30            /* Timer/Counter interrupt routing select offset */
#define USBIRS_OFFSET                     28            /* USB interrupt routing select offset */
#define PCMCIAIRS_OFFSET                26            /* PCI interrupt routing select offset */
#define UART1IRS_OFFSET                  24            /* UART 1 interrupt routing select offset */
#define UART0IRS_OFFSET                  22            /* UART 0 interrupt routing select offset */
#define PCIIRS_OFFSET                      20            /* USB 1 interrupt routing select offset */
#define SWIRS_OFFSET                      18            /* Switch core interrupt routing select offset */
#define PABCIRS_OFFSET                   16             /* GPIO port B interrupt routing select offset */
#define IREQ0RS_OFFSET                   14             /* External interrupt 0 routing select offset */
#define LBCTMOIRS_OFFSET               10             /* LBC time-out interrupt routing select offset */
#ifdef CONFIG_RTL865XB
#define TCIRS_MSB_OFFSET                9       /* Timer/Counter interrupt routing select MSB offset */
#define USBIRS_MSB_OFFSET               8       /* USB interrupt routing select MSB offset */
#define PCMCIAIRS_MSB_OFFSET            7       /* PCI interrupt routing select MSB offset */
#define UART1IRS_MSB_OFFSET             6       /* UART 1 interrupt routing select MSB offset */
#define UART0IRS_MSB_OFFSET             5       /* UART 0 interrupt routing select MSB offset */
#define PCIIRS_MSB_OFFSET               4       /* USB 1 interrupt routing select MSB offset */
#define SWIRS_MSB_OFFSET                3       /* Switch core interrupt routing select MSB offset */
#define PABCIRS_MSB_OFFSET              2       /* GPIO port B interrupt routing select MSB offset */
#define IREQ0RS_MSB_OFFSET              1       /* External interrupt 0 routing select MSB offset */
#define LBCTMOIRS_MSB_OFFSET            0       /* LBC time-out interrupt routing select MSB offset */
#define CRYPTIRS_OFFSET                 29      /* Crypto engine interrupt routing select offset */
#define AUTHIRS_OFFSET                  26      /* Authentication engine interrupt routing select offset */
#define PCMIRS_OFFSET                   23      /* PCM interface interrupt routing select offset */
#define PDEIRS_OFFSET                   20      /* GPIO port D,E,F,G,H,I interrupt routing select MSB offset */
#define PCIBTMOIRS_OFFSET               17      /* PCI bridge time-out interrupt routing select MSB offset */
#endif /*CONFIG_RTL865XB*/

/* GPIO Register Set */
#define PABCCNR			(0x00C + GICR_BASE)	/* Port A, B, C control register */
#define PABCDIR			(0x010 + GICR_BASE)	/* Port A, B, C direction register */
#define PABCDAT			(0x014 + GICR_BASE)	/* Port A, B, C data register */
#define PABCISR			(0x018 + GICR_BASE)	/* Port A, B, C interrupt status register */
#define PABIMR			(0x01C + GICR_BASE)	/* Port A, B interrupt mask register */
#define PCIMR				(0x050 + GICR_BASE)	/* Port C interrupt mask register */
#define PABCPTCR			(0x054 + GICR_BASE)	/* Port A, B, C peripheral type control register */
#define PDEPTCR			(0x070 + GICR_BASE)	/* Port D, E peripheral type control register */
#define PDEDIR			(0x074 + GICR_BASE)	/* Port D, E direction register */
#define PDEDAT			(0x078 + GICR_BASE)	/* Port D, E data register */
#define PDEISR			(0x07C + GICR_BASE)	/* Port D, E interrupt status register */
#define PDEIMR			(0x080 + GICR_BASE)	/* Port D, E interrupt mask register */
#define PFGHICNR			(0x084 + GICR_BASE)	/* Port F, G, H, I control register */
#define PFGHIDIR			(0x088 + GICR_BASE)	/* Port F, G, H, I direction register */
#define PFGHIDAT			(0x08C + GICR_BASE)	/* Port F, G, H, I data register */
#define PFGHIISR			(0x090 + GICR_BASE)	/* Port F, G, H, I interrupt status register */
#define PFGIMR			(0x094 + GICR_BASE)	/* Port F, G, interrupt status register */
#define PHIIMR			(0x098 + GICR_BASE)	/* Port H, I interrupt status register */






/* Timer control registers 
*/
#define TC0DATA                             (0x020 + GICR_BASE)       /* Timer/Counter 0 data */
#define TC1DATA                             (0x024 + GICR_BASE)       /* Timer/Counter 1 data */
#define TC0CNT                              (0x028 + GICR_BASE)       /* Timer/Counter 0 count */
#define TC1CNT                              (0x02C + GICR_BASE)       /* Timer/Counter 1 count */
#define TCCNR                               (0x030 + GICR_BASE)       /* Timer/Counter control */
#define TCIR                                (0x034 + GICR_BASE)       /* Timer/Counter intertupt */
#define CDBR                                (0x038 + GICR_BASE)       /* Clock division base */
#define WDTCNR                              (0x03C + GICR_BASE)       /* Watchdog timer control */
#define RTCCR                               (0x040 + GICR_BASE)       /* RTC control */
#define RTCDATAL                            (0x048 + GICR_BASE)       /* RTC data low word */
/* Timer/Counter data register field definitions 
*/
#define TCD_OFFSET                          8
/* Timer/Counter control register field defintions 
*/
#define TC0EN                               (1 << 31)       /* Timer/Counter 0 enable */
#define TC0MODE_COUNTER                     0               /* Timer/Counter 0 counter mode */
#define TC0MODE_TIMER                       (1 << 30)       /* Timer/Counter 0 timer mode */
#define TC1EN                               (1 << 29)       /* Timer/Counter 1 enable */
#define TC1MODE_COUNTER                     0               /* Timer/Counter 1 counter mode */
#define TC1MODE_TIMER                       (1 << 28)       /* Timer/Counter 1 timer mode */
/* Timer/Counter interrupt register field definitions 
*/
#define TC0IE                               ((uint32)1 << 31)       /* Timer/Counter 0 interrupt enable */
#define TC1IE                               (1 << 30)       /* Timer/Counter 1 interrupt enable */
#define TC0IP                               (1 << 29)       /* Timer/Counter 0 interrupt pending */
#define TC1IP                               (1 << 28)       /* Timer/Counter 1 interrupt pending */
/* Clock division base register field definitions 
*/
#define DIVF_OFFSET                         16
/* Watchdog control register field definitions 
*/
#define WDTE_OFFSET                         24              /* Watchdog enable */
#define WDSTOP_PATTERN                      0xA5            /* Watchdog stop pattern */
#define WDTCLR                              (1 << 23)       /* Watchdog timer clear */
#define OVSEL_15                            0               /* Overflow select count 2^15 */
#define OVSEL_16                            (1 << 21)       /* Overflow select count 2^16 */
#define OVSEL_17                            (2 << 21)       /* Overflow select count 2^17 */
#define OVSEL_18                            (3 << 21)       /* Overflow select count 2^18 */
#define WDTIND                              (1 << 20)       /* Indicate whether watchdog ever occurs */
/* RTC control register field definitions 
*/
#define WRITE_CDLW_OK                       (1 << 31)       /* Write CDLW OK */
#define RTCSEL                              (1 << 30)       /* Fase mode */
#define RTCEN                               (1 << 29)       /* Enable */




/* GPIO control registers 
*/
#define PABCNR                              (0x00C + GICR_BASE)     /* Port AB control */
#define PABDIR                              (0x010 + GICR_BASE)     /* Port AB direction */
#define PABDAT                              (0x014 + GICR_BASE)     /* Port AB data */
#define PABISR                              (0x018 + GICR_BASE)     /* Port AB interrupt status */
#define PABIMR                              (0x01C + GICR_BASE)     /* Port AB interrupt mask */
#define PCIMR                               (0x050 + GICR_BASE)     /* Port C interrupt mask */
/* Port ABC data register field definitions 
*/
#define PDA_OFFSET                          24              /* Port A data offset */
#define PDB_OFFSET                          16              /* Port B data offset */
#define PDC_OFFSET                          8               /* Port C data offset */
/* Port ABC interrupt status register field definitions 
*/
#define PAIP_OFFSET                         24              /* Port A pending status offset */
#define PBIP_OFFSET                         16              /* Port B pending status offset */
#define PCIP_OFFSET                         8               /* Port C pending status offset */



/* Peripheral Lexra timing control registers 
*/
#define PLTC                                (0x064 + GICR_BASE)     /* Peripheral Lexra timeout control */
/* Peripheral Lexra timeout control register field definitions
*/
#define TOEN                              	  (1 << 31)
#define TOLIMIT_2_7                         (0 << 28)
#define TOLIMIT_2_8                         (1 << 28)
#define TOLIMIT_2_9                         (2 << 28)
#define TOLIMIT_2_10                        (3 << 28)
#define TOLIMIT_2_11                        (4 << 28)
#define TOLIMIT_2_12                        (5 << 28)
#define TOLIMIT_2_13                        (6 << 28)
#define TOLIMIT_2_14                        (7 << 28)

/* Memory Control Register registers 
*/
#define MCR_BASE				(BD01_BASE+0x3000)	/* 0xBD013000 */
#define MCR					(0x000 + MCR_BASE)       /* Memory configuration register */
#define MTCR0					(0x004 + MCR_BASE)       /* Memory timing configuration register */
#define MTCR1					(0x008 + MCR_BASE)       /* Memory timing configuration register */


#define CRYPTO_BASE			 (BD01_BASE+0x5000)	/* 0xBD015000 */

#define DESSCR				(CRYPTO_BASE + 0x00)

#define DES_FETCH				0x00800000

#define DESDCR				(CRYPTO_BASE + 0x0c)
#define DESEIMR				(CRYPTO_BASE + 0x10)
#define DESEISR				(CRYPTO_BASE + 0x14)

#define DES_OPDONE			0x00200000
#define DES_OPALLDONE		0x00080000

#define DES_RUNCIPHER			0x80000000
#define DES_ENCRYPTION		0x08000000
#define DES_ALGO_3DES		0x01000000
#define DES_DESCIV			0x00200000
#define DES_ECB				0x00080000
#define DES_DMA_LEN_MASK		0x00000FFF

#define AUTH_BASE				(BD01_BASE+0x6000)	/* 0xBD016000 */

#define AECR					(AUTH_BASE + 0x00)	//Authentication Engine Control Register
#define ADCR					(AUTH_BASE + 0x04)	//Authentication Descriptor Control Register
#define AEIMR					(AUTH_BASE + 0x08)	//Authenticaiton Engine Interrupt Mask Register
#define AEISR					(AUTH_BASE + 0x0C)	//Authentication Engine Interrupt Status Register

#define AUTH_FETCH			0x00800000
#define AUTH_OPDONE			0x00200000
#define AUTH_OPALLDONE		0x00080000

#define AUTH_SHA1				0x01000000
#define AUTH_HMAC			0x00800000
#define AUTH_HASH_INIT		0x00400000
#define AUTH_HASH_UPD		0x00200000
#define AUTH_HASH_FIN			0x00100000
#define AUTH_DMA_LEN_MASK	0x00000FFF


#endif   /* _ASICREGS_H */

