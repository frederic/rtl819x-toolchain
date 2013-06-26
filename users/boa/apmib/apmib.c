/*
 *      Routines to handle MIB operation
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: apmib.c,v 1.16 2009/09/03 05:04:41 keith_huang Exp $
 *
 */

// include file
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "apmib.h"
#include "mibtbl.h"

/* Shared Memory */
#if CONFIG_APMIB_SHARED_MEMORY == 1
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#endif

// MAC address filtering
typedef struct _filter {
	struct _filter *prev, *next;
	char val[1];
} FILTER_T, *FILTER_Tp;

typedef struct _linkChain {
	FILTER_Tp pUsedList, pFreeList;
	int size, num, usedNum, compareLen, realsize;
	char *buf;
} LINKCHAIN_T, *LINKCHAIN_Tp;


// macro to remove a link list entry
#define REMOVE_LINK_LIST(entry) { \
	if ( entry ) { \
		if ( entry->prev ) \
			entry->prev->next = entry->next; \
		if ( entry->next ) \
			entry->next->prev = entry->prev; \
	} \
}

// macro to add a link list entry
#define ADD_LINK_LIST(list, entry) { \
	if ( list == NULL ) { \
		list = entry; \
		list->prev = list->next = entry; \
	} \
	else { \
		entry->prev = list; \
		entry->next = list->next; \
		list->next = entry; \
		entry->next->prev = entry; \
	} \
}

// local routine declaration
static int flash_read(char *buf, int offset, int len);
static int flash_write(char *buf, int offset, int len);
#ifndef MIB_TLV
static int init_linkchain(LINKCHAIN_Tp pLinkChain, int size, int num);
static int add_linkchain(LINKCHAIN_Tp pLinkChain, char *val);
static int delete_linkchain(LINKCHAIN_Tp pLinkChain, char *val);
static void delete_all_linkchain(LINKCHAIN_Tp pLinkChain);
static int get_linkchain(LINKCHAIN_Tp pLinkChain, char *val, int index);
static int mibtbl_check(const mib_table_entry_T *mib_tbl, int *size);
#endif

#ifdef COMPRESS_MIB_SETTING
unsigned int mib_compress_write(CONFIG_DATA_T type, unsigned char *data);
int mib_updateDef_compress_write(CONFIG_DATA_T type, char *data, PARAM_HEADER_T *pheader);
#endif

#ifdef MIB_TLV
int get_tblentry(void *pmib,unsigned int offset,int num,const mib_table_entry_T *mib_tbl,void *val, int index);
int add_tblentry(void *pmib, unsigned offset, int num,const mib_table_entry_T *mib_tbl,void *val);
int delete_tblentry(void *pmib, unsigned offset, int num,const mib_table_entry_T *mib_tbl,void *val);
int delete_all_tblentry(void *pmib, unsigned int offset, int num,const mib_table_entry_T *mib_tbl);
int Decode(unsigned char *ucInput, unsigned int inLen, unsigned char *ucOutput);
unsigned int mib_tlv_init(const mib_table_entry_T *mib_tbl, unsigned char *from_data, void *pfile, unsigned int tlv_content_len);
unsigned int mib_get_setting_len(CONFIG_DATA_T type);
unsigned int mib_tlv_save(CONFIG_DATA_T type, void *mib_data, unsigned char *mib_tlvfile, unsigned int *tlv_content_len);
void mib_display_data_content(CONFIG_DATA_T type, unsigned char * pdata, unsigned int mib_data_len);
void mib_display_tlv_content(CONFIG_DATA_T type, unsigned char * ptlv, unsigned int mib_tlv_len);
int mib_search_by_id(const mib_table_entry_T *mib_tbl, unsigned short mib_id, unsigned char *pmib_num, const mib_table_entry_T **ppmib, unsigned int *offset);
#endif
mib_table_entry_T* mib_get_table(CONFIG_DATA_T type);
int mib_write_to_raw(const mib_table_entry_T *mib_tbl, void *data, unsigned char *pfile, unsigned int *idx);

// local & global variable declaration
APMIB_Tp pMib=NULL;
APMIB_Tp pMibDef;
PARAM_HEADER_T hsHeader, dsHeader, csHeader;
HW_SETTING_Tp pHwSetting;
int compress_hw_setting = 1;
int wlan_idx=0;	// interface index 
int vwlan_idx=0;	// initially set interface index to root
int wlan_idx_bak=0;
int vwlan_idx_bak=0;

#ifdef MIB_TLV
#else
static LINKCHAIN_T wlanMacChain[NUM_WLAN_INTERFACE][NUM_VWLAN_INTERFACE+1];
static LINKCHAIN_T wdsChain[NUM_WLAN_INTERFACE][NUM_VWLAN_INTERFACE+1];
static LINKCHAIN_T scheduleRuleChain[NUM_WLAN_INTERFACE][NUM_VWLAN_INTERFACE+1];

//### edit by sen_liu 2011.5.13 sync to meshAclChain in apmib_get()
//#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)
static LINKCHAIN_T meshAclChain;
//#endif
//### end

#ifdef HOME_GATEWAY
static LINKCHAIN_T portFwChain, ipFilterChain, portFilterChain, macFilterChain, triggerPortChain;
static LINKCHAIN_T urlFilterChain;
#ifdef ROUTE_SUPPORT
static LINKCHAIN_T staticRouteChain;
#endif

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
static LINKCHAIN_T qosChain;
#endif
#endif
static LINKCHAIN_T dhcpRsvdIpChain;

#if defined(VLAN_CONFIG_SUPPORTED)
static LINKCHAIN_T vlanConfigChain;
#endif
#endif

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
static LINKCHAIN_T  ipsecTunnelChain;
#endif
#endif

#ifdef TLS_CLIENT
static LINKCHAIN_T  certRootChain;
static LINKCHAIN_T  certUserChain;
#endif

#if CONFIG_APMIB_SHARED_MEMORY == 1

#ifdef __mips__
#define FTOK_PATH "/var"
#else
#define FTOK_PATH "flash"
#endif

char *shm_name[] = {	"/var/HWCONF",	/* HWCONF_SHM_KEY */
			"/var/DSCONF",	/* DSCONF_SHM_KEY */
			"/var/CSCONF",	/* CSCONF_SHM_KEY */};
static int apmib_sem_id = -1;

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux specific) */
};
void apmib_save_wlanIdx()
{
	wlan_idx_bak	=	wlan_idx;
	vwlan_idx_bak	=	vwlan_idx;	
}
void apmib_recov_wlanIdx()
{
	wlan_idx	=	wlan_idx_bak;
	vwlan_idx	=	vwlan_idx_bak;
}
static int apmib_sem_create(void)
{
	int sem_id;
	
	/* Generate a System V IPC key */ 
	key_t key;
	key = ftok(FTOK_PATH, 0xD4);
	if (key == -1) {
		printf("APMIB Semaphore ftok() failed !! [%s]\n", strerror(errno));
		return -1;
	}
	
	/* Get a semaphore set with 1 semaphore */
	sem_id = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
	if (sem_id == -1) {
		if (errno == EEXIST) {
			sem_id = semget(key, 1, 0666);
			if (sem_id == -1) {
				printf("APMIB Semaphore semget() failed !! [%s]\n", strerror(errno));
				return -1;
			}
		} else {
			printf("APMIB Semaphore semget() failed !! [%s]\n", strerror(errno));
			return -1;
		}
	} else {
		/* Initialize semaphore #0 to 1 */
		union semun arg;
		
		arg.val = 1;
		if (semctl(sem_id, 0, SETVAL, arg) == -1) {
			printf("APMIB Semaphore semctl() failed !! [%s]\n", strerror(errno));
			return -1;
		}
	}

	apmib_sem_id = sem_id;
	return 0;
}

int apmib_sem_lock(void)
{
	struct sembuf sop[1];
	
	if (apmib_sem_id == -1) {
		if (apmib_sem_create() == -1) {
		    printf("apmib_sem_create fail\n");
			return -1;
		}
	}
	
	sop[0].sem_num = 0;
	sop[0].sem_op = -1;
	sop[0].sem_flg = SEM_UNDO;
try_again:
	if (semop(apmib_sem_id, sop, 1) == -1) {
		if (errno == EINTR) {
//			printf("APMIB Semaphore Lock semop() failed !! [%s]\n", strerror(errno));
			goto try_again;
		}
		printf("APMIB Semaphore Lock semop() failed !! [%s]\n", strerror(errno));
		return -1;
	}
	return 0;		
}

int apmib_sem_unlock(void)
{
	struct sembuf sop[1];
		
	sop[0].sem_num = 0;
	sop[0].sem_op = 1;
	sop[0].sem_flg = SEM_UNDO;
	if (semop(apmib_sem_id, sop, 1) == -1) {
		printf("APMIB Semaphore Unlock semop() failed !! [%s]\n", strerror(errno));
		return -1;
	}	
	return 0;	
}
	
int apmib_shm_free(void *shm_memory, int shm_key)
{
	return (shmdt(shm_memory));
}

static char *apmib_shm_calloc(size_t nmemb, size_t size, int shm_key, int *created)
{
	int shm_id, shm_size;
	char *shm_memory; //, *shm_name;	
	
	*created = 0;
	shm_size = (nmemb * size);

	/* Generate a System V IPC key */ 
	key_t key;
	key = ftok(FTOK_PATH, (0x3C + shm_key));
	if (key == -1) {
		printf("%s ftok() failed !! [%s]\n", shm_name[shm_key], strerror(errno));
		return NULL;
	}
		 
	/* Allocate a shared memory segment */
	shm_id = shmget(key, shm_size, IPC_CREAT | IPC_EXCL | 0666);
	if (shm_id == -1) {
		if (errno == EEXIST) {
			*created = 1;
			shm_id = shmget(key, shm_size, 0666);
			if (shm_id == -1) {
				printf("%s shmget() failed !! [%s]\n", shm_name[shm_key], strerror(errno));
				return NULL;
			}
		} else {
			printf("%s shmget() failed !! [%s]\n", shm_name[shm_key], strerror(errno));
			return NULL;
		}
	}
		 
	/* Attach the shared memory segment */
	shm_memory = (char *)shmat(shm_id, NULL, 0);
	if ((int)shm_memory == -1) {
		printf("%s shmat() failed [%s]\n", shm_name[shm_key], strerror(errno));
		return NULL;
	}
		
	if (*created) {
		return shm_memory;
	}

	memset(shm_memory, 0, shm_size);
	return shm_memory;
}
#endif

////////////////////////////////////////////////////////////////////////////////
char *apmib_hwconf(void)
{
	int ver;
	char *buff;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	int created;
#endif

	char hw_setting_start[6];
	
#if 1		
	if ( flash_read(hw_setting_start, HW_SETTING_OFFSET, 6)==0 ) {
			printf("Read hw setting header failed!\n");					
	}
	else 
	{
		//printf("===,the start char from HW_SETTING_OFFSET is %c%c%c%c%c%c\n",
		//		hw_setting_start[0], hw_setting_start[1], hw_setting_start[2], 
		//		hw_setting_start[3], hw_setting_start[4], hw_setting_start[5]);
		
		if (!memcmp(hw_setting_start, HW_SETTING_HEADER_TAG, TAG_LEN) ||
			!memcmp(hw_setting_start, HW_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
			!memcmp(hw_setting_start, HW_SETTING_HEADER_UPGRADE_TAG, TAG_LEN))
		{
			compress_hw_setting = 0;
		}
		else
		{
			compress_hw_setting = 1;
		}
	}
#endif

	
#if defined(COMPRESS_MIB_SETTING) 		
	int zipRate=0;
	unsigned char *compFile=NULL, *expFile=NULL;
	unsigned int expandLen=0, compLen=0;
	COMPRESS_MIB_HEADER_T compHeader;
#endif



#if defined(COMPRESS_MIB_SETTING)
	if(compress_hw_setting)
	{
	flash_read((char *)&compHeader, HW_SETTING_OFFSET, sizeof(COMPRESS_MIB_HEADER_T));
	if(memcmp(compHeader.signature, COMP_HS_SIGNATURE, COMP_SIGNATURE_LEN) == 0 ) //check whether compress mib data
	{
		zipRate = compHeader.compRate;
		compLen = compHeader.compLen;
		if ( (compLen > 0) && (compLen <= HW_SETTING_SECTOR_LEN) ) {
			compFile=malloc(compLen+sizeof(COMPRESS_MIB_HEADER_T));
			if(compFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);
				return 0;
			}
			expFile=malloc(zipRate*compLen);
			if(expFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);			
				free(compFile);
				return 0;
			}

			flash_read((char *)compFile, HW_SETTING_OFFSET, compLen+sizeof(COMPRESS_MIB_HEADER_T));

			expandLen = Decode(compFile+sizeof(COMPRESS_MIB_HEADER_T), compLen, expFile);
			memcpy((char *)&hsHeader, expFile, sizeof(hsHeader));
		} 
		else 
		{
			COMP_TRACE(stderr,"\r\n ERR:compLen = %u __[%s-%u]",compLen, __FILE__,__LINE__);
			return 0;
		}

#if defined(MIB_TLV)
		mib_table_entry_T *pmib_tl = NULL;
		unsigned char *hwMibData;
		unsigned int tlv_content_len = hsHeader.len - 1; // last one is checksum

		hwMibData = malloc(sizeof(HW_SETTING_T)+1); // 1: checksum

		if(hwMibData != NULL)
			memset(hwMibData, 0x00, sizeof(HW_SETTING_T)+1);
	
		pmib_tl = mib_get_table(HW_SETTING);
		unsigned int tlv_checksum = 0;

		if(expFile != NULL)
			tlv_checksum = CHECKSUM_OK(expFile+sizeof(PARAM_HEADER_T), hsHeader.len);

//mib_display_tlv_content(HW_SETTING, expFile+sizeof(PARAM_HEADER_T), hsHeader.len);

		if(tlv_checksum == 1 && mib_tlv_init(pmib_tl, expFile+sizeof(PARAM_HEADER_T), (void*)hwMibData, tlv_content_len) == 1) /* According to pmib_tl, get value from expFile to hwMibData. parse total len is  tlv_content_len*/
		{
			sprintf((char *)&hsHeader.signature[TAG_LEN], "%02d", HW_SETTING_VER);
			hsHeader.len = sizeof(HW_SETTING_T)+1;
			hwMibData[hsHeader.len-1]  = CHECKSUM(hwMibData, hsHeader.len-1);

			if(expFile!= NULL)
				free(expFile);

			expFile = malloc(sizeof(PARAM_HEADER_T)+hsHeader.len);
			memcpy(expFile, &hsHeader, sizeof(PARAM_HEADER_T));
			memcpy(expFile+sizeof(PARAM_HEADER_T), hwMibData, hsHeader.len);

//mib_display_data_content(HW_SETTING, expFile+sizeof(PARAM_HEADER_T), hsHeader.len-1);

			
		}	

		if(hwMibData != NULL)
			free(hwMibData);
		
#endif // #ifdef MIB_TLV

	}
	}
	else // not compress mib-data, get mib header from flash
#endif //#ifdef COMPRESS_MIB_SETTING
	// Read hw setting
{
	if ( flash_read((char *)&hsHeader, HW_SETTING_OFFSET, sizeof(hsHeader))==0 ) {
//		printf("Read hw setting header failed!\n");
		return NULL;
	}
}
	if ( sscanf((char *)&hsHeader.signature[TAG_LEN], "%02d", &ver) != 1)
		ver = -1;

//fprintf(stderr,"\r\n ver = %u",ver);

	if ( memcmp(hsHeader.signature, HW_SETTING_HEADER_TAG, TAG_LEN)) // invalid signatur
	{
		printf("Invalid hw setting signature [sig=%c%c]!\n", hsHeader.signature[0],hsHeader.signature[1]);
#if defined(COMPRESS_MIB_SETTING)
	if(compress_hw_setting)
	{

		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
	}
#endif
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
		return NULL;
	}

	if ( (ver != HW_SETTING_VER) || // version not equal to current
		(hsHeader.len < (sizeof(HW_SETTING_T)+1)) ) { // length is less than current
		printf("Invalid hw setting version number or data length[ver=%d, len=%d]!\n", ver, hsHeader.len);
#if defined(COMPRESS_MIB_SETTING)
	if(compress_hw_setting)
	{

		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
	}
#endif
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
		return NULL;
	}
//	if (ver > HW_SETTING_VER)
//		printf("HW setting version is greater than current [f:%d, c:%d]!\n", ver, HW_SETTING_VER);

#if CONFIG_APMIB_SHARED_MEMORY == 1
	buff = apmib_shm_calloc(1, hsHeader.len, HWCONF_SHM_KEY, &created);
#else
	buff = calloc(1, hsHeader.len);
#endif
	if ( buff == 0 ) {
//		printf("Allocate buffer failed!\n");
#if defined(COMPRESS_MIB_SETTING)
	if(compress_hw_setting)
	{
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
	}
#endif
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
		return NULL;
	}

#if CONFIG_APMIB_SHARED_MEMORY == 1
    if (created) {
        //printf("No need to read hw setting!\n");
#if defined(COMPRESS_MIB_SETTING)
	if(compress_hw_setting)
	{
	if(compFile != NULL)
		free(compFile);
	if(expFile != NULL)
		free(expFile);
	}
#endif        
        return buff;
    }
#endif

#if defined(COMPRESS_MIB_SETTING)
	if(compress_hw_setting)
	{
	if(expFile != NULL) //compress mib data, copy the mibdata body from expFile
	{
		memcpy(buff, expFile+sizeof(PARAM_HEADER_T), hsHeader.len);
		free(expFile);
		free(compFile);
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);		
	}
	}
	else // not compress mib data, copy mibdata from flash
#endif
	if ( flash_read(buff, HW_SETTING_OFFSET+sizeof(hsHeader), hsHeader.len)==0 ) {
//		printf("Read hw setting failed!\n");
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(buff, HWCONF_SHM_KEY);
#else
		free(buff);
#endif		
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
		return NULL;
	}
	if ( !CHECKSUM_OK((unsigned char *)buff, hsHeader.len) ) {
//		printf("Invalid checksum of hw setting!\n");
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(buff, HWCONF_SHM_KEY);
#else
		free(buff);
#endif		
		return NULL;
	}
	return buff;
}

////////////////////////////////////////////////////////////////////////////////
char *apmib_dsconf(void)
{
	int ver;
	char *buff;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	int created;
#endif

#ifdef COMPRESS_MIB_SETTING
	int zipRate=0;
	unsigned char *compFile=NULL, *expFile=NULL;
	unsigned int expandLen=0, compLen=0;
	COMPRESS_MIB_HEADER_T compHeader;
#endif

#ifdef COMPRESS_MIB_SETTING

//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
	flash_read((char *)&compHeader, DEFAULT_SETTING_OFFSET, sizeof(COMPRESS_MIB_HEADER_T));
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);	
	if(memcmp(compHeader.signature, COMP_DS_SIGNATURE, COMP_SIGNATURE_LEN) == 0 ) //check whether compress mib data
	{
		zipRate = compHeader.compRate;
		compLen = compHeader.compLen;
		if ( (compLen > 0) && (compLen <= DEFAULT_SETTING_SECTOR_LEN) ) {
			compFile=malloc(compLen+sizeof(COMPRESS_MIB_HEADER_T));
			if(compFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);
				return 0;
			}
			expFile=malloc(zipRate*compLen);
			if(expFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);			
				free(compFile);
				return 0;
			}

			flash_read((char *)compFile, DEFAULT_SETTING_OFFSET, compLen+sizeof(COMPRESS_MIB_HEADER_T));

			expandLen = Decode(compFile+sizeof(COMPRESS_MIB_HEADER_T), compLen, expFile);
			//printf("expandLen read len: %d\n", expandLen);

			// copy the mib header from expFile
			memcpy((char *)&dsHeader, expFile, sizeof(dsHeader));
			
		} 
		else 
		{
			COMP_TRACE(stderr,"\r\n ERR:compLen = %u __[%s-%u]",compLen, __FILE__,__LINE__);
			return 0;
		}
#ifdef MIB_TLV
		mib_table_entry_T *pmib_tl = NULL;
		unsigned char *defMibData;
		unsigned int tlv_content_len = dsHeader.len - 1; // last one is checksum

		defMibData = malloc(sizeof(APMIB_T)+1); // 1: checksum

		if(defMibData != NULL)
			memset(defMibData, 0x00, sizeof(APMIB_T)+1);
	
		pmib_tl = mib_get_table(DEFAULT_SETTING);
		unsigned int tlv_checksum = 0;

		if(expFile != NULL)
			tlv_checksum = CHECKSUM_OK(expFile+sizeof(PARAM_HEADER_T), dsHeader.len);

//mib_display_tlv_content(DEFAULT_SETTING, expFile+sizeof(PARAM_HEADER_T), dsHeader.len);
		
		if(tlv_checksum == 1 && mib_tlv_init(pmib_tl, expFile+sizeof(PARAM_HEADER_T), (void*)defMibData, tlv_content_len) == 1) /* According to pmib_tl, get value from expFile to hwMibData. parse total len is  tlv_content_len*/
		{
			sprintf((char *)&dsHeader.signature[TAG_LEN], "%02d", DEFAULT_SETTING_VER);
			dsHeader.len = sizeof(APMIB_T)+1;
			defMibData[dsHeader.len-1]  = CHECKSUM(defMibData, sizeof(APMIB_T));

			if(expFile!= NULL)
				free(expFile);

			expFile = malloc(sizeof(PARAM_HEADER_T)+dsHeader.len);
			memcpy(expFile, &dsHeader, sizeof(PARAM_HEADER_T));
			memcpy(expFile+sizeof(PARAM_HEADER_T), defMibData, dsHeader.len);
			
//mib_display_data_content(DEFAULT_SETTING, expFile+sizeof(PARAM_HEADER_T), dsHeader.len-1);
		}	
		else
		{
			COMP_TRACE(stderr,"\r\n ERR!Invalid checksum[%u] or mib_tlv_init() fail! __[%s-%u]",tlv_checksum,__FILE__,__LINE__);
		}
		
		if(defMibData != NULL)
			free(defMibData);
		
#endif // #ifdef MIB_TLV		
	}
	else // not compress mib-data, get mib header from flash
#endif //#ifdef COMPRESS_MIB_SETTING
	// Read default s/w mib
	if ( flash_read((char *)&dsHeader, DEFAULT_SETTING_OFFSET, sizeof(dsHeader))==0 ) {
//		printf("Read default setting header failed!\n");
		return NULL;
	}

	if ( sscanf((char *)&dsHeader.signature[TAG_LEN], "%02d", &ver) != 1)
		ver = -1;

//fprintf(stderr,"\r\n (sizeof(APMIB_T)=%u ",(sizeof(APMIB_T)));
//printf("default setting signature or version number [sig=%c%c, ver=%d, len=%d]!\n",	dsHeader.signature[0], dsHeader.signature[1], ver, dsHeader.len);
	if ( memcmp(dsHeader.signature, DEFAULT_SETTING_HEADER_TAG, TAG_LEN) || // invalid signatur
		(ver != DEFAULT_SETTING_VER) || // version not equal to current
		(dsHeader.len < (sizeof(APMIB_T)+1)) ) { // length is less than current
		printf("Invalid default setting signature or version number [sig=%c%c, ver=%d, len=%d]!\n",	dsHeader.signature[0], dsHeader.signature[1], ver, dsHeader.len);
		printf("Expect [sig=%s, ver=%d, len=%d]!\n", DEFAULT_SETTING_HEADER_TAG, DEFAULT_SETTING_VER, sizeof(APMIB_T)+1);
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
		return NULL;
	}
//	if (ver > DEFAULT_SETTING_VER)
//		printf("Default setting version is greater than current [f:%d, c:%d]!\n", ver, DEFAULT_SETTING_VER);

#if CONFIG_APMIB_SHARED_MEMORY == 1
	buff = apmib_shm_calloc(1, dsHeader.len, DSCONF_SHM_KEY, &created);
#else
	buff = calloc(1, dsHeader.len);
#endif	
	if ( buff == 0 ) {
//		printf("Allocate buffer failed!\n");
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
		return NULL;
	}

#if CONFIG_APMIB_SHARED_MEMORY == 1
    if (created) {
        //printf("No need to read default setting!\n");
#ifdef COMPRESS_MIB_SETTING
	if(compFile != NULL)
		free(compFile);
	if(expFile != NULL)
		free(expFile);
#endif        
        return buff;
    }
#endif

#ifdef COMPRESS_MIB_SETTING

	if(expFile != NULL) //compress mib data, copy the mibdata body from expFile
	{
		memcpy(buff, expFile+sizeof(PARAM_HEADER_T), dsHeader.len);
		free(expFile);
		free(compFile);
	}
	else // not compress mib data, copy mibdata from flash
#endif
	if ( flash_read(buff, DEFAULT_SETTING_OFFSET+sizeof(dsHeader), dsHeader.len)==0 ) {
//		printf("Read default setting failed!\n");
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(buff, DSCONF_SHM_KEY);
#else
		free(buff);
#endif		
		return NULL;
	}

	if ( !CHECKSUM_OK((unsigned char *)buff, dsHeader.len) ) {
//		printf("Invalid checksum of current setting!\n");
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(buff, DSCONF_SHM_KEY);
#else
		free(buff);
#endif		
		return NULL;
	}
	return buff;
}

////////////////////////////////////////////////////////////////////////////////

#ifdef COMPRESS_MIB_SETTING
int flash_read_raw_mib(unsigned char **compFile)
{
	int zipRate=0, i = 0;
	unsigned int compLen=0;
	COMPRESS_MIB_HEADER_T compHeader;
	void *pDst = (void*)(CURRENT_SETTING_OFFSET);

	if (compFile && *compFile)
		free(*compFile);
	else
		return 0;

	flash_read(&compHeader, pDst, sizeof(COMPRESS_MIB_HEADER_T));
	if(memcmp(compHeader.signature, COMP_CS_SIGNATURE, COMP_SIGNATURE_LEN) == 0 ) //check whether compress mib data
	{
		zipRate = compHeader.compRate;
		compLen = compHeader.compLen;
		if ( (compLen > 0) && (compLen <= CURRENT_SETTING_SECTOR_LEN) ) {
			*compFile=calloc(1,compLen+sizeof(COMPRESS_MIB_HEADER_T));
			if(*compFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);
				return 0;
			}
			flash_read(*compFile, pDst, compLen+sizeof(COMPRESS_MIB_HEADER_T));		
		}
		else 
		{
			COMP_TRACE(stderr,"\r\n ERR:compLen = %u __[%s-%u]",compLen, __FILE__,__LINE__);
			return 0;
		}
	}

	return 1;
}

int flash_write_raw_mib(unsigned char **compFile)
{
	COMPRESS_MIB_HEADER_T *compHeader = (COMPRESS_MIB_HEADER_T *)(*compFile);
	void *pDst = (void*)(CURRENT_SETTING_OFFSET);
	int ret = 1;

	if ( flash_write(*compFile, pDst, compHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T))==0 )
	{
		printf("Write flash compress setting failed![%s-%u]\n",__FILE__,__LINE__);			
		ret = 0;
	}

	if (compFile && *compFile)
		free(*compFile);

	return ret;
}
#else
int flash_read_raw_mib(unsigned char **compFile)
{
	return 0;
}

int flash_write_raw_mib(unsigned char **compFile)
{
	return 0;
}
#endif /* #ifdef COMPRESS_MIB_SETTING */

char *apmib_csconf(void)
{
	int ver;
	char *buff;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	int created;
#endif

#ifdef COMPRESS_MIB_SETTING
	int zipRate=0;
	unsigned char *compFile=NULL, *expFile=NULL;
	unsigned int expandLen=0, compLen=0;
	COMPRESS_MIB_HEADER_T compHeader;
#endif

#ifdef COMPRESS_MIB_SETTING

//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
	flash_read((char *)&compHeader, CURRENT_SETTING_OFFSET, sizeof(COMPRESS_MIB_HEADER_T));
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);	
	if(memcmp(compHeader.signature, COMP_CS_SIGNATURE, COMP_SIGNATURE_LEN) == 0 ) //check whether compress mib data
	{
		zipRate = compHeader.compRate;
		compLen = compHeader.compLen;
		if ( (compLen > 0) && (compLen <= CURRENT_SETTING_SECTOR_LEN) ) {
			compFile=calloc(1,compLen+sizeof(COMPRESS_MIB_HEADER_T));
			if(compFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);
				return 0;
			}
			expFile=calloc(1,zipRate*compLen);
			if(expFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);			
				free(compFile);
				return 0;
			}

			flash_read((char *)compFile, CURRENT_SETTING_OFFSET, compLen+sizeof(COMPRESS_MIB_HEADER_T));

			expandLen = Decode(compFile+sizeof(COMPRESS_MIB_HEADER_T), compLen, expFile);
			//printf("expandLen read len: %d\n", expandLen);

			// copy the mib header from expFile
			memcpy((char *)&csHeader, expFile, sizeof(csHeader));
			
		} 
		else 
		{
			COMP_TRACE(stderr,"\r\n ERR:compLen = %u __[%s-%u]",compLen, __FILE__,__LINE__);
			return 0;
		}
#ifdef MIB_TLV
		mib_table_entry_T *pmib_tl = NULL;
		unsigned char *curMibData;
		unsigned int tlv_content_len = csHeader.len - 1; // last one is checksum

		curMibData = malloc(sizeof(APMIB_T)+1); // 1: checksum

		if(curMibData != NULL)
			memset(curMibData, 0x00, sizeof(APMIB_T)+1);
	
		pmib_tl = mib_get_table(CURRENT_SETTING);
		unsigned int tlv_checksum = 0;

		if(expFile != NULL)
			tlv_checksum = CHECKSUM_OK(expFile+sizeof(PARAM_HEADER_T), csHeader.len);

//mib_display_tlv_content(CURRENT_SETTING, expFile+sizeof(PARAM_HEADER_T), csHeader.len);

		if(tlv_checksum == 1 && mib_tlv_init(pmib_tl, expFile+sizeof(PARAM_HEADER_T), (void*)curMibData, tlv_content_len) == 1) /* According to pmib_tl, get value from expFile to hwMibData. parse total len is  tlv_content_len*/
		{
			sprintf((char *)&csHeader.signature[TAG_LEN], "%02d", CURRENT_SETTING_VER);
			csHeader.len = sizeof(APMIB_T)+1;
			curMibData[csHeader.len-1]  = CHECKSUM(curMibData, csHeader.len-1);

			if(expFile!= NULL)
				free(expFile);

			expFile = malloc(sizeof(PARAM_HEADER_T)+csHeader.len);
			memcpy(expFile, &csHeader, sizeof(PARAM_HEADER_T));
			memcpy(expFile+sizeof(PARAM_HEADER_T), curMibData, csHeader.len);
			
//mib_display_data_content(CURRENT_SETTING, expFile+sizeof(PARAM_HEADER_T), csHeader.len-1);
		}	

		if(curMibData != NULL)
			free(curMibData);
		
#endif // #ifdef MIB_TLV		
	}
	else // not compress mib-data, get mib header from flash
#endif //#ifdef COMPRESS_MIB_SETTING
	// Read current s/w mib
	if ( flash_read((char *)&csHeader, CURRENT_SETTING_OFFSET, sizeof(csHeader))==0 ) {
//		printf("Read current setting header failed!\n");
		return NULL;
	}

//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);

	if ( sscanf((char *)&csHeader.signature[TAG_LEN], "%02d", &ver) != 1)
		ver = -1;

	if ( memcmp(csHeader.signature, CURRENT_SETTING_HEADER_TAG, TAG_LEN) || // invalid signatur
		(ver != CURRENT_SETTING_VER) || // version not equal to current
			(csHeader.len < (sizeof(APMIB_T)+1)) ) { // length is less than current
//		printf("Invalid current setting signature or version number [sig=%c%c, ver=%d, len=%d]!\n",
//			csHeader.signature[0], csHeader.signature[1], ver, csHeader.len);
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
		return NULL;
	}

//	if (ver > CURRENT_SETTING_VER)
//		printf("Current setting version is greater than current [f:%d, c:%d]!\n", ver, CURRENT_SETTING_VER);

#if CONFIG_APMIB_SHARED_MEMORY == 1
	buff = apmib_shm_calloc(1, csHeader.len, CSCONF_SHM_KEY, &created);
#else
	buff = calloc(1, csHeader.len);
#endif	
	if ( buff == 0 ) {
//		printf("Allocate buffer failed!\n");
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
		return NULL;
	}

#if CONFIG_APMIB_SHARED_MEMORY == 1
    if (created) {
        //printf("No need to read current setting!\n");
#ifdef COMPRESS_MIB_SETTING
	if(compFile != NULL)
		free(compFile);
	if(expFile != NULL)
		free(expFile);
#endif        
        return buff;
    }
#endif

#ifdef COMPRESS_MIB_SETTING

	if(expFile != NULL) //compress mib data, copy the mibdata body from expFile
	{
		memcpy(buff, expFile+sizeof(PARAM_HEADER_T), csHeader.len);
		free(expFile);
		free(compFile);
	}
	else // not compress mib data, copy mibdata from flash
#endif
	if ( flash_read(buff, CURRENT_SETTING_OFFSET+sizeof(csHeader), csHeader.len)==0 ) {
//		printf("Read current setting failed!\n");
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(buff, CSCONF_SHM_KEY);
#else
		free(buff);
#endif		
		return NULL;
	}

	if ( !CHECKSUM_OK((unsigned char *)buff, csHeader.len) ) {
//		printf("Invalid checksum of current setting!\n");
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(buff, CSCONF_SHM_KEY);
#else	
		free(buff);
#endif
		return NULL;
	}

	return buff;
}
////////////////////////////////////////////////////////////////////////////
int apmib_init_HW(void)
{
	char *buff;

	if ((buff=apmib_hwconf()) == NULL)
		return 0;
	pHwSetting = (HW_SETTING_Tp)buff;
	return 1;
}
////////////////////////////////////////////////////////////////////////////////
int apmib_init(void)
{
#ifndef MIB_TLV
	int i, j, k;
#endif
	char *buff;

#if CONFIG_APMIB_SHARED_MEMORY == 1	
    apmib_sem_lock();
#endif

	if ( pMib != NULL )	// has been initialized
#if CONFIG_APMIB_SHARED_MEMORY == 1
		goto linkchain;
#else
		return 1;
#endif

	if ((buff=apmib_hwconf()) == NULL) {
#if CONFIG_APMIB_SHARED_MEMORY == 1	
        apmib_sem_unlock();
#endif
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
		return 0;
	}
	pHwSetting = (HW_SETTING_Tp)buff;

	if ((buff=apmib_dsconf()) == NULL) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
		apmib_sem_unlock();
#else
		free(pHwSetting);
#endif
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
		return 0;
	}
	pMibDef = (APMIB_Tp)buff;

	if ((buff=apmib_csconf()) == NULL) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
		apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
		apmib_sem_unlock();
#else
		free(pHwSetting);
		free(pMibDef);
#endif
		return 0;
	}
	pMib = (APMIB_Tp)buff;

#if CONFIG_APMIB_SHARED_MEMORY == 1
linkchain:
#endif
#ifndef MIB_TLV
	for (j=0; j<NUM_WLAN_INTERFACE; j++)
		for (k=0; k<(NUM_VWLAN_INTERFACE+1); k++) // wlan[j][0] is for root
	{
		// initialize MAC access control list
		if ( !init_linkchain(&wlanMacChain[j][k], sizeof(MACFILTER_T), MAX_WLAN_AC_NUM)) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_shm_free(pMib, CSCONF_SHM_KEY);
			apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
			apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
			apmib_sem_unlock();
#else
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
#endif			
			return 0;
		}
		
		for (i=0; i<pMib->wlan[j][k].acNum; i++) {
			if ( !add_linkchain(&wlanMacChain[j][k], (char *)&pMib->wlan[j][k].acAddrArray[i]) ) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
				apmib_shm_free(pMib, CSCONF_SHM_KEY);
				apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
				apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
				apmib_sem_unlock();
#else
				free(pMib);
				free(pMibDef);
				free(pHwSetting);
#endif
				return 0;
			}
		}
		wlanMacChain[j][k].compareLen = sizeof(MACFILTER_T) - COMMENT_LEN;

		// initialize WDS list
		if ( !init_linkchain(&wdsChain[j][k], sizeof(WDS_T), MAX_WDS_NUM)) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_shm_free(pMib, CSCONF_SHM_KEY);
			apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
			apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
			apmib_sem_unlock();
#else
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
#endif
			return 0;
		}
		for (i=0; i<pMib->wlan[j][k].wdsNum; i++) {
			if ( !add_linkchain(&wdsChain[j][k], (char *)&pMib->wlan[j][k].wdsArray[i]) ) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
				apmib_shm_free(pMib, CSCONF_SHM_KEY);
				apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
				apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
				apmib_sem_unlock();
#else
				free(pMib);
				free(pMibDef);
				free(pHwSetting);
#endif
				return 0;
			}
		}
		wdsChain[j][k].compareLen = sizeof(WDS_T) - COMMENT_LEN;

		
		// initialize schedule table
		if ( !init_linkchain(&scheduleRuleChain[j][k], sizeof(SCHEDULE_T), MAX_SCHEDULE_NUM)) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_shm_free(pMib, CSCONF_SHM_KEY);
			apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
			apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
#else
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
#endif
			return 0;
		}
		for (i=0; i<pMib->wlan[j][k].scheduleRuleNum; i++) {
			if ( !add_linkchain(&scheduleRuleChain[j][k], (char *)&pMib->wlan[j][k].scheduleRuleArray[i]) ) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
				apmib_shm_free(pMib, CSCONF_SHM_KEY);
				apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
				apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
#else
				free(pMib);
				free(pMibDef);
				free(pHwSetting);
#endif
				return 0;
			}
		}
			scheduleRuleChain[j][k].compareLen = sizeof(SCHEDULE_T);
			
		
	}

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)	// below code copy above ACL code
	// initialize MAC access control list
	if ( !init_linkchain(&meshAclChain, sizeof(MACFILTER_T), MAX_MESH_ACL_NUM)) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(pMib, CSCONF_SHM_KEY);
		apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
		apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
		apmib_sem_unlock();
#else
		free(pMib);
		free(pMibDef);
		free(pHwSetting);
#endif			
		return 0;
	}

	for (i=0; i<pMib->meshAclNum; i++) {
		if ( !add_linkchain(&meshAclChain, (char *)&pMib->meshAclAddrArray[i]) ) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_shm_free(pMib, CSCONF_SHM_KEY);
			apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
			apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
			apmib_sem_unlock();
#else
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
#endif
			return 0;
		}
	}
	meshAclChain.compareLen = sizeof(MACFILTER_T) - COMMENT_LEN;
#endif	// CONFIG_RTK_MESH && _MESH_ACL_ENABLE_

	




#ifdef HOME_GATEWAY
	// initialize port forwarding table
	if ( !init_linkchain(&portFwChain, sizeof(PORTFW_T), MAX_FILTER_NUM)) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(pMib, CSCONF_SHM_KEY);
		apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
		apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
		apmib_sem_unlock();
#else
		free(pMib);
		free(pMibDef);
		free(pHwSetting);
#endif
		return 0;
	}
	for (i=0; i<pMib->portFwNum; i++) {
		if ( !add_linkchain(&portFwChain, (char *)&pMib->portFwArray[i]) ) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_shm_free(pMib, CSCONF_SHM_KEY);
			apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
			apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
			apmib_sem_unlock();
#else
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
#endif
			return 0;
		}
	}
	portFwChain.compareLen = sizeof(PORTFW_T) - COMMENT_LEN;

	// initialize ip-filter table
	if ( !init_linkchain(&ipFilterChain, sizeof(IPFILTER_T), MAX_FILTER_NUM)) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(pMib, CSCONF_SHM_KEY);
		apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
		apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
		apmib_sem_unlock();
#else
		free(pMib);
		free(pMibDef);
		free(pHwSetting);
#endif
		return 0;
	}
	for (i=0; i<pMib->ipFilterNum; i++) {
		if ( !add_linkchain(&ipFilterChain, (char *)&pMib->ipFilterArray[i]) ) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_shm_free(pMib, CSCONF_SHM_KEY);
			apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
			apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
			apmib_sem_unlock();
#else
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
#endif
			return 0;
		}
	}
	ipFilterChain.compareLen = sizeof(IPFILTER_T) - COMMENT_LEN;

	// initialize port-filter table
	if ( !init_linkchain(&portFilterChain, sizeof(PORTFILTER_T), MAX_FILTER_NUM)) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(pMib, CSCONF_SHM_KEY);
		apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
		apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
		apmib_sem_unlock();
#else
		free(pMib);
		free(pMibDef);
		free(pHwSetting);
#endif
		return 0;
	}
	for (i=0; i<pMib->portFilterNum; i++) {
		if ( !add_linkchain(&portFilterChain, (char *)&pMib->portFilterArray[i]) ) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_shm_free(pMib, CSCONF_SHM_KEY);
			apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
			apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
			apmib_sem_unlock();
#else
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
#endif
			return 0;
		}
	}
	portFilterChain.compareLen = sizeof(PORTFILTER_T) - COMMENT_LEN;

	// initialize mac-filter table
	if ( !init_linkchain(&macFilterChain, sizeof(MACFILTER_T), MAX_FILTER_NUM)) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(pMib, CSCONF_SHM_KEY);
		apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
		apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
		apmib_sem_unlock();
#else
		free(pMib);
		free(pMibDef);
		free(pHwSetting);
#endif
		return 0;
	}
	for (i=0; i<pMib->macFilterNum; i++) {
		if ( !add_linkchain(&macFilterChain, (char *)&pMib->macFilterArray[i]) ) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_shm_free(pMib, CSCONF_SHM_KEY);
			apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
			apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
			apmib_sem_unlock();
#else
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
#endif
			return 0;
		}
	}
	macFilterChain.compareLen = sizeof(MACFILTER_T) - COMMENT_LEN;

	// initialize url-filter table
	if ( !init_linkchain(&urlFilterChain, sizeof(URLFILTER_T), MAX_URLFILTER_NUM)) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(pMib, CSCONF_SHM_KEY);
		apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
		apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
		apmib_sem_unlock();
#else
		free(pMib);
		free(pMibDef);
		free(pHwSetting);
#endif
		return 0;
	}
	for (i=0; i<pMib->urlFilterNum; i++) {
		if ( !add_linkchain(&urlFilterChain, (char *)&pMib->urlFilterArray[i]) ) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_shm_free(pMib, CSCONF_SHM_KEY);
			apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
			apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
			apmib_sem_unlock();
#else
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
#endif
			return 0;
		}
	}
	urlFilterChain.compareLen = sizeof(URLFILTER_T);// - COMMENT_LEN;

	// initialize trigger-port table
	if ( !init_linkchain(&triggerPortChain, sizeof(TRIGGERPORT_T), MAX_FILTER_NUM)) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(pMib, CSCONF_SHM_KEY);
		apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
		apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
		apmib_sem_unlock();
#else
		free(pMib);
		free(pMibDef);
		free(pHwSetting);
#endif
		return 0;
	}
	for (i=0; i<pMib->triggerPortNum; i++) {
		if ( !add_linkchain(&triggerPortChain, (char *)&pMib->triggerPortArray[i]) ) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_shm_free(pMib, CSCONF_SHM_KEY);
			apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
			apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
			apmib_sem_unlock();
#else
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
#endif
			return 0;
		}
	}
	triggerPortChain.compareLen = 5;	// length of trigger port range + proto type
#ifdef GW_QOS_ENGINE
	// initialize QoS rules table
	if ( !init_linkchain(&qosChain, sizeof(QOS_T), MAX_QOS_RULE_NUM)) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(pMib, CSCONF_SHM_KEY);
		apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
		apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
		apmib_sem_unlock();
#else
		free(pMib);
		free(pMibDef);
		free(pHwSetting);
#endif
		return 0;
	}
	for (i=0; i<pMib->qosRuleNum; i++) {
		if ( !add_linkchain(&qosChain, (char *)&pMib->qosRuleArray[i]) ) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_shm_free(pMib, CSCONF_SHM_KEY);
			apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
			apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
			apmib_sem_unlock();
#else
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
#endif
			return 0;
		}
	}
	qosChain.compareLen =  sizeof(QOS_T);
#endif

#ifdef QOS_BY_BANDWIDTH
	// initialize QoS rules table
	if ( !init_linkchain(&qosChain, sizeof(IPQOS_T), MAX_QOS_RULE_NUM)) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(pMib, CSCONF_SHM_KEY);
		apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
		apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
		apmib_sem_unlock();
#else
		free(pMib);
		free(pMibDef);
		free(pHwSetting);
#endif
		return 0;
	}
	for (i=0; i<pMib->qosRuleNum; i++) {
		if ( !add_linkchain(&qosChain, (char *)&pMib->qosRuleArray[i]) ) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_shm_free(pMib, CSCONF_SHM_KEY);
			apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
			apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
			apmib_sem_unlock();
#else
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
#endif
			return 0;
		}
	}
	qosChain.compareLen =  sizeof(IPQOS_T);
#endif

#ifdef ROUTE_SUPPORT
	// initialize static route table
	if ( !init_linkchain(&staticRouteChain, sizeof(STATICROUTE_T), MAX_ROUTE_NUM)) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(pMib, CSCONF_SHM_KEY);
		apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
		apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
		apmib_sem_unlock();
#else
		free(pMib);
		free(pMibDef);
		free(pHwSetting);
#endif
		return 0;
	}
	for (i=0; i<pMib->staticRouteNum; i++) {
		if ( !add_linkchain(&staticRouteChain, (char *)&pMib->staticRouteArray[i]) ) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_shm_free(pMib, CSCONF_SHM_KEY);
			apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
			apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
			apmib_sem_unlock();
#else
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
#endif
			return 0;
		}
	}
	staticRouteChain.compareLen = sizeof(STATICROUTE_T) -4 ; // not contain gateway
#endif //ROUTE
#ifdef VPN_SUPPORT
	// initialize port forwarding table
	if ( !init_linkchain(&ipsecTunnelChain, sizeof(IPSECTUNNEL_T), MAX_TUNNEL_NUM)) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(pMib, CSCONF_SHM_KEY);
		apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
		apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
		apmib_sem_unlock();
#else
		free(pMib);
		free(pMibDef);
		free(pHwSetting);
#endif
		return 0;
	}
	for (i=0; i<pMib->ipsecTunnelNum; i++) {
		if ( !add_linkchain(&ipsecTunnelChain, (char *)&pMib->ipsecTunnelArray[i]) ) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_shm_free(pMib, CSCONF_SHM_KEY);
			apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
			apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
			apmib_sem_unlock();
#else
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
#endif
			return 0;
		}
	}
	ipsecTunnelChain.compareLen = 1 ;  // only tunnel id
#endif
#endif // HOME_GATEWAY
#ifdef TLS_CLIENT
	if ( !init_linkchain(&certRootChain, sizeof(CERTROOT_T), MAX_CERTROOT_NUM)) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(pMib, CSCONF_SHM_KEY);
		apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
		apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
		apmib_sem_unlock();
#else
		free(pMib);
		free(pMibDef);
		free(pHwSetting);
#endif
		return 0;
	}
	for (i=0; i<pMib->certRootNum; i++) {
		if ( !add_linkchain(&certRootChain, (char *)&pMib->certRootArray[i]) ) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_shm_free(pMib, CSCONF_SHM_KEY);
			apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
			apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
			apmib_sem_unlock();
#else
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
#endif
			return 0;
		}
	}
	certRootChain.compareLen = 21 ;  // only comment
	if ( !init_linkchain(&certUserChain, sizeof(CERTUSER_T), MAX_CERTUSER_NUM)) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_shm_free(pMib, CSCONF_SHM_KEY);
		apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
		apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
		apmib_sem_unlock();
#else
		free(pMib);
		free(pMibDef);
		free(pHwSetting);
#endif
		return 0;
	}
	for (i=0; i<pMib->certUserNum; i++) {
		if ( !add_linkchain(&certUserChain, (char *)&pMib->certUserArray[i]) ) {
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_shm_free(pMib, CSCONF_SHM_KEY);
			apmib_shm_free(pMibDef, DSCONF_SHM_KEY);
			apmib_shm_free(pHwSetting, HWCONF_SHM_KEY);
			apmib_sem_unlock();
#else
			free(pMib);
			free(pMibDef);
			free(pHwSetting);
#endif
			return 0;
		}
	}
	certUserChain.compareLen = 21 ;  // only comment	
#endif
	init_linkchain(&dhcpRsvdIpChain, sizeof(DHCPRSVDIP_T), MAX_DHCP_RSVD_IP_NUM);
	for (i=0; i<pMib->dhcpRsvdIpNum; i++)
		add_linkchain(&dhcpRsvdIpChain, (char *)&pMib->dhcpRsvdIpArray[i]);	
	dhcpRsvdIpChain.compareLen = 4;

#if defined(VLAN_CONFIG_SUPPORTED)
	init_linkchain(&vlanConfigChain, sizeof(VLAN_CONFIG_T), MAX_IFACE_VLAN_CONFIG);
	for (i=0; i<pMib->VlanConfigNum; i++)
		add_linkchain(&vlanConfigChain, (char *)&pMib->VlanConfigArray[i]);	
	vlanConfigChain.compareLen = sizeof(VLAN_CONFIG_T);

#endif

#endif /*no def MIB_TLV*/

#if CONFIG_APMIB_SHARED_MEMORY == 1
    apmib_sem_unlock();
#endif

	return 1;
}


///////////////////////////////////////////////////////////////////////////////
#if CONFIG_APMIB_SHARED_MEMORY == 1
char *apmib_load_hwconf(void)
{
	int ver;
	char *buff;
	int created;

#ifdef COMPRESS_MIB_SETTING
	int zipRate=0;
	unsigned char *compFile=NULL, *expFile=NULL;
	unsigned int expandLen=0, compLen=0;
	COMPRESS_MIB_HEADER_T compHeader;
#endif

#ifdef COMPRESS_MIB_SETTING

	flash_read((char *)&compHeader, HW_SETTING_OFFSET, sizeof(COMPRESS_MIB_HEADER_T));
	if(memcmp(compHeader.signature, COMP_HS_SIGNATURE, COMP_SIGNATURE_LEN) == 0 ) //check whether compress mib data
	{
		zipRate = compHeader.compRate;
		compLen = compHeader.compLen;
		if ( (compLen > 0) && (compLen <= HW_SETTING_SECTOR_LEN) ) {
			compFile=malloc(compLen+sizeof(COMPRESS_MIB_HEADER_T));
			if(compFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);
				return 0;
			}
			expFile=malloc(zipRate*compLen);
			if(expFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);			
				free(compFile);
				return 0;
			}

			flash_read((char *)compFile, HW_SETTING_OFFSET, compLen+sizeof(COMPRESS_MIB_HEADER_T));

			expandLen = Decode(compFile+sizeof(COMPRESS_MIB_HEADER_T), compLen, expFile);
			// copy the mib header from expFile
			memcpy((char *)&hsHeader, expFile, sizeof(hsHeader));
		} 
		else 
		{
			COMP_TRACE(stderr,"\r\n ERR:compLen = %u __[%s-%u]",compLen, __FILE__,__LINE__);
			return 0;
		}

#ifdef MIB_TLV
		mib_table_entry_T *pmib_tl = NULL;
		unsigned char *hwMibData;
		unsigned int tlv_content_len = hsHeader.len - 1; // last one is checksum

		hwMibData = malloc(sizeof(HW_SETTING_T)+1); // 1: checksum

		if(hwMibData != NULL)
			memset(hwMibData, 0x00, sizeof(HW_SETTING_T)+1);
	
		pmib_tl = mib_get_table(HW_SETTING);
		unsigned int tlv_checksum = 0;

		if(expFile != NULL)
			tlv_checksum = CHECKSUM_OK(expFile+sizeof(PARAM_HEADER_T), hsHeader.len);

//mib_display_tlv_content(HW_SETTING, expFile+sizeof(PARAM_HEADER_T), hsHeader.len);

		if(tlv_checksum == 1 && mib_tlv_init(pmib_tl, expFile+sizeof(PARAM_HEADER_T), (void*)hwMibData, tlv_content_len) == 1) /* According to pmib_tl, get value from expFile to hwMibData. parse total len is  tlv_content_len*/
		{
			sprintf((char *)&hsHeader.signature[TAG_LEN], "%02d", HW_SETTING_VER);
			hsHeader.len = sizeof(HW_SETTING_T)+1;
			hwMibData[hsHeader.len-1]  = CHECKSUM(hwMibData, hsHeader.len-1);

			if(expFile!= NULL)
				free(expFile);

			expFile = malloc(sizeof(PARAM_HEADER_T)+hsHeader.len);
			memcpy(expFile, &hsHeader, sizeof(PARAM_HEADER_T));
			memcpy(expFile+sizeof(PARAM_HEADER_T), hwMibData, hsHeader.len);

//mib_display_data_content(HW_SETTING, expFile+sizeof(PARAM_HEADER_T), hsHeader.len);

			
		}	

		if(hwMibData != NULL)
			free(hwMibData);
		
#endif // #ifdef MIB_TLV

		
	}
	else // not compress mib-data, get mib header from flash
#endif //#ifdef COMPRESS_MIB_SETTING
	// Read hw setting
	if ( flash_read((char *)&hsHeader, HW_SETTING_OFFSET, sizeof(hsHeader))==0 ) {
//		printf("Read hw setting header failed!\n");
		return NULL;
	}

	if ( sscanf((char *)&hsHeader.signature[TAG_LEN], "%02d", &ver) != 1)
		ver = -1;

	if ( memcmp(hsHeader.signature, HW_SETTING_HEADER_TAG, TAG_LEN) || // invalid signatur
		(ver != HW_SETTING_VER) || // version not equal to current
		(hsHeader.len < (sizeof(HW_SETTING_T)+1)) ) { // length is less than current
//		printf("Invalid hw setting signature or version number [sig=%c%c, ver=%d, len=%d]!\n", hsHeader.signature[0],
//			hsHeader.signature[1], ver, hsHeader.len);
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
		return NULL;
	}
//	if (ver > HW_SETTING_VER)
//		printf("HW setting version is greater than current [f:%d, c:%d]!\n", ver, HW_SETTING_VER);

	buff = apmib_shm_calloc(1, hsHeader.len, HWCONF_SHM_KEY, &created);
	if ( buff == 0 ) {
//		printf("Allocate buffer failed!\n");
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
		return NULL;
	}
#ifdef COMPRESS_MIB_SETTING
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
	if(expFile != NULL) //compress mib data, copy the mibdata body from expFile
	{
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);	
		memcpy(buff, expFile+sizeof(PARAM_HEADER_T), hsHeader.len);
		free(expFile);
		free(compFile);
	}
	else // not compress mib data, copy mibdata from flash
#endif
	if ( flash_read(buff, HW_SETTING_OFFSET+sizeof(hsHeader), hsHeader.len)==0 ) {
//		printf("Read hw setting failed!\n");
		apmib_shm_free(buff, HWCONF_SHM_KEY);
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
		return NULL;
	}
	if ( !CHECKSUM_OK((unsigned char *)buff, hsHeader.len) ) {
//		printf("Invalid checksum of hw setting!\n");
		apmib_shm_free(buff, HWCONF_SHM_KEY);
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
		return NULL;
	}
	return buff;
}

////////////////////////////////////////////////////////////////////////////////
char *apmib_load_dsconf(void)
{
	int ver;
	char *buff;
	int created;

#ifdef COMPRESS_MIB_SETTING
	int zipRate=0;
	unsigned char *compFile=NULL, *expFile=NULL;
	unsigned int expandLen=0, compLen=0;
	COMPRESS_MIB_HEADER_T compHeader;
#endif

#ifdef COMPRESS_MIB_SETTING

	flash_read((char *)&compHeader, DEFAULT_SETTING_OFFSET, sizeof(COMPRESS_MIB_HEADER_T));

	if(memcmp(compHeader.signature, COMP_DS_SIGNATURE, COMP_SIGNATURE_LEN) == 0 ) //check whether compress mib data
	{
		zipRate = compHeader.compRate;
		compLen = compHeader.compLen;
		if ( (compLen > 0) && (compLen <= DEFAULT_SETTING_SECTOR_LEN) ) {
			compFile=malloc(compLen+sizeof(COMPRESS_MIB_HEADER_T));
			if(compFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);
				return 0;
			}
			expFile=malloc(zipRate*compLen);
			if(expFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);			
				free(compFile);
				return 0;
			}

			flash_read((char *)compFile, DEFAULT_SETTING_OFFSET, compLen+sizeof(COMPRESS_MIB_HEADER_T));

			expandLen = Decode(compFile+sizeof(COMPRESS_MIB_HEADER_T), compLen, expFile);

			// copy the mib header from expFile
			memcpy((char *)&dsHeader, expFile, sizeof(dsHeader));
			
		} 
		else 
		{
			COMP_TRACE(stderr,"\r\n ERR:compLen = %u __[%s-%u]",compLen, __FILE__,__LINE__);
			return 0;
		}

#ifdef MIB_TLV
		mib_table_entry_T *pmib_tl = NULL;
		unsigned char *defMibData;
		unsigned int tlv_content_len = dsHeader.len - 1; // last one is checksum

		defMibData = malloc(sizeof(APMIB_T)+1); // 1: checksum

		if(defMibData != NULL)
			memset(defMibData, 0x00, sizeof(APMIB_T)+1);
	
		pmib_tl = mib_get_table(DEFAULT_SETTING);
		unsigned int tlv_checksum = 0;

		if(expFile != NULL)
			tlv_checksum = CHECKSUM_OK(expFile+sizeof(PARAM_HEADER_T), dsHeader.len);

//mib_display_tlv_content(DEFAULT_SETTING, expFile+sizeof(PARAM_HEADER_T), dsHeader.len);
		
		if(tlv_checksum == 1 && mib_tlv_init(pmib_tl, expFile+sizeof(PARAM_HEADER_T), (void*)defMibData, tlv_content_len) == 1) /* According to pmib_tl, get value from expFile to hwMibData. parse total len is  tlv_content_len*/
		{
			sprintf((char *)&dsHeader.signature[TAG_LEN], "%02d", DEFAULT_SETTING_VER);
			dsHeader.len = sizeof(APMIB_T)+1;
			defMibData[dsHeader.len-1]  = CHECKSUM(defMibData, sizeof(APMIB_T));

			if(expFile!= NULL)
				free(expFile);

			expFile = malloc(sizeof(PARAM_HEADER_T)+dsHeader.len);
			memcpy(expFile, &dsHeader, sizeof(PARAM_HEADER_T));
			memcpy(expFile+sizeof(PARAM_HEADER_T), defMibData, dsHeader.len);
			
//mib_display_data_content(DEFAULT_SETTING, expFile+sizeof(PARAM_HEADER_T), dsHeader.len);
		}	
		else
		{
			COMP_TRACE(stderr,"\r\n ERR!Invalid checksum[%u] or mib_tlv_init() fail! __[%s-%u]",tlv_checksum,__FILE__,__LINE__);
		}
		
		if(defMibData != NULL)
			free(defMibData);
		
#endif // #ifdef MIB_TLV		

		
	}
	else // not compress mib-data, get mib header from flash
#endif //#ifdef COMPRESS_MIB_SETTING
	// Read default s/w mib
	if ( flash_read((char *)&dsHeader, DEFAULT_SETTING_OFFSET, sizeof(dsHeader))==0 ) {
//		printf("Read default setting header failed!\n");
		return NULL;
	}

	if ( sscanf((char *)&dsHeader.signature[TAG_LEN], "%02d", &ver) != 1)
		ver = -1;

	if ( memcmp(dsHeader.signature, DEFAULT_SETTING_HEADER_TAG, TAG_LEN) || // invalid signatur
		(ver != DEFAULT_SETTING_VER) || // version not equal to current
		(dsHeader.len < (sizeof(APMIB_T)+1)) ) { // length is less than current
//		printf("Invalid default setting signature or version number [sig=%c%c, ver=%d, len=%d]!\n",
//			dsHeader.signature[0], dsHeader.signature[1], ver, dsHeader.len);
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
		return NULL;
	}
//	if (ver > DEFAULT_SETTING_VER)
//		printf("Default setting version is greater than current [f:%d, c:%d]!\n", ver, DEFAULT_SETTING_VER);

	buff = apmib_shm_calloc(1, dsHeader.len, DSCONF_SHM_KEY, &created);
	if ( buff == 0 ) {
//		printf("Allocate buffer failed!\n");
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
		return NULL;
	}

#ifdef COMPRESS_MIB_SETTING

	if(expFile != NULL) //compress mib data, copy the mibdata body from expFile
	{
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);		
		memcpy(buff, expFile+sizeof(PARAM_HEADER_T), dsHeader.len);
		free(expFile);
		free(compFile);
	}
	else // not compress mib data, copy mibdata from flash
#endif
	if ( flash_read(buff, DEFAULT_SETTING_OFFSET+sizeof(dsHeader), dsHeader.len)==0 ) {
//		printf("Read default setting failed!\n");
		apmib_shm_free(buff, DSCONF_SHM_KEY);
		return NULL;
	}

	if ( !CHECKSUM_OK((unsigned char *)buff, dsHeader.len) ) {
//		printf("Invalid checksum of current setting!\n");
		apmib_shm_free(buff, DSCONF_SHM_KEY);
		return NULL;
	}
	return buff;
}

////////////////////////////////////////////////////////////////////////////////
char *apmib_load_csconf(void)
{
	int ver;
	char *buff;
	int created;

#ifdef COMPRESS_MIB_SETTING
	int zipRate=0;
	unsigned char *compFile=NULL, *expFile=NULL;
	unsigned int expandLen=0, compLen=0;
	COMPRESS_MIB_HEADER_T compHeader;
#endif

#ifdef COMPRESS_MIB_SETTING

//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
	flash_read((char *)&compHeader, CURRENT_SETTING_OFFSET, sizeof(COMPRESS_MIB_HEADER_T));
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);	
	if(memcmp(compHeader.signature, COMP_CS_SIGNATURE, COMP_SIGNATURE_LEN) == 0 ) //check whether compress mib data
	{
		zipRate = compHeader.compRate;
		compLen = compHeader.compLen;
		if ( (compLen > 0) && (compLen <= CURRENT_SETTING_SECTOR_LEN) ) {
			compFile=calloc(1,compLen+sizeof(COMPRESS_MIB_HEADER_T));
			if(compFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);
				return 0;
			}
			expFile=calloc(1,zipRate*compLen);
			if(expFile==NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR:compFile malloc fail! __[%s-%u]",__FILE__,__LINE__);			
				free(compFile);
				return 0;
			}

			flash_read((char *)compFile, CURRENT_SETTING_OFFSET, compLen+sizeof(COMPRESS_MIB_HEADER_T));

			expandLen = Decode(compFile+sizeof(COMPRESS_MIB_HEADER_T), compLen, expFile);
			//printf("expandLen read len: %d\n", expandLen);

			// copy the mib header from expFile
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);				
			memcpy((char *)&csHeader, expFile, sizeof(csHeader));
			
		} 
		else 
		{
			COMP_TRACE(stderr,"\r\n ERR:compLen = %u __[%s-%u]",compLen, __FILE__,__LINE__);
			return 0;
		}

#ifdef MIB_TLV
		mib_table_entry_T *pmib_tl = NULL;
		unsigned char *curMibData;
		unsigned int tlv_content_len = csHeader.len - 1; // last one is checksum

		curMibData = malloc(sizeof(APMIB_T)+1); // 1: checksum

		if(curMibData != NULL)
			memset(curMibData, 0x00, sizeof(APMIB_T)+1);
	
		pmib_tl = mib_get_table(CURRENT_SETTING);
		unsigned int tlv_checksum = 0;

		if(expFile != NULL)
			tlv_checksum = CHECKSUM_OK(expFile+sizeof(PARAM_HEADER_T), csHeader.len);

//mib_display_tlv_content(CURRENT_SETTING, expFile+sizeof(PARAM_HEADER_T), csHeader.len);

		if(tlv_checksum == 1 && mib_tlv_init(pmib_tl, expFile+sizeof(PARAM_HEADER_T), (void*)curMibData, tlv_content_len) == 1) /* According to pmib_tl, get value from expFile to hwMibData. parse total len is  tlv_content_len*/
		{
			sprintf((char *)&csHeader.signature[TAG_LEN], "%02d", CURRENT_SETTING_VER);
			csHeader.len = sizeof(APMIB_T)+1;
			curMibData[csHeader.len-1]  = CHECKSUM(curMibData, csHeader.len-1);

			if(expFile!= NULL)
				free(expFile);

			expFile = malloc(sizeof(PARAM_HEADER_T)+csHeader.len);
			memcpy(expFile, &csHeader, sizeof(PARAM_HEADER_T));
			memcpy(expFile+sizeof(PARAM_HEADER_T), curMibData, csHeader.len);
			
//mib_display_data_content(CURRENT_SETTING, expFile+sizeof(PARAM_HEADER_T), csHeader.len);
		}	

		if(curMibData != NULL)
			free(curMibData);
		
#endif // #ifdef MIB_TLV	

	}
	else // not compress mib-data, get mib header from flash
#endif //#ifdef COMPRESS_MIB_SETTING
	// Read current s/w mib
	if ( flash_read((char *)&csHeader, CURRENT_SETTING_OFFSET, sizeof(csHeader))==0 ) {
//		printf("Read current setting header failed!\n");
		return NULL;
	}

	if ( sscanf((char *)&csHeader.signature[TAG_LEN], "%02d", &ver) != 1)
		ver = -1;

	if ( memcmp(csHeader.signature, CURRENT_SETTING_HEADER_TAG, TAG_LEN) || // invalid signatur
		(ver != CURRENT_SETTING_VER) || // version not equal to current
			(csHeader.len < (sizeof(APMIB_T)+1)) ) { // length is less than current
//		printf("Invalid current setting signature or version number [sig=%c%c, ver=%d, len=%d]!\n",
//			csHeader.signature[0], csHeader.signature[1], ver, csHeader.len);
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
		return NULL;
	}

//	if (ver > CURRENT_SETTING_VER)
//		printf("Current setting version is greater than current [f:%d, c:%d]!\n", ver, CURRENT_SETTING_VER);

	buff = apmib_shm_calloc(1, csHeader.len, CSCONF_SHM_KEY, &created);
	if ( buff == 0 ) {
//		printf("Allocate buffer failed!\n");
#ifdef COMPRESS_MIB_SETTING
		if(compFile != NULL)
			free(compFile);
		if(expFile != NULL)
			free(expFile);
#endif
		return NULL;
	}

#ifdef COMPRESS_MIB_SETTING

	if(expFile != NULL) //compress mib data, copy the mibdata body from expFile
	{
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);					
		memcpy(buff, expFile+sizeof(PARAM_HEADER_T), csHeader.len);
		free(expFile);
		free(compFile);
	}
	else // not compress mib data, copy mibdata from flash
#endif
	if ( flash_read(buff, CURRENT_SETTING_OFFSET+sizeof(csHeader), csHeader.len)==0 ) {
//		printf("Read current setting failed!\n");
		apmib_shm_free(buff, CSCONF_SHM_KEY);
		return NULL;
	}

	if ( !CHECKSUM_OK((unsigned char *)buff, csHeader.len) ) {
//		printf("Invalid checksum of current setting!\n");
		apmib_shm_free(buff, CSCONF_SHM_KEY);
		return NULL;
	}

	return buff;
}
#endif

int apmib_reinit(void)
{
#ifndef MIB_TLV
	int i, j;
#endif

	if (pMib == NULL)	// has not been initialized
		return 0;

#if CONFIG_APMIB_SHARED_MEMORY != 1
	free(pMib);
	free(pMibDef);
	free(pHwSetting);
#endif

#ifdef MIB_TLV
#else
	for (i=0; i<NUM_WLAN_INTERFACE; i++) 
		for (j=0; j<(NUM_VWLAN_INTERFACE+1); j++) 
	{
		free(wlanMacChain[i][j].buf);
		free(wdsChain[i][j].buf);
	}

#ifdef HOME_GATEWAY
	free(portFwChain.buf);
	free(ipFilterChain.buf);
	free(portFilterChain.buf);
	free(macFilterChain.buf);
	free(urlFilterChain.buf);
	free(triggerPortChain.buf);
#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
    	free(qosChain.buf);
#endif
#ifdef ROUTE_SUPPORT
	free(staticRouteChain.buf);
#endif //ROUTE
#ifdef VPN_SUPPORT
	free(ipsecTunnelChain.buf);
#endif
#endif
#ifdef TLS_CLIENT
	free(certRootChain.buf);
	free(certUserChain.buf);
#endif

	free(dhcpRsvdIpChain.buf);
#if defined(VLAN_CONFIG_SUPPORTED)
	free(vlanConfigChain.buf);
#endif	
#endif

#if CONFIG_APMIB_SHARED_MEMORY != 1
	pMib=NULL;
	pMibDef=NULL;
	pHwSetting=NULL;
#endif


#if CONFIG_APMIB_SHARED_MEMORY == 1	
    apmib_sem_lock();
    apmib_load_hwconf();
    apmib_load_dsconf();
    apmib_load_csconf();
    apmib_sem_unlock();
#endif

	return apmib_init();
}

////////////////////////////////////////////////////////////////////////////////
static int search_tbl(int id, mib_table_entry_T *pTbl, int *idx)
{
	int i;
	for (i=0; pTbl[i].id; i++) {
		if ( pTbl[i].id == id ) {
			*idx = i;
			return id;
		}
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
int apmib_get(int id, void *value)
{
	int i, index;
	void *pMibTbl;
	mib_table_entry_T *pTbl;
	unsigned char ch;
	unsigned short wd;
	unsigned long dwd;
#ifdef MIB_TLV
	//unsigned int offset;
	unsigned int num;
#endif

#ifndef MIB_TLV
#if CONFIG_APMIB_SHARED_MEMORY == 1
	apmib_sem_lock();
#endif
#endif

	if ( search_tbl(id, mib_table, &i) ) {
		pMibTbl = (void *)pMib;
		pTbl = mib_table;
	}
	else if ( search_tbl(id, mib_wlan_table, &i) ) {
		pMibTbl = (void *)&pMib->wlan[wlan_idx][vwlan_idx];
		pTbl = mib_wlan_table;
	}
	else if ( search_tbl(id, hwmib_table, &i) ) {
		pMibTbl = (void *)pHwSetting;
		pTbl = hwmib_table;
	}
	else if ( search_tbl(id, hwmib_wlan_table, &i) ) {
		pMibTbl = (void *)&pHwSetting->wlan[wlan_idx];
		pTbl = hwmib_wlan_table;
	}
	else {
#ifndef MIB_TLV
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif
#endif
		return 0;
	}
	
#ifdef MIB_TLV
	if(pTbl[i].type > TABLE_LIST_T)
	{		
		apmib_get(((pTbl[i].id & MIB_ID_MASK)-1),&num);
		index = (int)( *((unsigned char *)value));
		//printf("get index %d\n",index);
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_lock();
#endif		
		get_tblentry((void*)pMibTbl,pTbl[i].offset,num,&pTbl[i],(void *)value,index);
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif
		return 1;
	}
#endif

#ifdef MIB_TLV
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_lock();
#endif	
#endif

	switch (pTbl[i].type) {
	case BYTE_T:
//		*((int *)value) =(int)(*((unsigned char *)(((long)pMibTbl) + pTbl[i].offset)));
		memcpy((char *)&ch, ((char *)pMibTbl) + pTbl[i].offset, 1);
		*((int *)value) = (int)ch;
		break;

	case WORD_T:
//		*((int *)value) =(int)(*((unsigned short *)(((long)pMibTbl) + pTbl[i].offset)));
		memcpy((char *)&wd, ((char *)pMibTbl) + pTbl[i].offset, 2);
		*((int *)value) = (int)wd;
		break;

	case STRING_T:
		strcpy( (char *)value, (const char *)(((long)pMibTbl) + pTbl[i].offset) );
		break;

	case BYTE5_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 5);
		break;

	case BYTE6_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 6);
		break;

	case BYTE13_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 13);
		break;

	case DWORD_T:
		memcpy((char *)&dwd, ((char *)pMibTbl) + pTbl[i].offset, 4);
		*((int *)value) = (int)dwd;
		break;

	case BYTE_ARRAY_T:
#ifdef VOIP_SUPPORT
		if(id == MIB_VOIP_CFG){
			// rock: do nothing here, use flash voip get xxx to replace
		}
		else
#endif /*VOIP_SUPPORT*/
		if(id == MIB_L2TP_PAYLOAD)
		{
			memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 38);
		}
		else
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		break;

	case IA_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 4);
		break;

#ifdef HOME_GATEWAY
#ifdef CONFIG_IPV6
	case RADVDPREFIX_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		break;
		
	case DNSV6_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		break;
	case DHCPV6S_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		break;
	case ADDR6_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		break;
	case TUNNEL6_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		break;	
#endif
#endif

#ifdef VOIP_SUPPORT
	case VOIP_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		//printf("apimb: mib_get MIB_VOIP_CFG, do nothing here\n");
		break;
#endif

#ifndef MIB_TLV
	case WLAC_ARRAY_T:
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif	
		index = (int)( *((unsigned char *)value));
		return get_linkchain(&wlanMacChain[wlan_idx][vwlan_idx], (char *)value, index );

//#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
	case MESH_ACL_ARRAY_T:
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif	
		index = (int)( *((unsigned char *)value));
		return get_linkchain(&meshAclChain, (char *)value, index );
//#endif	// CONFIG_RTK_MESH && _MESH_ACL_ENABLE_

	case WDS_ARRAY_T:
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif	
		index = (int)( *((unsigned char *)value));
		return get_linkchain(&wdsChain[wlan_idx][vwlan_idx], (char *)value, index );

	case SCHEDULE_ARRAY_T:
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif	
    index = (int)( *((unsigned char *)value));
 		return get_linkchain(&scheduleRuleChain[wlan_idx][vwlan_idx], (char *)value, index ); 		


#ifdef HOME_GATEWAY
	case PORTFW_ARRAY_T:
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif	
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&portFwChain, (char *)value, index );

	case IPFILTER_ARRAY_T:
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif	
		index = (int)( *((unsigned char *)value));
 		return get_linkchain(&ipFilterChain, (char *)value, index );

	case PORTFILTER_ARRAY_T:
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif	
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&portFilterChain, (char *)value, index );

	case MACFILTER_ARRAY_T:
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif	
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&macFilterChain, (char *)value, index );

	case URLFILTER_ARRAY_T:
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif	
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&urlFilterChain, (char *)value, index );

	case TRIGGERPORT_ARRAY_T:
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif	
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&triggerPortChain, (char *)value, index );

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
	case QOS_ARRAY_T:
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&qosChain, (char *)value, index );
#endif
#ifdef ROUTE_SUPPORT
	case STATICROUTE_ARRAY_T:
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif	
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&staticRouteChain, (char *)value, index );
#endif

#ifdef VPN_SUPPORT
	case IPSECTUNNEL_ARRAY_T:
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&ipsecTunnelChain, (char *)value, index );
#endif

#endif /*HOME_GATEWAY*/
#ifdef TLS_CLIENT
	case CERTROOT_ARRAY_T:
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&certRootChain, (char *)value, index );
	case CERTUSER_ARRAY_T:
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif
                index = (int)( *((unsigned char *)value));
 		return get_linkchain(&certUserChain, (char *)value, index ); 		
#endif
	case DHCPRSVDIP_ARRY_T:	
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif
		index = (int)( *((unsigned char *)value));
 		return get_linkchain(&dhcpRsvdIpChain, (char *)value, index );

#if defined(VLAN_CONFIG_SUPPORTED)
	case VLANCONFIG_ARRAY_T:	
#if CONFIG_APMIB_SHARED_MEMORY == 1
	apmib_sem_unlock();
#endif
		index = (int)( *((unsigned char *)value));
 		return get_linkchain(&vlanConfigChain, (char *)value, index ); 		
#endif 	
#endif /*no def MIB_TLV*/
	default:
		break;
	}
#if CONFIG_APMIB_SHARED_MEMORY == 1
	apmib_sem_unlock();
#endif
	return 1;
}


////////////////////////////////////////////////////////////////////////////////
int apmib_getDef(int id, void *value)
{
	int ret;
	APMIB_Tp saveMib=pMib;

	pMib = pMibDef;
	ret = apmib_get(id, value);
	pMib = saveMib;
	return ret;
}


////////////////////////////////////////////////////////////////////////////////
int apmib_set(int id, void *value)
{
	int i, ret=1;
	void *pMibTbl;
	mib_table_entry_T *pTbl;
	unsigned char ch;
	unsigned short wd;
	unsigned long dwd;
	unsigned char* tmp;
	int max_chan_num=MAX_2G_CHANNEL_NUM_MIB;
#ifdef MIB_TLV
	//unsigned int offset=0;
	unsigned int mib_num_id=0;
	unsigned int num;
	unsigned int id_orig;
	#if defined(MIB_MOD_TBL_ENTRY)
	unsigned int mod_tbl=0;
	#endif
#endif

#ifdef MIB_TLV
#else
#if CONFIG_APMIB_SHARED_MEMORY == 1
	apmib_sem_lock();
#endif
#endif

#ifdef MIB_TLV
	id_orig = id;
	if( id_orig & MIB_ADD_TBL_ENTRY)
	{
		id=((id_orig & MIB_ID_MASK)-1 )|(MIB_TABLE_LIST);
		mib_num_id=(id_orig & MIB_ID_MASK)-2;

	}
	else if(id_orig & MIB_DEL_TBL_ENTRY)
	{
	#if defined(MIB_MOD_TBL_ENTRY)
		if (id_orig & MIB_MOD_TBL_ENTRY) {
			id_orig &= ~MIB_MOD_TBL_ENTRY;
			id = id_orig;
			mod_tbl = 1;
		}
	#endif

		id=((id_orig & MIB_ID_MASK)-2 )|(MIB_TABLE_LIST);
		mib_num_id=(id_orig & MIB_ID_MASK)-3;	
	}
	else if(id_orig & MIB_DELALL_TBL_ENTRY)
	{
		id=((id_orig & MIB_ID_MASK)-3 )|(MIB_TABLE_LIST);
		mib_num_id=(id_orig & MIB_ID_MASK)-4;	
	}
#else

	if (id == MIB_WLAN_AC_ADDR_ADD) {
		ret = add_linkchain(&wlanMacChain[wlan_idx][vwlan_idx], (char *)value);
		if ( ret )
			pMib->wlan[wlan_idx][vwlan_idx].acNum++;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_WLAN_AC_ADDR_DEL) {
		ret = delete_linkchain(&wlanMacChain[wlan_idx][vwlan_idx], (char *)value);
		if ( ret )
			pMib->wlan[wlan_idx][vwlan_idx].acNum--;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_WLAN_AC_ADDR_DELALL) {
		delete_all_linkchain(&wlanMacChain[wlan_idx][vwlan_idx]);
		pMib->wlan[wlan_idx][vwlan_idx].acNum = 0;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
	if (id == MIB_MESH_ACL_ADDR_ADD) {
		ret = add_linkchain(&meshAclChain, (char *)value);
		if ( ret )
			pMib->meshAclNum++;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_MESH_ACL_ADDR_DEL) {
		ret = delete_linkchain(&meshAclChain, (char *)value);
		if ( ret )
			pMib->meshAclNum--;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_MESH_ACL_ADDR_DELALL) {
		delete_all_linkchain(&meshAclChain);
		pMib->meshAclNum = 0;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}
#endif	// CONFIG_RTK_MESH && _MESH_ACL_ENABLE_

	if (id == MIB_WLAN_WDS_ADD) {
		ret = add_linkchain(&wdsChain[wlan_idx][vwlan_idx], (char *)value);
		if ( ret )
			pMib->wlan[wlan_idx][vwlan_idx].wdsNum++;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_WLAN_WDS_DEL) {
		ret = delete_linkchain(&wdsChain[wlan_idx][vwlan_idx], (char *)value);
		if ( ret )
			pMib->wlan[wlan_idx][vwlan_idx].wdsNum--;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_WLAN_WDS_DELALL) {
		delete_all_linkchain(&wdsChain[wlan_idx][vwlan_idx]);
		pMib->wlan[wlan_idx][vwlan_idx].wdsNum = 0;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}
//Schedule Mib
	if (id == MIB_WLAN_SCHEDULE_ADD) {
		ret = add_linkchain(&scheduleRuleChain[wlan_idx][vwlan_idx], (char *)value);
		if ( ret )
			pMib->wlan[wlan_idx][vwlan_idx].scheduleRuleNum++;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	//### Edit by sen_liu 2011.5.13 I believe it is a clerical error
	//if (id == MIB_SCHEDULE_DEL) {
	if (id == MIB_WLAN_SCHEDULE_DEL) {
	//### end
		ret = delete_linkchain(&scheduleRuleChain[wlan_idx][vwlan_idx], (char *)value);
		if ( ret )
			pMib->wlan[wlan_idx][vwlan_idx].scheduleRuleNum--;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_WLAN_SCHEDULE_DELALL) {
		delete_all_linkchain(&scheduleRuleChain[wlan_idx][vwlan_idx]);
		pMib->wlan[wlan_idx][vwlan_idx].scheduleRuleNum = 0;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}

#if defined(VLAN_CONFIG_SUPPORTED)
	if (id == MIB_VLANCONFIG_ADD || id == MIB_VLANCONFIG_DEL) {
		int entryNum=0,i;
		VLAN_CONFIG_T entry;
		VLAN_CONFIG_Tp entry_new;
		entry_new = (VLAN_CONFIG_Tp)value;
		apmib_get(MIB_VLANCONFIG_TBL_NUM, (void *)&entryNum);
		for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
			apmib_get(MIB_VLANCONFIG_TBL, (void *)&entry);
			if(!strcmp(entry.netIface, entry_new->netIface)){
				update_linkchain(VLANCONFIG_ARRAY_T, &entry, entry_new, sizeof(VLAN_CONFIG_T));
				break;
			}
		}
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}

#if defined(VLAN_CONFIG_SUPPORTED)
	if (id == MIB_VLANCONFIG_DELALL) {
		int entryNum=0,i;
		VLAN_CONFIG_T entry;
		VLAN_CONFIG_T entry_new;
		apmib_get(MIB_VLANCONFIG_TBL_NUM, (void *)&entryNum);

		for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
			apmib_get(MIB_VLANCONFIG_TBL, (void *)&entry);
			memcpy(&entry_new, &entry, sizeof(VLAN_CONFIG_T));
			entry_new.enabled=0;
			update_linkchain(VLANCONFIG_ARRAY_T, &entry, &entry_new, sizeof(VLAN_CONFIG_T));
		}
#endif		
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}

#endif

#ifdef HOME_GATEWAY
	if (id == MIB_PORTFW_ADD) {
		ret = add_linkchain(&portFwChain, (char *)value);
		if ( ret )
			pMib->portFwNum++;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_PORTFW_DEL) {
		ret = delete_linkchain(&portFwChain, (char *)value);
		if ( ret )
			pMib->portFwNum--;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_PORTFW_DELALL) {
		delete_all_linkchain(&portFwChain);
		pMib->portFwNum = 0;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}

	if (id == MIB_IPFILTER_ADD) {
		ret = add_linkchain(&ipFilterChain, (char *)value);
		if ( ret )
			pMib->ipFilterNum++;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_IPFILTER_DEL) {
		ret = delete_linkchain(&ipFilterChain, (char *)value);
		if ( ret )
			pMib->ipFilterNum--;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_IPFILTER_DELALL) {
		delete_all_linkchain(&ipFilterChain);
		pMib->ipFilterNum = 0;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}

	if (id == MIB_PORTFILTER_ADD) {
		ret = add_linkchain(&portFilterChain, (char *)value);
		if ( ret )
			pMib->portFilterNum++;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_PORTFILTER_DEL) {
		ret = delete_linkchain(&portFilterChain, (char *)value);
		if ( ret )
			pMib->portFilterNum--;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_PORTFILTER_DELALL) {
		delete_all_linkchain(&portFilterChain);
		pMib->portFilterNum = 0;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}

	if (id == MIB_MACFILTER_ADD) {
		ret = add_linkchain(&macFilterChain, (char *)value);
		if ( ret )
			pMib->macFilterNum++;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_MACFILTER_DEL) {
		ret = delete_linkchain(&macFilterChain, (char *)value);
		if ( ret )
			pMib->macFilterNum--;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_MACFILTER_DELALL) {
		delete_all_linkchain(&macFilterChain);
		pMib->macFilterNum = 0;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}

	if (id == MIB_URLFILTER_ADD) {
		ret = add_linkchain(&urlFilterChain, (char *)value);
		if ( ret )
			pMib->urlFilterNum++;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_URLFILTER_DEL) {
		ret = delete_linkchain(&urlFilterChain, (char *)value);
		if ( ret )
			pMib->urlFilterNum--;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_URLFILTER_DELALL) {
		delete_all_linkchain(&urlFilterChain);
		pMib->urlFilterNum = 0;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}

	if (id == MIB_TRIGGERPORT_ADD) {
		ret = add_linkchain(&triggerPortChain, (char *)value);
		if ( ret )
			pMib->triggerPortNum++;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_TRIGGERPORT_DEL) {
		ret = delete_linkchain(&triggerPortChain, (char *)value);
		if ( ret )
			pMib->triggerPortNum--;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_TRIGGERPORT_DELALL) {
		delete_all_linkchain(&triggerPortChain);
		pMib->triggerPortNum = 0;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
	if (id == MIB_QOS_ADD) {
		ret = add_linkchain(&qosChain, (char *)value);
		if ( ret )
			pMib->qosRuleNum++;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_QOS_DEL) {
		ret = delete_linkchain(&qosChain, (char *)value);
		if ( ret )
			pMib->qosRuleNum--;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_QOS_DELALL) {
		delete_all_linkchain(&qosChain);
		pMib->qosRuleNum = 0;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}
#endif
#ifdef ROUTE_SUPPORT
	if (id == MIB_STATICROUTE_ADD) {
		ret = add_linkchain(&staticRouteChain, (char *)value);
		if ( ret )
			pMib->staticRouteNum++;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_STATICROUTE_DEL) {
		ret = delete_linkchain(&staticRouteChain, (char *)value);
		if ( ret )
			pMib->staticRouteNum--;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_STATICROUTE_DELALL) {
		delete_all_linkchain(&staticRouteChain);
		pMib->staticRouteNum = 0;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}
#endif //ROUTE
#endif

	if (id == MIB_DHCPRSVDIP_DEL) {
		ret = delete_linkchain(&dhcpRsvdIpChain, (char *)value);
		if ( ret )
			pMib->dhcpRsvdIpNum--;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_DHCPRSVDIP_DELALL) {
		delete_all_linkchain(&dhcpRsvdIpChain);
		pMib->dhcpRsvdIpNum = 0;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}
	if (id == MIB_DHCPRSVDIP_ADD) {
		ret = add_linkchain(&dhcpRsvdIpChain, (char *)value);
		if ( ret )
			pMib->dhcpRsvdIpNum++;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
	if (id == MIB_IPSECTUNNEL_ADD) {
		ret = add_linkchain(&ipsecTunnelChain, (char *)value);
		if ( ret )
			pMib->ipsecTunnelNum++;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_IPSECTUNNEL_DEL) {
		ret = delete_linkchain(&ipsecTunnelChain, (char *)value);
		if ( ret )
			pMib->ipsecTunnelNum--;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_IPSECTUNNEL_DELALL) {
		delete_all_linkchain(&ipsecTunnelChain);
		pMib->ipsecTunnelNum= 0;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}
#endif
#endif
#ifdef TLS_CLIENT
	if (id == MIB_CERTROOT_ADD) {
		ret = add_linkchain(&certRootChain, (char *)value);
		if ( ret )
			pMib->certRootNum++;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_CERTROOT_DEL) {
		ret = delete_linkchain(&certRootChain, (char *)value);
		if ( ret )
			pMib->certRootNum--;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_CERTROOT_DELALL) {
		delete_all_linkchain(&certRootChain);
		pMib->certRootNum= 0;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}
	if (id == MIB_CERTUSER_ADD) {
		ret = add_linkchain(&certUserChain, (char *)value);
		if ( ret )
			pMib->certUserNum++;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_CERTUSER_DEL) {
		ret = delete_linkchain(&certUserChain, (char *)value);
		if ( ret )
			pMib->certUserNum--;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return ret;
	}
	if (id == MIB_CERTUSER_DELALL) {
		delete_all_linkchain(&certUserChain);
		pMib->certUserNum= 0;
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif
		return 1;
	}	
#endif
#endif /*MIB_TLV*/

	if ( search_tbl(id, mib_table, &i) ) {
		pMibTbl = (void *)pMib;
		pTbl = mib_table;
	}
	else if ( search_tbl(id, mib_wlan_table, &i) ) {
		pMibTbl = (void *)&pMib->wlan[wlan_idx][vwlan_idx];
		pTbl = mib_wlan_table;
	}
	else if ( search_tbl(id, hwmib_table, &i) ) {
		pMibTbl = (void *)pHwSetting;
		pTbl = hwmib_table;
	}
	else if ( search_tbl(id, hwmib_wlan_table, &i) ) {
		pMibTbl = (void *)&pHwSetting->wlan[wlan_idx];
		pTbl = hwmib_wlan_table;
	}
	else {
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    apmib_sem_unlock();
#endif	 
		printf("id not found\n");
		return 0;	
	}

#ifdef MIB_TLV
	if(pTbl[i].type > TABLE_LIST_T)
	{		
		apmib_get(mib_num_id,&num);
		
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_lock();
#endif
		if(id_orig & MIB_ADD_TBL_ENTRY)
		{
			ret= add_tblentry((void *)pMibTbl,pTbl[i].offset,num,&pTbl[i],(void *)value);
			if(ret)
				num++;
		}
		else if(id_orig & MIB_DEL_TBL_ENTRY)
		{
		#if defined(MIB_MOD_TBL_ENTRY)
			if (mod_tbl == 1) {
				ret= mod_tblentry((void *)pMibTbl,pTbl[i].offset,num,&pTbl[i],(void *)value);
			}
			else 
		#endif
			{
			ret= delete_tblentry((void *)pMibTbl,pTbl[i].offset,num,&pTbl[i],(void *)value);
			if(ret)
				num--;
		}
		}
		else if(id_orig & MIB_DELALL_TBL_ENTRY)
		{
			ret= delete_all_tblentry((void *)pMibTbl,pTbl[i].offset,num,&pTbl[i]);
			if(ret)
				num=0;
		}
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif
		if(ret)
			apmib_set(mib_num_id,&num);		
		//printf("num %d ret %d\n",num,ret);
		return ret;
	}
#endif

#ifdef MIB_TLV
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_sem_lock();
#endif
#endif

	switch (pTbl[i].type) {
	case BYTE_T:
//		*((unsigned char *)(((long)pMibTbl) + pTbl[i].offset)) = (unsigned char)(*((int *)value));
		ch = (unsigned char)(*((int *)value));
		memcpy( ((char *)pMibTbl) + pTbl[i].offset, &ch, 1);
		break;

	case WORD_T:
//		*((unsigned short *)(((long)pMibTbl) + pTbl[i].offset)) = (unsigned short)(*((int *)value));
		wd = (unsigned short)(*((int *)value));
		memcpy( ((char *)pMibTbl) + pTbl[i].offset, &wd, 2);
		break;

	case STRING_T:
		if ( strlen(value)+1 > pTbl[i].size )
		{
#if CONFIG_APMIB_SHARED_MEMORY == 1
	    		apmib_sem_unlock();
#endif		
			return 0;
		}
		if (value==NULL || strlen(value)==0)
			*((char *)(((long)pMibTbl) + pTbl[i].offset)) = '\0';
		else
			strcpy((char *)(((long)pMibTbl) + pTbl[i].offset), (char *)value);
		break;

	case BYTE5_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, 5);
		break;

	case BYTE6_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, 6);
		break;

	case BYTE13_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, 13);
		break;

	case DWORD_T:
		dwd = (unsigned long)(*((int *)value));
		memcpy( ((char *)pMibTbl) + pTbl[i].offset, &dwd, 4);
		break;
	case BYTE_ARRAY_T:
		tmp = (unsigned char*) value;
#ifdef VPN_SUPPORT
		if(id == MIB_IPSEC_RSA_FILE){
                        memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, MAX_RSA_FILE_LEN);
		}
		else
#endif

		if(id == MIB_L2TP_PAYLOAD){
			  memcpy((unsigned char *)((( long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, 38);
		}
		else
			
#ifdef VOIP_SUPPORT
#ifdef VOIP_SUPPORT_TLV_CFG
	// do nothing
#else
		if(id == MIB_VOIP_CFG){
			printf("apimb: mib_set MIB_VOIP_CFG\n");

			memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, pTbl[i].size);

		}
		else
#endif
#endif /*VOIP_SUPPORT*/

		{
#if defined(CONFIG_RTL_8196B)
			max_chan_num = (id == MIB_HW_TX_POWER_CCK)? MAX_CCK_CHAN_NUM: MAX_OFDM_CHAN_NUM ;
#elif defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
                        if((id >= MIB_HW_TX_POWER_CCK_A &&  id <=MIB_HW_TX_POWER_DIFF_OFDM))
				max_chan_num = MAX_2G_CHANNEL_NUM_MIB;
			else if((id >= MIB_HW_TX_POWER_5G_HT40_1S_A &&  id <=MIB_HW_TX_POWER_DIFF_5G_OFDM))
				max_chan_num = MAX_5G_CHANNEL_NUM_MIB;         

#endif
			if(tmp[0]==2){
				if(tmp[3] == 0xff){ // set one channel value
					memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset + (long)tmp[1] -1), (unsigned char *)(tmp+2), 1);
				}
			}else{
					memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)(value+1), max_chan_num);
				}		
		}
		break;
	case IA_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  4);
		break;
#ifdef HOME_GATEWAY
#ifdef CONFIG_IPV6
	case RADVDPREFIX_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  sizeof(radvdCfgParam_t));
		break;

	case DNSV6_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  sizeof(dnsv6CfgParam_t));
		break;
	case DHCPV6S_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  sizeof(dhcp6sCfgParam_t));
		break;
	case ADDR6_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  sizeof(addrIPv6CfgParam_t));
		break;
	case TUNNEL6_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  sizeof(tunnelCfgParam_t));
		break;		
#endif
#endif

#ifdef VOIP_SUPPORT
	case VOIP_T:
		//tr-104 use 
		printf("apimb: mib_set MIB_VOIP_CFG\n");
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, pTbl[i].size);
		break;
#endif

	case WLAC_ARRAY_T:
	
//#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)
	case MESH_ACL_ARRAY_T:
//#endif

	case WDS_ARRAY_T:
	case SCHEDULE_ARRAY_T:
#ifdef HOME_GATEWAY
	case PORTFW_ARRAY_T:
	case IPFILTER_ARRAY_T:
	case PORTFILTER_ARRAY_T:
	case MACFILTER_ARRAY_T:
	case URLFILTER_ARRAY_T:
	case TRIGGERPORT_ARRAY_T:

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
	case QOS_ARRAY_T:
#endif
#ifdef ROUTE_SUPPORT
	case STATICROUTE_ARRAY_T:
#endif
#endif

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
	case IPSECTUNNEL_ARRAY_T:
#endif
#endif
#ifdef TLS_CLIENT
	case CERTROOT_ARRAY_T:
	case CERTUSER_ARRAY_T:
#endif
	case DHCPRSVDIP_ARRY_T:		
#if defined(VLAN_CONFIG_SUPPORTED)	
	case VLANCONFIG_ARRAY_T:		
#endif
#ifdef WLAN_PROFILE
	case PROFILE_ARRAY_T:		
#endif
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif
		return 0;
	default:
		break;
	}
	
#if CONFIG_APMIB_SHARED_MEMORY == 1
    apmib_sem_unlock();
#endif

	return 1;
}


////////////////////////////////////////////////////////////////////////////////
int apmib_setDef(int id, void *value)
{
	int ret;
	APMIB_Tp saveMib=pMib;

	pMib = pMibDef;
	ret = apmib_set(id, value);
	pMib = saveMib;
	return ret;
}



////////////////////////////////////////////////////////////////////////////////
/* Update current used MIB into flash in current setting area
 */
int apmib_update(CONFIG_DATA_T type)
{
	int i, len;
#ifndef MIB_TLV
	int j, k;
#endif
	unsigned char checksum;
	unsigned char *data;
#ifdef MIB_TLV
	unsigned char *pfile = NULL;
	unsigned char *mib_tlv_data = NULL;
	unsigned int tlv_content_len = 0;
	unsigned int mib_tlv_max_len = 0;
#endif

	int write_hw_tlv = 0;

#if CONFIG_APMIB_SHARED_MEMORY == 1
	apmib_sem_lock();
#endif
	if (type & HW_SETTING) {
//	always Write HW setting uncompressed		
//		if(compress_hw_setting == 0)
//		write_hw_tlv = 0;
		
		data = (unsigned char *)pHwSetting;
		checksum = CHECKSUM(data, hsHeader.len-1);
		data[hsHeader.len-1] = checksum;
		len=hsHeader.len;

#ifdef COMPRESS_MIB_SETTING
	if(write_hw_tlv)
	{
#ifdef MIB_TLV
		mib_tlv_max_len = mib_get_setting_len(type)*4;
//fprintf(stderr,"\r\n mib_tlv_max_len = %p, __[%s-%u]",mib_tlv_max_len,__FILE__,__LINE__);

		pfile = malloc(mib_tlv_max_len);
		tlv_content_len = 0;
		
//mib_display_data_content(HW_SETTING, data, hsHeader.len);		

		if(pfile != NULL && mib_tlv_save(type, (void*)data, pfile, &tlv_content_len) == 1)
		{

			mib_tlv_data = malloc(tlv_content_len+1); // 1:checksum
			if(mib_tlv_data != NULL)
			{
				memcpy(mib_tlv_data, pfile, tlv_content_len);
			}
					
			free(pfile);
			pfile = NULL;

		}
		
		if(mib_tlv_data != NULL)
		{
			hsHeader.len = tlv_content_len+1;
			data = mib_tlv_data;
			data[tlv_content_len] = CHECKSUM(data, tlv_content_len);
//mib_display_tlv_content(HW_SETTING, data, hsHeader.len);		
		}

#endif // #ifdef MIB_TLV		
		if( mib_compress_write(type, data) == 1)
		{

		}
#ifdef MIB_TLV
		/*restore len to MIB structure size since len changed to TLV_data length*/
		hsHeader.len = len;
#endif
	}
		else
#endif //#ifedf COMPRESS_MIB_SETTING	

		/*overwrite header*/
		if(compress_hw_setting && (write_hw_tlv == 0))
			flash_write((char *)&hsHeader,HW_SETTING_OFFSET,sizeof(hsHeader));
		
		if ( flash_write((char *)data, HW_SETTING_OFFSET+sizeof(hsHeader), hsHeader.len)==0 ) {
			printf("write hs MIB failed!\n");
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_sem_unlock();
#endif
#ifdef MIB_TLV
	if(write_hw_tlv)	{

			if(mib_tlv_data)
				free(mib_tlv_data);
			
			if(pfile)
				free(pfile);
	}
#endif
			return 0;
		}
	}


	if ((type & CURRENT_SETTING) || (type & DEFAULT_SETTING)) {
		
#ifndef MIB_TLV
		for (j=0; j<NUM_WLAN_INTERFACE; j++) 
			for (k=0; k<(NUM_VWLAN_INTERFACE+1); k++)
		{
			memset( pMib->wlan[j][k].acAddrArray, '\0', MAX_WLAN_AC_NUM*sizeof(MACFILTER_T) );
			for (i=0; i<pMib->wlan[j][k].acNum; i++) {
				get_linkchain(&wlanMacChain[j][k], (void *)&pMib->wlan[j][k].acAddrArray[i], i+1);
			}
			memset( pMib->wlan[j][k].wdsArray, '\0', MAX_WDS_NUM*sizeof(WDS_T) );
			for (i=0; i<pMib->wlan[j][k].wdsNum; i++) {
				get_linkchain(&wdsChain[j][k], (void *)&pMib->wlan[j][k].wdsArray[i], i+1);
			}
			memset( pMib->wlan[j][k].scheduleRuleArray, '\0', MAX_SCHEDULE_NUM*sizeof(SCHEDULE_T) );
			for (i=0; i<pMib->wlan[j][k].scheduleRuleNum; i++) {
				get_linkchain(&scheduleRuleChain[j][k], (void *)&pMib->wlan[j][k].scheduleRuleArray[i], i+1);
			}
		}

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)	// below code copy above ACL code
		memset( pMib->meshAclAddrArray, '\0', MAX_MESH_ACL_NUM*sizeof(MACFILTER_T) );
		for (i=0; i<pMib->meshAclNum; i++) {
			get_linkchain(&meshAclChain, (void *)&pMib->meshAclAddrArray[i], i+1);
		}
#endif

		
#ifdef HOME_GATEWAY
		memset( pMib->portFwArray, '\0', MAX_FILTER_NUM*sizeof(PORTFW_T) );
		for (i=0; i<pMib->portFwNum; i++) {
			get_linkchain(&portFwChain, (void *)&pMib->portFwArray[i], i+1);
		}

		memset( pMib->ipFilterArray, '\0', MAX_FILTER_NUM*sizeof(IPFILTER_T) );
		for (i=0; i<pMib->ipFilterNum; i++) {
			get_linkchain(&ipFilterChain, (void *)&pMib->ipFilterArray[i], i+1);
		}
		memset( pMib->portFilterArray, '\0', MAX_FILTER_NUM*sizeof(PORTFILTER_T) );
		for (i=0; i<pMib->portFilterNum; i++) {
			get_linkchain(&portFilterChain, (void *)&pMib->portFilterArray[i], i+1);
		}
		memset( pMib->macFilterArray, '\0', MAX_FILTER_NUM*sizeof(MACFILTER_T) );
		for (i=0; i<pMib->macFilterNum; i++) {
			get_linkchain(&macFilterChain, (void *)&pMib->macFilterArray[i], i+1);
		}
		memset( pMib->urlFilterArray, '\0', MAX_URLFILTER_NUM*sizeof(URLFILTER_T) );
		for (i=0; i<pMib->urlFilterNum; i++) {
			get_linkchain(&urlFilterChain, (void *)&pMib->urlFilterArray[i], i+1);
		}
		memset( pMib->triggerPortArray, '\0', MAX_FILTER_NUM*sizeof(TRIGGERPORT_T) );
		for (i=0; i<pMib->triggerPortNum; i++) {
			get_linkchain(&triggerPortChain, (void *)&pMib->triggerPortArray[i], i+1);
		}
#ifdef GW_QOS_ENGINE
		memset( pMib->qosRuleArray, '\0', MAX_QOS_RULE_NUM*sizeof(QOS_T) );
		for (i=0; i<pMib->qosRuleNum; i++) {
			get_linkchain(&qosChain, (void *)&pMib->qosRuleArray[i], i+1);
		}
#endif

#ifdef QOS_BY_BANDWIDTH
		memset( pMib->qosRuleArray, '\0', MAX_QOS_RULE_NUM*sizeof(IPQOS_T) );
		for (i=0; i<pMib->qosRuleNum; i++) {
			get_linkchain(&qosChain, (void *)&pMib->qosRuleArray[i], i+1);
		}
#endif

#ifdef ROUTE_SUPPORT
		memset( pMib->staticRouteArray, '\0', MAX_ROUTE_NUM*sizeof(STATICROUTE_T) );
		for (i=0; i<pMib->staticRouteNum; i++) {
			get_linkchain(&staticRouteChain, (void *)&pMib->staticRouteArray[i], i+1);
		}
#endif //ROUTE

#endif

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
		memset( pMib->ipsecTunnelArray, '\0', MAX_TUNNEL_NUM*sizeof(IPSECTUNNEL_T) );
		for (i=0; i<pMib->ipsecTunnelNum; i++) {
			get_linkchain(&ipsecTunnelChain, (void *)&pMib->ipsecTunnelArray[i], i+1);
		}
#endif
#endif
#ifdef TLS_CLIENT
		memset( pMib->certRootArray, '\0', MAX_CERTROOT_NUM*sizeof(CERTROOT_T) );
		for (i=0; i<pMib->certRootNum; i++) {
			get_linkchain(&certRootChain, (void *)&pMib->certRootArray[i], i+1);
		}
		memset( pMib->certUserArray, '\0', MAX_CERTUSER_NUM*sizeof(CERTUSER_T) );
		for (i=0; i<pMib->certUserNum; i++) {
			get_linkchain(&certUserChain, (void *)&pMib->certUserArray[i], i+1);
		}		
#endif
		memset(pMib->dhcpRsvdIpArray, '\0', MAX_DHCP_RSVD_IP_NUM*sizeof(DHCPRSVDIP_T));
		for (i=0; i<pMib->dhcpRsvdIpNum; i++) {
			get_linkchain(&dhcpRsvdIpChain, (void *)&pMib->dhcpRsvdIpArray[i], i+1);
		}
#if defined(VLAN_CONFIG_SUPPORTED)		
		memset(pMib->VlanConfigArray, '\0', MAX_IFACE_VLAN_CONFIG*sizeof(VLAN_CONFIG_T));
		for (i=0; i<pMib->VlanConfigNum; i++) {
			get_linkchain(&vlanConfigChain, (void *)&pMib->VlanConfigArray[i], i+1);
		}
#endif	
#endif /*not def MI B_TLV*/
		if (type & CURRENT_SETTING) {
			data = (unsigned char *)pMib;
			checksum = CHECKSUM(data, csHeader.len-1);
			*(data + csHeader.len - 1) = checksum;
			i = CURRENT_SETTING_OFFSET + sizeof(csHeader);
			len = csHeader.len;
		}
		else {
			data = (unsigned char *)pMibDef;
			checksum = CHECKSUM(data, dsHeader.len-1);
			*(data + dsHeader.len - 1) = checksum;
			i = DEFAULT_SETTING_OFFSET + sizeof(dsHeader);
			len = dsHeader.len;
		}

#ifdef COMPRESS_MIB_SETTING
#ifdef MIB_TLV
		mib_tlv_max_len = mib_get_setting_len(type)*4;


		pfile = malloc(mib_tlv_max_len);
		tlv_content_len = 0;

//mib_display_data_content(type, data, len);

		if(pfile != NULL && mib_tlv_save(type, (void*)data, pfile, &tlv_content_len) == 1)
		{

			mib_tlv_data = malloc(tlv_content_len+1); // 1:checksum
			if(mib_tlv_data != NULL)
			{
				memcpy(mib_tlv_data, pfile, tlv_content_len);
			}
					
			free(pfile);
			pfile = NULL;

		}
		
		if(mib_tlv_data != NULL)
		{
			if (type & CURRENT_SETTING)
				csHeader.len = tlv_content_len+1;
			else
				dsHeader.len = tlv_content_len+1;
			
			data = mib_tlv_data;
			data[tlv_content_len] = CHECKSUM(data, tlv_content_len);
			
//mib_display_tlv_content(type, data, tlv_content_len+1);

		}

#endif // #ifdef MIB_TLV
		if( mib_compress_write(type, data) == 1)
		{

		}	
		else
#endif //#ifedf COMPRESS_MIB_SETTING	
		if ( flash_write((char *)data, i, len)==0 ) {
			printf("Write flash current-setting failed!\n");
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_sem_unlock();
#endif

#ifdef MIB_TLV
			if(mib_tlv_data)
				free(mib_tlv_data);
			
			if(pfile)
				free(pfile);
#endif
			return 0;
		}
#ifdef MIB_TLV
		/*restore len to APMIB_T structure size*/
		if (type & CURRENT_SETTING)
			csHeader.len = len;
		else
			dsHeader.len = len;
#endif
	}
#if CONFIG_APMIB_SHARED_MEMORY == 1
	apmib_sem_unlock();
#endif

#ifdef MIB_TLV
	if(mib_tlv_data)
		free(mib_tlv_data);
	if(pfile)
		free(pfile);
#endif	

	return 1;
}


////////////////////////////////////////////////////////////////////////////////
/* Update default setting MIB into current setting area
 */
int apmib_updateDef(void)
{
	unsigned char *data, checksum;
	PARAM_HEADER_T header;
	int i;
#ifdef MIB_TLV
	unsigned char *pfile = NULL;
	unsigned char *mib_tlv_data = NULL;
	unsigned int tlv_content_len = 0;
	unsigned int mib_tlv_max_len = 0;
#endif

#if CONFIG_APMIB_SHARED_MEMORY == 1
	apmib_sem_lock();
#endif	
	memcpy(header.signature, CURRENT_SETTING_HEADER_TAG, TAG_LEN);
	memcpy(&header.signature[TAG_LEN], &dsHeader.signature[TAG_LEN], SIGNATURE_LEN-TAG_LEN);

	header.len = dsHeader.len;
	data = (unsigned char *)pMibDef;
	checksum = CHECKSUM(data, header.len-1);
	*(data + header.len - 1) = checksum;

	i = CURRENT_SETTING_OFFSET;
#ifdef COMPRESS_MIB_SETTING
#ifdef MIB_TLV
	mib_tlv_max_len = mib_get_setting_len(DEFAULT_SETTING)*4;

	pfile = malloc(mib_tlv_max_len);
	tlv_content_len = 0;

//mib_display_data_content(DEFAULT_SETTING, data, sizeof(APMIB_T));	
	if(pfile != NULL && mib_tlv_save(DEFAULT_SETTING, (void*)data, pfile, &tlv_content_len) == 1)
	{

		mib_tlv_data = malloc(tlv_content_len+1); // 1:checksum
		if(mib_tlv_data != NULL)
		{
			memcpy(mib_tlv_data, pfile, tlv_content_len);
		}
				
		free(pfile);
	}
	
	if(mib_tlv_data != NULL)
	{
		header.len = tlv_content_len+1;
		
		data = mib_tlv_data;
		data[tlv_content_len] = CHECKSUM(data, tlv_content_len);

//mib_display_tlv_content(CURRENT_SETTING, data, tlv_content_len+1);			

	}

#endif // #ifdef MIB_TLV

	if(mib_updateDef_compress_write(CURRENT_SETTING, (char *)data, &header) == 1)
	{
		COMP_TRACE(stderr,"\r\n mib_updateDef_compress_write CURRENT_SETTING DONE, __[%s-%u]", __FILE__,__LINE__);			
	}
	else
	{
#endif

	if ( flash_write((char *)&header, i, sizeof(header))==0 ) {
		printf("Write flash current-setting header failed!\n");
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif
		return 0;
	}
	i += sizeof(header);

	if ( flash_write((char *)data, i, header.len)==0 ) {
		printf("Write flash current-setting failed!\n");
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif
		return 0;
	}
#ifdef COMPRESS_MIB_SETTING
	}
#endif

#if CONFIG_APMIB_SHARED_MEMORY == 1
	apmib_sem_unlock();
#endif	
	return 1;
}


////////////////////////////////////////////////////////////////////////////////
/* Update MIB into flash current setting area
 */
#ifdef COMPRESS_MIB_SETTING
int apmib_updateFlash(CONFIG_DATA_T type, char *data, int len, int force, int ver)
{
	int offset;
	/*since COMPRESS MIB no way to keep old or upgrade mib. so only replaced*/
	if ( type == HW_SETTING ) {
		offset = HW_SETTING_OFFSET;
	}
	else if ( type == DEFAULT_SETTING ) {
		offset = DEFAULT_SETTING_OFFSET;
	}
	else  {
		offset = CURRENT_SETTING_OFFSET;
	}
	/*no care force and ver.*/
	if(0==flash_write(data,offset,len)){
		printf("flash write apmib failed");
		return 0;
	}
	return 1;
}

#else
int apmib_updateFlash(CONFIG_DATA_T type, char *data, int len, int force, int ver)
{
	unsigned char checksum, checksum1, *ptr=NULL;
	int i, offset=0, curLen, curVer;
	unsigned char *pMibData, *pHdr, tmpBuf[20];

#if CONFIG_APMIB_SHARED_MEMORY == 1
	apmib_sem_lock();
#endif

	if ( type == HW_SETTING ) {
		curLen = hsHeader.len - 1;
		pMibData = (unsigned char *)pHwSetting;
		pHdr = (unsigned char *)&hsHeader;
		i = HW_SETTING_OFFSET;
	}
	else if ( type == DEFAULT_SETTING ) {
		curLen = dsHeader.len - 1;
		pMibData = (unsigned char *)pMibDef;
		pHdr = (unsigned char *)&dsHeader;
		i = DEFAULT_SETTING_OFFSET;
	}
	else  {
		curLen = csHeader.len - 1;
		pMibData = (unsigned char *)pMib;
		pHdr = (unsigned char *)&csHeader;
		i = CURRENT_SETTING_OFFSET;
	}

	if (force==2) { // replace by input mib
		((PARAM_HEADER_Tp)pHdr)->len = len + 1;
		sprintf(tmpBuf, "%02d", ver);
		memcpy(&pHdr[TAG_LEN], tmpBuf, SIGNATURE_LEN-TAG_LEN);
		checksum = CHECKSUM(data, len);
		pMibData = data;
		curLen = len;
	}
	else if (force==1) { // update mib but keep not used mib
		sscanf(&((PARAM_HEADER_Tp)pHdr)->signature[TAG_LEN], "%02d", &curVer);
		if ( curVer < ver ) {
			sprintf(tmpBuf, "%02d", ver);
			memcpy(&((PARAM_HEADER_Tp)pHdr)->signature[TAG_LEN],
					tmpBuf, SIGNATURE_LEN-TAG_LEN);
		}
		checksum = CHECKSUM(data, len);
		if (curLen > len) {
			((PARAM_HEADER_Tp)pHdr)->len = curLen + 1;
			ptr = pMibData + len;
			offset = curLen - len;
			checksum1 = CHECKSUM(ptr, offset);
			checksum +=  checksum1;
		}
		else
			((PARAM_HEADER_Tp)pHdr)->len = len + 1;

		curLen = len;
		pMibData = data;
	}
	else { // keep old mib, only update new added portion
		sscanf(&((PARAM_HEADER_Tp)pHdr)->signature[TAG_LEN], "%02d", &curVer);
		if ( curVer < ver ) {
			sprintf(tmpBuf, "%02d", ver);
			memcpy(&((PARAM_HEADER_Tp)pHdr)->signature[TAG_LEN],
					tmpBuf, SIGNATURE_LEN-TAG_LEN);
		}
		if ( len > curLen ) {
			((PARAM_HEADER_Tp)pHdr)->len = len + 1;
			offset = len - curLen;
			checksum = CHECKSUM(pMibData, curLen);
			ptr = data + curLen;
			checksum1 = CHECKSUM(ptr, offset);
			checksum +=  checksum1;
		}
		else
			checksum = CHECKSUM(pMibData, curLen);
	}

	if ( flash_write((char *)pHdr, i, sizeof(PARAM_HEADER_T))==0 ) {
		printf("Write flash current-setting header failed!\n");
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif
		return 0;
	}
	i += sizeof(PARAM_HEADER_T);

	if ( flash_write(pMibData, i, curLen)==0 ) {
		printf("Write flash current-setting failed!\n");
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif
		return 0;
	}
	i += curLen;

	if (offset > 0) {
		if ( flash_write((char *)ptr, i, offset)==0 ) {
			printf("Write flash current-setting failed!\n");
#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_sem_unlock();
#endif
			return 0;
		}
		i += offset;
	}

	if ( flash_write((char *)&checksum, i, sizeof(checksum))==0 ) {
		printf("Write flash current-setting checksum failed!\n");
#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif
		return 0;
	}

#if CONFIG_APMIB_SHARED_MEMORY == 1
	apmib_sem_unlock();
#endif	
	return 1;
}
#endif
/////////////////////////////////////////////////////////////////////////////////
static int flash_read(char *buf, int offset, int len)
{
	int fh;
	int ok=1;

	fh = open(FLASH_DEVICE_NAME, O_RDONLY);
	if ( fh == -1 )
		return 0;

	lseek(fh, offset, SEEK_SET);

	if ( read(fh, buf, len) != len)
		ok = 0;

	close(fh);

	return ok;
}


////////////////////////////////////////////////////////////////////////////////
static int flash_write(char *buf, int offset, int len)
{
	int fh;
	int ok=1;

	fh = open(FLASH_DEVICE_NAME, O_RDWR);

	if ( fh == -1 )
		return 0;

	lseek(fh, offset, SEEK_SET);

	if ( write(fh, buf, len) != len)
		ok = 0;

	close(fh);
	sync();

	return ok;
}

#ifdef MIB_TLV
int add_tblentry(void *pmib, unsigned offset, int num,const mib_table_entry_T *mib_tbl,void *val)
{
	//printf("num %d count %d \n",num,(mib_tbl->total_size/mib_tbl->unit_size));
	if( num >= (mib_tbl->total_size/mib_tbl->unit_size))
		return 0;
	memcpy(pmib+offset+num*mib_tbl->unit_size,val,mib_tbl->unit_size);
	return 1;
}

int delete_tblentry(void *pmib, unsigned offset, int num,const mib_table_entry_T *mib_tbl,void *val)
{
	int i = 0;
	int total_size = mib_tbl->total_size;
	int unit_size = mib_tbl->unit_size;
	if(num > total_size/unit_size)
		num=total_size/unit_size;
	for(i=0;i<num;i++)
	{
		if(0==memcmp(pmib+offset+i*unit_size,val,unit_size))
			break;
	}
	/*not found*/
	if(i == num)
		return 0;
	
	if(i==(num-1))
	{
		memset(pmib+offset+i*unit_size,0x0,unit_size);	
	}
	else
	{
		for(;i<num;i++)
		{
			memcpy(pmib+offset+i*unit_size,pmib+offset+(i+1)*unit_size,unit_size);
		}
	}
	return 1;
}

#if defined(MIB_MOD_TBL_ENTRY) //brucehou
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
int mod_tblentry(void *pmib, unsigned offset, int num,const mib_table_entry_T *mib_tbl,void *val)
{
	int i = 0;
	int total_size = mib_tbl->total_size;
	int unit_size = mib_tbl->unit_size;
	if(num > total_size/unit_size)
		num=total_size/unit_size;
	for(i=0;i<num;i++)
	{
		if(0==memcmp(pmib+offset+i*unit_size,val,unit_size))
			break;
	}

	/*not found*/
	if(i == num)
		return 0;

	memcpy(pmib+offset+i*unit_size,val+unit_size,unit_size);
	return 1;
}
#endif /* #if defined(MIB_MOD_TBL_ENTRY) */

int delete_all_tblentry(void *pmib, unsigned int offset, int num,const mib_table_entry_T *mib_tbl)
{
	memset(pmib+offset,0x0,mib_tbl->total_size);
	return 1;
}
int update_tblentry(void *pmib,unsigned int offset,int num,const mib_table_entry_T *mib_tbl,void *old, void *newone)
{
	int i=0;
	int total_size = mib_tbl->total_size;
	int unit_size = mib_tbl->unit_size;
	if(num > total_size/unit_size)
		return 0;
	for(i=0;i<num;i++)
	{
		if(0==memcmp(pmib+offset+i*unit_size,old,unit_size))
		{
			break;
		}
	}
	if(i == num)
		return 0;

	/*found*/
	memcpy(pmib+offset+i*unit_size,newone,unit_size);
	return 1;

}

int get_tblentry(void *pmib,unsigned int offset,int num,const mib_table_entry_T *mib_tbl,void *val, int index)

{
	if(index > num)
		return 0;
	memcpy(val,pmib+offset+(index-1)*mib_tbl->unit_size,mib_tbl->unit_size);
	return 1;
}


#else
///////////////////////////////////////////////////////////////////////////////
static int init_linkchain(LINKCHAIN_Tp pLinkChain, int size, int num)
{
	FILTER_Tp entry;
	int offset=sizeof(FILTER_Tp)*2;
	char *pBuf;
	int i;

	pLinkChain->realsize = size;

	if (size%4)
		size = (size/4+1)*4;

	pBuf = calloc(num, size+offset);
	if ( pBuf == NULL )
		return 0;

	pLinkChain->buf = pBuf;
	pLinkChain->pUsedList = NULL;
	pLinkChain->pFreeList = NULL;
	entry = (FILTER_Tp)pBuf;

	ADD_LINK_LIST(pLinkChain->pFreeList, entry);
	for (i=1; i<num; i++) {
		entry = (FILTER_Tp)&pBuf[i*(size+offset)];
		ADD_LINK_LIST(pLinkChain->pFreeList, entry);
	}

	pLinkChain->size = size;
	pLinkChain->num = num;
	pLinkChain->usedNum = 0;
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
static int add_linkchain(LINKCHAIN_Tp pLinkChain, char *val)
{
	FILTER_Tp entry;

	// get a free entry
	entry = pLinkChain->pFreeList;
	if (entry == NULL)
		return 0;

	if (entry->next==pLinkChain->pFreeList)
		pLinkChain->pFreeList = NULL;
	else
		pLinkChain->pFreeList = entry->next;

	REMOVE_LINK_LIST(entry);

	// copy content
	memcpy(entry->val, val, pLinkChain->realsize);

	// add to used list
	if (pLinkChain->pUsedList == NULL) {
		ADD_LINK_LIST(pLinkChain->pUsedList, entry);
	}
	else {
		ADD_LINK_LIST(pLinkChain->pUsedList->prev, entry);
	}
	pLinkChain->usedNum++;
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
static int delete_linkchain(LINKCHAIN_Tp pLinkChain, char *val)
{
	FILTER_Tp curEntry=pLinkChain->pUsedList;

	while (curEntry != NULL) {
		if ( !memcmp(curEntry->val,(unsigned char *)val,pLinkChain->compareLen) ) {
				if (curEntry == pLinkChain->pUsedList) {
					if ( pLinkChain->pUsedList->next != pLinkChain->pUsedList )
						pLinkChain->pUsedList = pLinkChain->pUsedList->next;
					else
						pLinkChain->pUsedList = NULL;
				}
				REMOVE_LINK_LIST(curEntry);
				ADD_LINK_LIST(pLinkChain->pFreeList, curEntry);
				pLinkChain->usedNum--;
				return 1;
		}
		if ( curEntry->next == pLinkChain->pUsedList )
		{
			return 0;
		}
		curEntry = curEntry->next;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
static void delete_all_linkchain(LINKCHAIN_Tp pLinkChain)
{
	FILTER_Tp curEntry;

	if (pLinkChain->pUsedList==NULL)
		return;

	// search for matched mac address
	while (pLinkChain->pUsedList) {
		curEntry = pLinkChain->pUsedList;
		if (pLinkChain->pUsedList->next != pLinkChain->pUsedList)
			pLinkChain->pUsedList = pLinkChain->pUsedList->next;
		else
			pLinkChain->pUsedList = NULL;

		REMOVE_LINK_LIST(curEntry);
		ADD_LINK_LIST(pLinkChain->pFreeList, curEntry);
		pLinkChain->usedNum--;
	}
}

///////////////////////////////////////////////////////////////////////////////
static int get_linkchain(LINKCHAIN_Tp pLinkChain, char *val, int index)
{
	FILTER_Tp curEntry=pLinkChain->pUsedList;

	if ( curEntry == NULL || index > pLinkChain->usedNum)
 		return 0;

	while (--index > 0)
        	curEntry = curEntry->next;
	
	memcpy( (unsigned char *)val, curEntry->val, pLinkChain->realsize);

	return 1;
}

int update_linkchain(int fmt, void *Entry_old, void *Entry_new, int type_size)
{
	LINKCHAIN_Tp pLinkChain=NULL;
	FILTER_Tp curEntry;
	void *entry;
	int i; 
	int entry_cmp;
#ifdef HOME_GATEWAY
	 	if(fmt==PORTFW_ARRAY_T){
			pLinkChain = &portFwChain;	 
		}else if(fmt == IPFILTER_ARRAY_T){
			pLinkChain = &ipFilterChain;
		}else if(fmt == PORTFILTER_ARRAY_T){
			pLinkChain = &portFilterChain;
		}else if(fmt == MACFILTER_ARRAY_T){
			pLinkChain = &macFilterChain;
		}else if(fmt == URLFILTER_ARRAY_T){
			pLinkChain = &urlFilterChain;
		}else	if(fmt==TRIGGERPORT_ARRAY_T){
				pLinkChain = &triggerPortChain;
		}else if(fmt==DHCPRSVDIP_ARRY_T){
			pLinkChain = &dhcpRsvdIpChain;			
		}
#ifdef ROUTE_SUPPORT				
		if(fmt==STATICROUTE_ARRAY_T){
		 	pLinkChain = &staticRouteChain;
		}
#endif			
#else
		 if(fmt==DHCPRSVDIP_ARRY_T){
			pLinkChain = &dhcpRsvdIpChain;	
		}		
#endif			

#if defined(VLAN_CONFIG_SUPPORTED)
		 if(fmt==VLANCONFIG_ARRAY_T){
		 	pLinkChain = &vlanConfigChain;	
		}
#endif	
	curEntry = pLinkChain->pUsedList;
	for(i=0;i<pLinkChain->usedNum;i++){
		entry = curEntry->val;
		entry_cmp=memcmp(entry, Entry_old, type_size );
		if(entry_cmp ==0){
		//	fprintf(stderr,"find the entry to update!\n");
			memcpy(entry, Entry_new, type_size);
			break;
		}
		curEntry = curEntry->next;
	}
	return 1;
}
#endif

#ifdef COMPRESS_MIB_SETTING

#define N		 4096	/* size of ring buffer */
#define F		   18	/* upper limit for match_length */
#define THRESHOLD	2   /* encode string into position and length if match_length is greater than this */
static unsigned char *text_buf;	/* ring buffer of size N, with extra F-1 bytes to facilitate string comparison */
#define LZSS_TYPE	unsigned short
#define NIL			N	/* index for root of binary search trees */
struct lzss_buffer {
	unsigned char	text_buf[N + F - 1];
	LZSS_TYPE	lson[N + 1];
	LZSS_TYPE	rson[N + 257];
	LZSS_TYPE	dad[N + 1];
};
static LZSS_TYPE		match_position, match_length;  /* of longest match.  These are set by the InsertNode() procedure. */
static LZSS_TYPE		*lson, *rson, *dad;  /* left & right children & parents -- These constitute binary search trees. */

void InsertNode(LZSS_TYPE r)
	/* Inserts string of length F, text_buf[r..r+F-1], into one of the
	   trees (text_buf[r]'th tree) and returns the longest-match position
	   and length via the global variables match_position and match_length.
	   If match_length = F, then removes the old node in favor of the new
	   one, because the old one will be deleted sooner.
	   Note r plays double role, as tree node and position in buffer. */
{
	LZSS_TYPE  i, p, cmp;
	unsigned char  *key;

	cmp = 1;
	key = &text_buf[r];
	p = N + 1 + key[0];
	rson[r] = lson[r] = NIL;
	match_length = 0;
	while(1) {
		if (cmp >= 0) {
			if (rson[p] != NIL)
				p = rson[p];
			else {
				rson[p] = r;
				dad[r] = p;
				return;
			}
		} else {
			if (lson[p] != NIL)
				p = lson[p];
			else {
				lson[p] = r;
				dad[r] = p;
				return;
			}
		}
		for (i = 1; i < F; i++)
			if ((cmp = key[i] - text_buf[p + i]) != 0)
				break;
		if (i > match_length) {
			match_position = p;
			if ((match_length = i) >= F)
				break;
		}
	}
	dad[r] = dad[p];
	lson[r] = lson[p];
	rson[r] = rson[p];
	dad[lson[p]] = r;
	dad[rson[p]] = r;
	if (rson[dad[p]] == p)
		rson[dad[p]] = r;
	else
		lson[dad[p]] = r;
	dad[p] = NIL;  /* remove p */
}

void InitTree(void)  /* initialize trees */
{
	int  i;

	/* For i = 0 to N - 1, rson[i] and lson[i] will be the right and
	   left children of node i.  These nodes need not be initialized.
	   Also, dad[i] is the parent of node i.  These are initialized to
	   NIL (= N), which stands for 'not used.'
	   For i = 0 to 255, rson[N + i + 1] is the root of the tree
	   for strings that begin with character i.  These are initialized
	   to NIL.  Note there are 256 trees. */

	for (i = N + 1; i <= N + 256; i++)
		rson[i] = NIL;
	for (i = 0; i < N; i++)
		dad[i] = NIL;
}

void DeleteNode(LZSS_TYPE p)  /* deletes node p from tree */
{
	LZSS_TYPE  q;
	
	if (dad[p] == NIL)
		return;  /* not in tree */
	if (rson[p] == NIL)
		q = lson[p];
	else if (lson[p] == NIL)
		q = rson[p];
	else {
		q = lson[p];
		if (rson[q] != NIL) {
			do {
				q = rson[q];
			} while (rson[q] != NIL);
			rson[dad[q]] = lson[q];
			dad[lson[q]] = dad[q];
			lson[q] = lson[p];
			dad[lson[p]] = q;
		}
		rson[q] = rson[p];
		dad[rson[p]] = q;
	}
	dad[q] = dad[p];
	if (rson[dad[p]] == p)
		rson[dad[p]] = q;
	else
		lson[dad[p]] = q;
	dad[p] = NIL;
}
int Encode(unsigned char *ucInput, unsigned int inLen, unsigned char *ucOutput)
{
	
#if defined(CHEAT_COMPRESS_MIB_SETTING)
	memcpy(ucOutput, ucInput, inLen);
	return inLen;
#else
	
	LZSS_TYPE  i, len, r, s, last_match_length, code_buf_ptr;
	unsigned char c;
	unsigned char  code_buf[17], mask;
	unsigned int ulPos=0;
	int enIdx=0;

	struct lzss_buffer *lzssbuf;

	if (0 != (lzssbuf = malloc(sizeof(struct lzss_buffer)))) {
		memset(lzssbuf, 0, sizeof(struct lzss_buffer));
		text_buf = lzssbuf->text_buf;
		rson = lzssbuf->rson;
		lson = lzssbuf->lson;
		dad = lzssbuf->dad;
	} else {
		return 0;
	}
	
	InitTree();  /* initialize trees */
	code_buf[0] = 0;  /* code_buf[1..16] saves eight units of code, and
		code_buf[0] works as eight flags, "1" representing that the unit
		is an unencoded letter (1 byte), "0" a position-and-length pair
		(2 bytes).  Thus, eight units require at most 16 bytes of code. */
	code_buf_ptr = mask = 1;
	s = 0;
	r = N - F;
	for (i = s; i < r; i++)
		text_buf[i] = ' ';  /* Clear the buffer with
		any character that will appear often. */

	for (len = 0; (len < F) && ulPos < inLen; len++)
		text_buf[r + len] = ucInput[ulPos++];  /* Read F bytes into the last F bytes of the buffer */
	
	//if ((textsize = len) == 0) return;  /* text of size zero */
	if (len == 0) {
		enIdx = 0;
		goto finished;
	}
	
	for (i = 1; i <= F; i++)
		InsertNode(r - i);  /* Insert the F strings,
		each of which begins with one or more 'space' characters.  Note
		the order in which these strings are inserted.  This way,
		degenerate trees will be less likely to occur. */
	InsertNode(r);  /* Finally, insert the whole string just read.  The
		global variables match_length and match_position are set. */
	do {
		if (match_length > len) match_length = len;  /* match_length
			may be spuriously long near the end of text. */
		if (match_length <= THRESHOLD) {
			match_length = 1;  /* Not long enough match.  Send one byte. */
			code_buf[0] |= mask;  /* 'send one byte' flag */
			code_buf[code_buf_ptr++] = text_buf[r];  /* Send uncoded. */
		} else {
			code_buf[code_buf_ptr++] = (unsigned char) match_position;
			code_buf[code_buf_ptr++] = (unsigned char)
				(((match_position >> 4) & 0xf0)
			  | (match_length - (THRESHOLD + 1)));  /* Send position and
					length pair. Note match_length > THRESHOLD. */
		}
		if ((mask <<= 1) == 0) {  /* Shift mask left one bit. */
			for (i = 0; i < code_buf_ptr; i++)  /* Send at most 8 units of */
				ucOutput[enIdx++]=code_buf[i];
			//codesize += code_buf_ptr;
			code_buf[0] = 0;  code_buf_ptr = mask = 1;
		}
		last_match_length = match_length;

		for (i = 0; i< last_match_length && 
			ulPos < inLen; i++){
			c = ucInput[ulPos++];
			DeleteNode(s);		/* Delete old strings and */
			text_buf[s] = c;	/* read new bytes */
			if (s < F - 1)
				text_buf[s + N] = c;  /* If the position is near the end of buffer, extend the buffer to make string comparison easier. */
			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
				/* Since this is a ring buffer, increment the position
				   modulo N. */
			InsertNode(r);	/* Register the string in text_buf[r..r+F-1] */
		}
		
		while (i++ < last_match_length) {	/* After the end of text, */
			DeleteNode(s);					/* no need to read, but */
			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
			if (--len) InsertNode(r);		/* buffer may not be empty. */
		}
	} while (len > 0);	/* until length of string to be processed is zero */
	if (code_buf_ptr > 1) {		/* Send remaining code. */
		for (i = 0; i < code_buf_ptr; i++) 
			ucOutput[enIdx++]=code_buf[i];
		//codesize += code_buf_ptr;
	}
finished:
	free(lzssbuf);
	return enIdx;
	
#endif //#if defined(CHEAT_COMPRESS_MIB_SETTING)
}

int Decode(unsigned char *ucInput, unsigned int inLen, unsigned char *ucOutput)	/* Just the reverse of Encode(). */
{
	
#if defined(CHEAT_COMPRESS_MIB_SETTING)
	memcpy(ucOutput, ucInput, inLen);
	return inLen;
#else
	int  i, j, k, r, c;
	unsigned int  flags;
	unsigned int ulPos=0;
	unsigned int ulExpLen=0;

	if ((text_buf = malloc( N + F - 1 )) == 0) {
		//fprintf(stderr, "fail to get mem %s:%d\n", __FUNCTION__, __LINE__);
		return 0;
	}
	
	for (i = 0; i < N - F; i++)
		text_buf[i] = ' ';
	r = N - F;
	flags = 0;
	while(1) {
		if (((flags >>= 1) & 256) == 0) {
			c = ucInput[ulPos++];
			if (ulPos>inLen)
				break;
			flags = c | 0xff00;		/* uses higher byte cleverly */
		}							/* to count eight */
		if (flags & 1) {
			c = ucInput[ulPos++];
			if ( ulPos > inLen )
				break;
			ucOutput[ulExpLen++] = c;
			text_buf[r++] = c;
			r &= (N - 1);
		} else {
			i = ucInput[ulPos++];
			if ( ulPos > inLen ) break;
			j = ucInput[ulPos++];
			if ( ulPos > inLen ) break;
			
			i |= ((j & 0xf0) << 4);
			j = (j & 0x0f) + THRESHOLD;
			for (k = 0; k <= j; k++) {
				c = text_buf[(i + k) & (N - 1)];
				ucOutput[ulExpLen++] = c;
				text_buf[r++] = c;
				r &= (N - 1);
			}
		}
	}

	free(text_buf);
	return ulExpLen;
#endif //#if defined(CHEAT_COMPRESS_MIB_SETTING)	
}

int mib_get_flash_offset(CONFIG_DATA_T type)
{
	switch(type)
	{
		case HW_SETTING:
			return HW_SETTING_OFFSET;
		case DEFAULT_SETTING:
			return DEFAULT_SETTING_OFFSET;			
		case CURRENT_SETTING:
			return CURRENT_SETTING_OFFSET;
		default:
			return -1;
	}
	
}

unsigned int mib_get_real_len(CONFIG_DATA_T type)
{
	switch(type)
	{
		case HW_SETTING:
			return HW_SETTING_SECTOR_LEN;
		case DEFAULT_SETTING:
			return DEFAULT_SETTING_SECTOR_LEN;			
		case CURRENT_SETTING:
			return CURRENT_SETTING_SECTOR_LEN;
		default:			
			return 0;
	}
	
}

unsigned int mib_get_setting_len(CONFIG_DATA_T type)
{
	switch(type)
	{
		case HW_SETTING:
			return sizeof(HW_SETTING_T);
		case DEFAULT_SETTING:
		case CURRENT_SETTING:
			return sizeof(APMIB_T);
		default:			
			return 0;
	}
	
}
PARAM_HEADER_T* mib_get_header(CONFIG_DATA_T type)
{
	
	switch(type)
	{
		case HW_SETTING:
			return &hsHeader;
		case DEFAULT_SETTING:
			return &dsHeader;
		case CURRENT_SETTING:
			return &csHeader;
		default :
			return NULL;
		
	}

}
unsigned int mib_compress_write(CONFIG_DATA_T type, unsigned char *data)
{
	unsigned char* pContent = NULL;

	COMPRESS_MIB_HEADER_T compHeader;
	unsigned char *expPtr, *compPtr;
	unsigned int expLen = 0;
	unsigned int compLen;
	unsigned int real_size = 0;
	PARAM_HEADER_T *pheader;
	int dst;




	dst = mib_get_flash_offset(type);
	real_size = mib_get_flash_offset(type);
	pheader = mib_get_header(type);
	expLen = pheader->len+sizeof(PARAM_HEADER_T);

	if( (compPtr = malloc(real_size)) == NULL)
	{
		printf("\r\n ERR!! malloc  size %u failed! __[%s-%u]\n",real_size,__FILE__,__LINE__);
	}
	if( (expPtr = malloc(expLen)) == NULL)
	{
		printf("\r\n ERR!! malloc  size %u failed! __[%s-%u]\n",expLen,__FILE__,__LINE__);
		if(compPtr != NULL)
			free(compPtr);
	}

	if(compPtr != NULL && expPtr!= NULL)
	{
		pContent = &expPtr[sizeof(PARAM_HEADER_T)];	// point to start of MIB data 

		memcpy(pContent, data, pheader->len);
		memcpy(expPtr, pheader, sizeof(PARAM_HEADER_T));

		compLen = Encode(expPtr, expLen, compPtr+sizeof(COMPRESS_MIB_HEADER_T));
		
		if(type == HW_SETTING)
			sprintf((char *)compHeader.signature,"%s",COMP_HS_SIGNATURE);
		else if(type == DEFAULT_SETTING)
			sprintf((char *)compHeader.signature,"%s",COMP_DS_SIGNATURE);
		else
			sprintf((char *)compHeader.signature,"%s",COMP_CS_SIGNATURE);

		compHeader.compRate = (expLen/compLen)+1;
		compHeader.compLen = compLen;
		memcpy(compPtr, &compHeader, sizeof(COMPRESS_MIB_HEADER_T));

		if ( flash_write((char *)compPtr, dst, compLen+sizeof(COMPRESS_MIB_HEADER_T))==0 ) {
			COMP_TRACE(stderr,"Write flash compress setting[%u] failed![%s-%u]\n",type,__FILE__,__LINE__);
			
			if(expPtr)
				free(expPtr);
			if(compPtr)
				free(compPtr);
			return 0;
		}
		else
		{
			COMP_TRACE(stderr,"\r\n Compress [%u] to [%u]. Compress rate=%u, __[%s-%u]",expLen,compLen,compHeader.compRate ,__FILE__,__LINE__);

			COMP_TRACE(stderr,"\r\n Flash write to 0x%x len=%d\n",dst,compLen+sizeof(COMPRESS_MIB_HEADER_T));

			COMP_TRACE(stderr,"\r\n");
			if(expPtr)
				free(expPtr);
			if(compPtr)
				free(compPtr);
			return 1;
		}
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);			
	}

	if(expPtr)
		free(expPtr);
	if(compPtr)
		free(compPtr);
	return 0;
}

int mib_updateDef_compress_write(CONFIG_DATA_T type, char *data, PARAM_HEADER_T *pheader)
{
	unsigned char* pContent = NULL;

	COMPRESS_MIB_HEADER_T compHeader;
	unsigned char *expPtr, *compPtr;
	unsigned int expLen = 0;
	unsigned int compLen;
	unsigned int real_size = 0;
	//int zipRate=0;
	char *pcomp_sig;
	int dst = mib_get_flash_offset(type);

	if(dst == 0)
	{
		printf("\r\n ERR!! no flash offset! __[%s-%u]\n",__FILE__,__LINE__);
		return 0;
	}
	
	switch(type)
	{
		case HW_SETTING:
			pcomp_sig = COMP_HS_SIGNATURE;
			break;
		case DEFAULT_SETTING:
			pcomp_sig = COMP_DS_SIGNATURE;
			break;
		case CURRENT_SETTING:
			pcomp_sig = COMP_CS_SIGNATURE;
			break;
		default:
			printf("\r\n ERR!! no type match __[%s-%u]\n",__FILE__,__LINE__);
			return 0;

	}
	expLen = pheader->len+sizeof(PARAM_HEADER_T);
	if(expLen == 0)
	{
		printf("\r\n ERR!! no expLen! __[%s-%u]\n",__FILE__,__LINE__);
		return 0;
	}
	real_size = mib_get_real_len(type);
	if(real_size == 0)
	{
		printf("\r\n ERR!! no expLen! __[%s-%u]\n",__FILE__,__LINE__);
		return 0;
	}
	
	if( (compPtr = malloc(real_size)) == NULL)
	{
		printf("\r\n ERR!! malloc  size %u failed! __[%s-%u]\n",real_size,__FILE__,__LINE__);
	}

	if( (expPtr = malloc(expLen)) == NULL)
	{
		printf("\r\n ERR!! malloc  size %u failed! __[%s-%u]\n",expLen,__FILE__,__LINE__);
		if(compPtr != NULL)
			free(compPtr);
	}
	
	if(compPtr != NULL && expPtr!= NULL)
	{
		//int status;
		pContent = &expPtr[sizeof(PARAM_HEADER_T)];	// point to start of MIB data 

		memcpy(pContent, data, pheader->len);
		memcpy(expPtr, pheader, sizeof(PARAM_HEADER_T));

		compLen = Encode(expPtr, expLen, compPtr+sizeof(COMPRESS_MIB_HEADER_T));
		sprintf((char *)compHeader.signature,"%s",pcomp_sig);
		compHeader.compRate = (expLen/compLen)+1;
		compHeader.compLen = compLen;
		memcpy(compPtr, &compHeader, sizeof(COMPRESS_MIB_HEADER_T));

		if ( flash_write((char *)compPtr, dst, compLen+sizeof(COMPRESS_MIB_HEADER_T))==0 )
//		if ( write(fh, (const void *)compPtr, compLen+sizeof(COMPRESS_MIB_HEADER_T))!=compLen+sizeof(COMPRESS_MIB_HEADER_T) ) 
		{
			printf("Write flash compress [%u] setting failed![%s-%u]\n",type,__FILE__,__LINE__);			
			return 0;
		}
		
		if(compPtr != NULL)
			free(compPtr);
		if(expPtr != NULL)
			free(expPtr);

		return 1;
			
	}
	else
	{
		return 0;
	}

	return 0;
}
#endif //#ifdef COMPRESS_MIB_SETTING

int save_cs_to_file(void)
{
	char *buf, *ptr=NULL;
	PARAM_HEADER_Tp pHeader;
	//unsigned char checksum;
	int len, fh;
	char tmpBuf[100];
#ifdef COMPRESS_MIB_SETTING
#ifdef MIB_TLV
	int tlv_content_len,compLen;
	unsigned char *pCompptr;
	COMPRESS_MIB_HEADER_Tp pcompHeader;
#endif
#endif

#ifdef MIB_TLV
	len=mib_get_setting_len(CURRENT_SETTING)*4;
#else
	len = csHeader.len;
#endif

#ifdef _LITTLE_ENDIAN_
	//len  = WORD_SWAP(len);
#endif
	len += sizeof(PARAM_HEADER_T);
	buf = malloc(len);
	if ( buf == NULL ) {
		strcpy(tmpBuf, "Allocate buffer failed!");
		return 0;
	}
//	fprintf(stderr,"%s %d\n",__FUNCTION__,__LINE__);
#ifdef __mips__
	fh = open("/web/config.dat", O_RDWR|O_CREAT|O_TRUNC);
#else
	fh = open("/web/config.dat", O_RDWR|O_CREAT|O_TRUNC);
#endif
	if (fh == -1) {
		printf("Create config file error!\n");
		free(buf);
		fprintf(stderr,"%s %d\n",__FUNCTION__,__LINE__);
		return 0;
	}

	pHeader = (PARAM_HEADER_Tp)buf;	
#ifdef MIB_TLV
#else
	len = pHeader->len = csHeader.len;
	memcpy(&buf[sizeof(PARAM_HEADER_T)], pMib, len-1);
#endif

#ifdef _LITTLE_ENDIAN_
#ifdef VOIP_SUPPORT
	// rock: need swap here 
	// 1. write to share space (ex: save setting to config file)
	// 2. read from share space (ex: import config file) 
	pHeader->len  = WORD_SWAP(pHeader->len);
#else
	//pHeader->len  = WORD_SWAP(pHeader->len);
#endif
	swap_mib_word_value((APMIB_Tp)&buf[sizeof(PARAM_HEADER_T)]);
#endif
	memcpy(pHeader->signature, csHeader.signature, SIGNATURE_LEN);
	ptr = (char *)&buf[sizeof(PARAM_HEADER_T)];
	
#ifdef COMPRESS_MIB_SETTING
#ifdef MIB_TLV
	//fprintf(stderr,"%s %d\n",__FUNCTION__,__LINE__);

	tlv_content_len=0;
	if(mib_tlv_save(CURRENT_SETTING, (void*)pMib, (unsigned char *)ptr, (unsigned int *)&tlv_content_len) == 1){
		if(tlv_content_len >= (mib_get_setting_len(CURRENT_SETTING)*4)){
			printf("TLV Data len is too long");
			close(fh);
			free(buf);
			//fprintf(stderr,"%s %d tlv_content_len 0x%x len 0x%x\n",__FUNCTION__,__LINE__,tlv_content_len,len);
			return 0;
		}
		ptr[tlv_content_len] = CHECKSUM((unsigned char *)ptr, tlv_content_len);	
		pHeader->len=tlv_content_len+1; /*add checksum*/
	}

	/*compress*/
	pCompptr = malloc((WEB_PAGE_OFFSET-CURRENT_SETTING_OFFSET)+sizeof(COMPRESS_MIB_HEADER_T));
	if(NULL == pCompptr){
			printf("malloc for Compress buffer failed!! \n");
			close(fh);
			free(buf);
			//fprintf(stderr,"%s %d\n",__FUNCTION__,__LINE__);
			return 0;
	}
	compLen = Encode((unsigned char *)buf, pHeader->len+sizeof(PARAM_HEADER_T), pCompptr+sizeof(COMPRESS_MIB_HEADER_T));
	pcompHeader=(COMPRESS_MIB_HEADER_Tp)pCompptr;
	memcpy(pcompHeader->signature,COMP_CS_SIGNATURE,COMP_SIGNATURE_LEN);
	pcompHeader->compRate = (pHeader->len/compLen)+1;
	pcompHeader->compLen = compLen;
#endif	
#endif

#ifdef MIB_TLV
	//fprintf(stderr,"%s %d compLen %d\n",__FUNCTION__,__LINE__,compLen);

	if ( write(fh, pCompptr, compLen+sizeof(COMPRESS_MIB_HEADER_T)) != compLen+sizeof(COMPRESS_MIB_HEADER_T)) {
		printf("Write config file error!\n");
		close(fh);
		free(pCompptr);
		free(buf);
		fprintf(stderr,"%s %d\n",__FUNCTION__,__LINE__);
		return 0;
	}
	//fprintf(stderr,"%s %d compLen %d\n",__FUNCTION__,__LINE__,compLen);
#else
	checksum = CHECKSUM(ptr, len-1);
	buf[sizeof(PARAM_HEADER_T)+len-1] = checksum;

	ptr = &buf[sizeof(PARAM_HEADER_T)];
	ENCODE_DATA(ptr, len);


	if ( write(fh, buf, len+sizeof(PARAM_HEADER_T)) != len+sizeof(PARAM_HEADER_T)) {
		printf("Write config file error!\n");
		close(fh);
		free(buf);
		return 0;
	}
#endif

	
	//fprintf(stderr,"%s %d compLen %d\n",__FUNCTION__,__LINE__,compLen);
close(fh);
sync();

#ifdef MIB_TLV	
	if(pCompptr) {
		free(pCompptr);
		pCompptr=NULL;
	}
#endif	
	free(buf);

	return 1;
}
//#endif // WEBS


#ifdef MIB_TLV

int mib_search_by_id(const mib_table_entry_T *mib_tbl, unsigned short mib_id, unsigned char *pmib_num, const mib_table_entry_T **ppmib, unsigned int *offset)
{
	int i=0;
	const mib_table_entry_T *mib;
	unsigned short mib_num=0;
	
	memcpy(&mib_num, pmib_num, 1);
	
//printf("\r\n search mib_id=%u, offset=%u, mib_num=%u",mib_id,*offset, mib_num);
	
	
	for (i=0; mib_tbl[i].id; i++)
	{
//printf("\r\n mib_tbl[%u].mib_name=%s",i, mib_tbl[i].mib_name);
		mib = &mib_tbl[i];
		
		if(mib_id == mib_tbl[i].id)
		{
			*offset += mib->offset + mib->unit_size*mib_num;
			*ppmib = mib;
//printf("\r\n !! FIND at %s TBL !!",mib_tbl[i].mib_name);
			return 1;
		}
		else
		{
			if(mib_tbl[i].type >= TABLE_LIST_T)
			{
				
				if((mib->total_size%mib->unit_size) == 0 && mib_num < (mib->total_size /mib->unit_size))
				{					
					*offset += mib->offset + mib->unit_size*mib_num;
//printf("\r\n >> Entry %s TBL >>",mib->mib_name);
					if(mib_search_by_id(mib->next_mib_table, mib_id, pmib_num+1, ppmib, offset) == 1)
					{
						return 1;
					}
					else
					{
//printf("\r\n << Leave %s TBL <<",mib->mib_name);
						*offset -= mib->offset + mib->unit_size*mib_num;
					}
				}
				
			}
		}
	}
	return 0;
}

static int mib_init_value(unsigned char *ptlv_data_value, unsigned short tlv_len, const mib_table_entry_T *mib_tbl, void *data)
{
	unsigned int vInt;
	unsigned short vShort;
	unsigned char *pChar;
	
	
	
#if 0
int j=0;
fprintf(stderr,"\r\n mib_tbl->type = %u",mib_tbl->type);
fprintf(stderr,"\r\n %s = ",mib_tbl->name);
for(j=0; j<tlv_len; j++)
	fprintf(stderr,"0x%x_", *(ptlv_data_value+j));
fprintf(stderr,"\r\n");
#endif
			
	switch (mib_tbl->type)
	{
		case BYTE_T:
		case BYTE_ARRAY_T:
			pChar = (unsigned char *) data;
			memcpy(data, ptlv_data_value, tlv_len);
						
			break;
			
		case WORD_T:
			pChar = (unsigned char *) data;
			memcpy(&vShort, ptlv_data_value, sizeof(vShort));
			memcpy(data, &vShort, sizeof(vShort));
			break;
			
		case DWORD_T:
			pChar = (unsigned char *) data;
			memcpy(&vInt, ptlv_data_value, sizeof(vInt));
			memcpy(data, &vInt, sizeof(vInt));
			break;
			
		case STRING_T:
			pChar = (unsigned char *) data;
			strncpy((char *)pChar, (char *)ptlv_data_value, mib_tbl->total_size);
			break;
			
		case IA_T:
			pChar = (unsigned char *) data;
			memcpy(data, ptlv_data_value, mib_tbl->total_size);
			
			break;
			
		case BYTE5_T:
		case BYTE6_T:
		case BYTE13_T:
			pChar = (unsigned char *) data;
			memcpy(data, ptlv_data_value, mib_tbl->total_size); // avoid alignment issue
			
			break;
			
#ifdef HOME_GATEWAY
#ifdef CONFIG_IPV6
		case RADVDPREFIX_T:
		case DNSV6_T:
		case DHCPV6S_T:
		case ADDR6_T:
		case TUNNEL6_T:
			memcpy(data, ptlv_data_value, mib_tbl->total_size);
			break;		
#endif
#endif
#ifdef VOIP_SUPPORT
#ifdef VOIP_SUPPORT_TLV_CFG
		case VOIP_T:
			break;
#else
		case VOIP_T:
			pChar = (unsigned char *) data;
			memcpy(data, ptlv_data_value, tlv_len);
			break;
#endif
#endif
		default :
			fprintf(stderr,"\r\n ERR!no mib_name[%s] type[%u]. __[%s-%u]",mib_tbl->name, mib_tbl->type,__FILE__,__LINE__);			
			return 0;
		
		
	}
		
	return 1;
}

unsigned short find_same_tag_times(unsigned char *pdata_array, unsigned short data_size)
{
	unsigned short first_tlv_tag;
	unsigned short tlv_tag;	
	unsigned short tlv_len;
	unsigned short times=0;
	unsigned char *idx = pdata_array;
	int i=0;
	
	memcpy(&first_tlv_tag, idx, sizeof(first_tlv_tag));
	
	while(i<data_size)
	{
		memcpy(&tlv_tag, idx+i, sizeof(tlv_tag));
		i+=sizeof(tlv_tag);			
		
		if(tlv_tag == first_tlv_tag)
			times++;
		
		memcpy(&tlv_len, idx+i, sizeof(tlv_len));
		i+=sizeof(tlv_len);
		
		i+=tlv_len;
		
	}
	
	return times;
}

/* TLV to --> Data structure */
unsigned int mib_tlv_init_from(const mib_table_entry_T *mib_root_tbl, unsigned char *pdata_array, void *pfile, unsigned int data_size, unsigned int *pmib_root_offset)
{	
	unsigned char *idx;		
	int i;
	//int j;
	unsigned short tlv_tag;		
	unsigned short tlv_len;	
	unsigned short tlv_num;	
	unsigned char tlv_data_value[1000];	
	unsigned char *ptlv_data_value=tlv_data_value;		
	//unsigned int offset=0;		
	unsigned char mib_num[10];	
	unsigned char *pmib_num = mib_num;	
	memset(mib_num, 0x00, sizeof(mib_num));		

#if 0
int kk;
idx=pdata_array;	
fprintf(stderr,"\r\n data_size=%u \r\n",data_size);	
for(kk=0; kk<data_size; kk++)	
{		
fprintf(stderr,"0x%02x_", *(idx+kk));		
if((kk+1)%10==0) fprintf(stderr,"\r\n");
}	
fprintf(stderr,"\r\n");
#endif		

	i=0;	
	idx=pdata_array;	
	while(i<data_size)	
	{		
		memcpy(&tlv_tag, idx+i, sizeof(tlv_tag));		
		i+=sizeof(tlv_tag);					

		memcpy(&tlv_len, idx+i, sizeof(tlv_len));
		i+=sizeof(tlv_len);

#ifdef VOIP_SUPPORT
#ifdef VOIP_SUPPORT_TLV_CFG
		if( tlv_tag == MIB_VOIP_CFG ) 
		{
			unsigned int mib_offset=0;
			const mib_table_entry_T *pmib_tbl;

			if( mib_search_by_id(mib_root_tbl, tlv_tag, pmib_num, &pmib_tbl, &mib_offset) != 1) {
				printf("\r\n%s(%d) tag:0x%04x, len:0x%04x\n",__FUNCTION__,__LINE__,tlv_tag,tlv_len);
				
                i+=tlv_len;
                continue;
			}

			voip_mib_cfg_init_from_tlv(	mibtbl_voip, pdata_array+i, 
										pfile, tlv_len, &mib_offset );
			i+=tlv_len;				

			/* voip mib has done, move forward */
			continue;
		}
#endif
#endif

		if((tlv_tag & MIB_TABLE_LIST) == 0) // NO member		
		{
			const mib_table_entry_T *mib_tbl;
			unsigned int mib_offset=0;

			if((ptlv_data_value=malloc(tlv_len)) == NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR! malloc fail. tlv_tag=%p, tlv_len=%u __[%s-%u]",tlv_tag, tlv_len, __FILE__,__LINE__);
				return 0;
			}
			memcpy(ptlv_data_value, idx+i, tlv_len);
			i+=tlv_len;

#if 0
fprintf(stderr,"\r\n NO member >>");
fprintf(stderr,"\r\n tlv_tag=0x%x", tlv_tag);
fprintf(stderr,"\r\n tlv_len=0x%x", tlv_len);
fprintf(stderr,"\r\n tlv_value=");
for(j=0; j<tlv_len; j++)
	fprintf(stderr,"0x%x_", *(ptlv_data_value+j));
fprintf(stderr,"\r\n");
#endif
			if( mib_search_by_id(mib_root_tbl, tlv_tag, pmib_num, &mib_tbl, &mib_offset) != 1)			
			{
				//fprintf(stderr,"\r\n Can't find mib_id=%u",tlv_tag);
			}
			else
			{
//fprintf(stderr,"\r\n %u+%u",mib_offset,*pmib_root_offset );
				mib_offset += *pmib_root_offset;
				if(mib_tbl != NULL)
				{
				//fprintf(stderr,"\r\n");
#if 0 // TLV_DEBUG
int k;
unsigned char *pChar = ptlv_data_value;
fprintf(stderr,"\r\n >>mib_tbl->name=%s",mib_tbl->name);	
fprintf(stderr,"\r\n >>>tlv_tag=0x%x, tlv_len=0x%x, tlv_value = ",tlv_tag,tlv_len);
for(k=0; k< tlv_len; k++)
{	
fprintf(stderr,"0x%02x_", *(pChar+k));	
if( (k+1)%10 == 0) fprintf(stderr,"\r\n");
}
fprintf(stderr,"\r\n");
#endif
					if(mib_init_value(ptlv_data_value, tlv_len, mib_tbl, (void *)((int) pfile + mib_offset)) != 1)
					{
						fprintf(stderr,"\r\n Assign mib_name[%s] fail!", mib_tbl->name);
						fprintf(stderr,"\r\n mibtbl->id (%08x) unitsize (%d) totoal size (%d) mibtbl->nextbl %p",mib_tbl->id,mib_tbl->unit_size,mib_tbl->total_size,mib_tbl->next_mib_table);
					}
					else
					{
                        /* Done */
						if(ptlv_data_value != NULL) {
							free(ptlv_data_value);
                            ptlv_data_value = NULL;
                        } 
						//mib_display_value(mib_tbl, (void *)((int) pmib_data + mib_offset));
					}
				}
			}
		}
		else // have member		
		{			
			int j=0;
			const mib_table_entry_T *pmib_tbl;
			unsigned int mib_offset=0;

			if( mib_search_by_id(mib_root_tbl, tlv_tag, pmib_num, &pmib_tbl, &mib_offset) != 1)			
			{
				//fprintf(stderr,"\r\n Can't find mib_id=%u",tlv_tag);		
				i+=tlv_len;	
			}
			else
			{
				if((ptlv_data_value=malloc(tlv_len)) == NULL)
				{
					COMP_TRACE(stderr,"\r\n ERR! malloc fail. tlv_tag=%p, tlv_len=%u __[%s-%u]",tlv_tag, tlv_len, __FILE__,__LINE__);
					return 0;
				}
				
				memcpy(ptlv_data_value, idx+i, tlv_len);
				tlv_num = find_same_tag_times(ptlv_data_value, tlv_len);
//fprintf(stderr,"\r\n tlv_num=%u __[%s-%u]", tlv_num,  __FILE__,__LINE__);
				if(tlv_num != 0)
					tlv_len = tlv_len/tlv_num;

				while(j<tlv_num)
				{
//fprintf(stderr,"\r\n TREE_NODE %s[%u] ENTRY",pmib_tbl->name,j);
					memcpy(ptlv_data_value, idx+i+(tlv_len*j), tlv_len);
					//printf("\r\n %u<%u/%u",j,pmib_tbl->total_size, pmib_tbl->unit_size );
					if( j < (pmib_tbl->total_size / pmib_tbl->unit_size))
					{
						unsigned int mib_tlb_offset=0;
//fprintf(stderr,"\r\n  __[%s-%u]",  __FILE__,__LINE__);
						//printf("\r\n %u+%u+%u*%u",mib_offset,*pmib_root_offset, j,pmib_tbl->unit_size );
						mib_tlb_offset = mib_offset + *pmib_root_offset+j*(pmib_tbl->unit_size);
						//printf("\r\n TREE_NODE name =%s[%u] mib_tbl_offset is %u",pmib_tbl->mib_name,  j, mib_tlb_offset);
						//printf("\r\n tlv_len=%u __[%s-%u]", tlv_len,  __FILE__,__LINE__);
						mib_tlv_init_from(pmib_tbl->next_mib_table, ptlv_data_value, pfile, tlv_len, &mib_tlb_offset);
					}
//fprintf(stderr,"\r\n TREE_NODE %s[%u] LEAVE",pmib_tbl->name,j);					
					j++;
				}

                if(ptlv_data_value != NULL) {
                    free(ptlv_data_value);
                    ptlv_data_value = NULL;
                } 
				i+=tlv_len*tlv_num;				
			}
		}
	}

	if(ptlv_data_value != NULL) {
		free(ptlv_data_value);
		ptlv_data_value = NULL;
	} 

	return 1;
}

unsigned int mib_tlv_init(const mib_table_entry_T *mib_tbl, unsigned char *from_data, void *pfile, unsigned int tlv_content_len)
{
	unsigned int mib_offset = 0;

	if(mib_tbl == NULL || from_data == NULL || pfile == NULL || tlv_content_len == 0)
		return 0;

	if(mib_tlv_init_from(mib_tbl, from_data, pfile, tlv_content_len, &mib_offset) == 1)
		return 1;
	else
		return 0;

}

unsigned int mib_tlv_save(CONFIG_DATA_T type, void *mib_data, unsigned char *mib_tlvfile, unsigned int *tlv_content_len)
{
	mib_table_entry_T *pmib_tl = NULL;

//fprintf(stderr,"\r\n tlv_content_len = %p, __[%s-%u]",tlv_content_len,__FILE__,__LINE__);				


	if(mib_tlvfile == NULL)
	{
		return 0;
	}
//fprintf(stderr,"\r\n mib_tlvfile = 0x%x, __[%s-%u]",mib_tlvfile,__FILE__,__LINE__);		
	
	pmib_tl = mib_get_table(type);

	if(pmib_tl==0)
	{
		return 0;
	}
//fprintf(stderr,"\r\n mib_data = %p, __[%s-%u]",mib_data,__FILE__,__LINE__);					
//fprintf(stderr,"\r\n mib_tlvfile = %p, __[%s-%u]",mib_tlvfile,__FILE__,__LINE__);	
//fprintf(stderr,"\r\n pmib_tl->name=%s, __[%s-%u]",pmib_tl->name,__FILE__,__LINE__);	
//fprintf(stderr,"\r\n tlv_content_len = %p, __[%s-%u]",tlv_content_len,__FILE__,__LINE__);		
	mib_write_to_raw(pmib_tl, (void *)((int) mib_data), mib_tlvfile, tlv_content_len);

	return 1;

}

void mib_display_data_content(CONFIG_DATA_T type, unsigned char * pdata, unsigned int mib_data_len)
{
	int kk;
	fprintf(stderr,"\r\n type=%u, mibdata_len = %u",type, mib_data_len);
	fprintf(stderr,"\r\n pdata=");
	for(kk=0; kk< mib_data_len; kk++)
	{
		fprintf(stderr,"0x%02x_", *(pdata+kk));
		if( (kk+1)%10 == 0) fprintf(stderr,"\r\n");
	}
	fprintf(stderr,"0x%02x_",CHECKSUM(pdata, mib_data_len));
}

void mib_display_tlv_content(CONFIG_DATA_T type, unsigned char * ptlv, unsigned int mib_tlv_len)
{
	int kk;
	fprintf(stderr,"\r\n type=%u, tlv_content_len = %u",type, mib_tlv_len);			
	fprintf(stderr,"\r\n tlv_content=");
	for(kk=0; kk< mib_tlv_len; kk++)
	{
		fprintf(stderr,"0x%02x_", *(ptlv+kk));
		if( (kk+1)%10 == 0) fprintf(stderr,"\r\n");
	}
}

int mib_write_to_raw(const mib_table_entry_T *mib_tbl, void *data, unsigned char *pfile, unsigned int *idx)
{	
	unsigned short tlv_tag=0;	
	unsigned short tlv_len=0;	
	unsigned short tlv_num=0;		
	int i, j;
	//int k;

#if 0
fprintf(stderr,"\r\n > mib_tbl->name=%s, __[%s-%u]",mib_tbl->name,__FILE__,__LINE__);	
#endif

//fprintf(stderr,"\r\n idx = %p, __[%s-%u]",idx,__FILE__,__LINE__);		
//fprintf(stderr,"\r\n data = %p, __[%s-%u]",data,__FILE__,__LINE__);					
//fprintf(stderr,"\r\n pfile = %p, __[%s-%u]",pfile,__FILE__,__LINE__);	

	/* If pointing to other table */
	if(mib_tbl->type >= TABLE_LIST_T)	
	{

		const mib_table_entry_T *mib = mib_tbl->next_mib_table;		
		unsigned int offset=0;				

		/* traverse whole table */
		for(i=0 ; mib[i].id ; i++)		
		{			
			const mib_table_entry_T *pmib = &mib[i];			
#if 0
fprintf(stderr,"\r\n Turn mib[i].mib_name=%s __[%s-%u]",mib[i].name,__FILE__,__LINE__);
#endif
			if(mib[i].type < TABLE_LIST_T)			
			{				
				mib_write_to_raw(pmib, (void *)((int) data + offset), pfile, idx);				
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);				
//printf("\r\n offset + %u __[%s-%u]",pmib->unit_size,__FILE__,__LINE__);				
				//offset += pmib->size;
				offset += pmib->total_size;
			}
			else
			{				
				unsigned int ori_idx = 0;				
				unsigned short *ptlv_len = NULL;								
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);				
				if((pmib->total_size%pmib->unit_size) == 0)					
					tlv_num = pmib->total_size/pmib->unit_size;
				
				tlv_tag = (pmib->id);

				/* tag */
				memcpy(pfile+*idx, &tlv_tag, 2);
				*idx+=2;	
				
				//tlv_len = WORD_SWAP(tlv_len);				
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);				

				/* Set length to zero. It will be updated later */
				memcpy(pfile+*idx, &tlv_len, 2);				

				/* Remember pointer for saving Table length */
				/* We don't know exact length now. Length will be updated later. */
				ptlv_len = (unsigned short *)(pfile+*idx);				
				*idx+=2;								
				tlv_num = pmib->total_size/pmib->unit_size;				
				ori_idx = *idx;
//printf("\r\n -- ptlv_len[0x%x] *idx[%u] ori_idx[%u] tlv_num[%u]<< ",ptlv_len, *idx,ori_idx,tlv_num);

				for(j=0 ; j<tlv_num ; j++)				
				{
				
					mib_write_to_raw(pmib, (void *)((int) data + offset), pfile, idx);
					offset += pmib->unit_size;					
				}											
				/* Now, we know the Table length, fill it! */
				tlv_len = (*idx-ori_idx);				
				memcpy(ptlv_len, &tlv_len, 2);							
			}								
		} /* for(i) */		
	}
	else
	{	
		/* Most item run into here, including VoIP configuration, to save TLV */

		unsigned char *pChar = (unsigned char *) data;		
		//unsigned short mib_value;

#ifdef VOIP_SUPPORT_TLV_CFG
		if(mib_tbl->type == VOIP_T) 
		{			
			unsigned short *ptlv_len;
			unsigned short tlv_tag;
			unsigned short tlv_len;
			unsigned int tlv_offset_org;

			//printf("%s(%d)VoIP save TLV\n",__FUNCTION__,__LINE__);
			//printf("%s(%d)id=0x%04x, name=%s, u_sz=%d, t_sz=%d\n",__FUNCTION__,__LINE__,
			//			mib_tbl->id,mib_tbl->name,mib_tbl->unit_size,mib_tbl->total_size);

			/* Tag */
			tlv_tag = mib_tbl->id;
			//tlv_tag = WORD_SWAP(tlv_tag);
			memcpy((unsigned char*)((int)pfile + *idx),&tlv_tag,sizeof(tlv_tag));
			*idx+=2;

			/* addr of tlv_len */
			ptlv_len = (unsigned short *)(pfile+*idx);				

			/* we don't know exact len now */
			tlv_len = 0;
			//tlv_len = WORD_SWAP(tlv_len);
			memcpy(ptlv_len,&tlv_len,sizeof(tlv_len));
			*idx+=2;
			tlv_offset_org = *idx;

			/* call voip TLV process */
			voip_mib_cfg_write_to_tlv(&mibtbl_voip_root, (void *)data, pfile, idx);				

			/* fill correct tlv_len */
			tlv_len = *idx - tlv_offset_org;
			//tlv_len = WORD_SWAP(tlv_len);
			memcpy(ptlv_len,&tlv_len,sizeof(tlv_len));

			return 1;
		}
#endif
		/* Saving Tag */
		tlv_tag = (mib_tbl->id);

		memcpy(pfile+*idx, &tlv_tag, 2);		
		*idx+=2;				

		/* Saving Length */
        tlv_len = (mib_tbl->total_size);
		memcpy(pfile+*idx, &tlv_len, 2);

		*idx+=2;	

		/* Saving Value */
		memcpy(pfile+*idx, pChar, tlv_len);				

		*idx+=tlv_len;
		
#if 0 // TLV_DEBUG
fprintf(stderr,"\r\n >>mib_tbl->name=%s",mib_tbl->name);	
fprintf(stderr,"\r\n >>>tlv_tag=0x%x, tlv_len=0x%x, tlv_value = ",tlv_tag,tlv_len);
for(k=0; k< tlv_len; k++)
{	
fprintf(stderr,"0x%02x_", *(pChar+k));	
if( (k+1)%10 == 0) fprintf(stderr,"\r\n");
}
fprintf(stderr,"\r\n");
#endif

#if 0
fprintf(stderr,"\r\n pFile=");
for(k=0; k< *idx; k++)
{	
fprintf(stderr,"0x%02x_", *(pfile+k));	
if( (k+1)%10 == 0) fprintf(stderr,"\r\n");
}
#endif			
	}	
	return 1;
}

static int _mibtbl_check(const mib_table_entry_T *mib_tbl)
{
	int i, j;

	for (i=0; mib_tbl[i].id; i++)
	{
		if (mib_tbl[i].type >= TABLE_LIST_T)
		{
			if (_mibtbl_check(mib_tbl[i].next_mib_table) != 0)
			{
				fprintf(stderr, "MIB (%s, %d, %d) Error: check mib_tbl failed\n",
					mib_tbl[i].name, mib_tbl[i].total_size, mib_tbl[i].unit_size);
				return -1;
			}
		}

		// check duplicate mib id
		for (j=i+1; mib_tbl[j].id; j++)
		{
			if (mib_tbl[i].id == mib_tbl[j].id)
			{
				fprintf(stderr, "MIB Error: %s detect duplicate id in %s\n",
					mib_tbl[i].name, mib_tbl[j].name);
				return -1;
			}
		}
	}

	return 0;
}

int mibtbl_check(void)
{
	mib_table_entry_T *pmib_tl_hw;
	mib_table_entry_T *pmib_tl;

	pmib_tl_hw = mib_get_table(HW_SETTING);
	pmib_tl = mib_get_table(CURRENT_SETTING);

	if ((_mibtbl_check(pmib_tl_hw) != 0) ||
		(_mibtbl_check(pmib_tl) != 0))
		return -1;

	return 0;
}

#endif
