/*
 * hash_table.c
 *
 *  Created on: 2014/5/30
 *      Author: MTK04880
 */
#include <linux/version.h>
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#include <linux/slab.h>
#endif
#include "hash_utils.h"
#include <linux/list.h>
#include "list_utils.h"

/*******************************************************************************/
/* local definitions                                                    */
/*******************************************************************************/
#define CONFIG_HASH_SIZE		64
#define CONFIG_SHRINK_HEAD_IDENTIFIER "CONF_SHRINK_HEAD"
#define CONFIG_SHRINK_TAIL_IDENTIFIER "CONF_SHRINK_TAIL"
//#define HASH_STAT_DBG
typedef enum{
	NOT_INITED = 0,
	NOT_COMPLETED,
	INIT_COMPLETED
}configNodeStat_e;


/*******************************************************************************/
/* SNCFGD local prototypes                                                     */
/*******************************************************************************/
struct config {
	struct list_head next;		/* hash list, for lookup */
	int idx;
	int stat;
	char *key;
	char *value;
};

#ifdef HASH_STAT_DBG
typedef struct hash_statistic_s{
	unsigned int confCnt;
	unsigned int oriUsedMem;
	unsigned int hashUsedMem;
	unsigned int idxRecord[CONFIG_HASH_SIZE];
}hash_statistic_t;

hash_statistic_t gHashStat;
#endif

/*******************************************************************************/
/* local variables                                                      */
/*******************************************************************************/
//static hashTb_funcSet_t gfuncSet;
/*sync flag indicates that the memory which Hash have been constructed was sync into Flash*/
static int gSyncFlag  = 0;
static struct list_head gConfigHashTable[CONFIG_HASH_SIZE];

/*******************************************************************************/
/* global variables                                                            */
/*******************************************************************************/

/*******************************************************************************/
/* Function prototypes                                                     */
/*******************************************************************************/
#if 0
static char *malloc_line_from_file(FILE *file, int *size);
#endif

static int hash_config(char *key);
static struct config *find_config(char *key);
static struct list_head *find_uncompleted_config(char *key);
#if 1
static int hashTb_header_identify(char *data, int *retLen);
#endif
static int hashTb_initNode_set(int key,int idx, char *value);
static int hashTb_conf_runtime_set(char *key, char *value);
#ifdef HASH_STAT_DBG
static void hashStatShow(void);
#endif

static char *hashTb_conf_get(char *key);
static int hashTb_conf_set(char *key, char *value);
static int hashTb_conf_init(char **data,int len);
static int hashTb_conf_clear(void);
static int hashTb_conf_getall(char** data,int maxLen,int target);

/*******************************************************************************/
/* Function Declartions                                                     */
/*******************************************************************************/

#define SYNC_FLAG_NOT_YET_CHECK if(!gSyncFlag)\
									return HASHTB_RET_FAIL;
#define SYNC_FLAG_DONE_CHECK if(gSyncFlag)\
								return HASHTB_RET_FAIL;
#define SYNC_FLAG_SET(b) gSyncFlag = b;

#ifdef HT_DEBUG
#define DEBUG_PRINT(str, args...) \
	do { \
		printk("[%s-%d] ", __FUNCTION__,__LINE__); \
		printk(str, ##args); \
		printk("\n"); \
	} while(0)
#else
#define DEBUG_PRINT(str, args...)
#endif

#define LIST_CHECK_KEY(x) \
	if(!x) continue

#define ALIGN_32B(x) ((x) + 32 - ((x)%32))
#define MALLOC(x) kmalloc(GFP_KERNEL,ALIGN_32B(x))

int hash_funcSet_reg(hashTb_funcSet_t *funcP){

	int i = 0;
	if(!funcP)
		return HASHTB_RET_FAIL;

	//funcP = &gfuncSet;
	memset(funcP,0,sizeof(hashTb_funcSet_t));
	funcP->conf_init = hashTb_conf_init;
	funcP->conf_get = hashTb_conf_get;
	funcP->conf_set = hashTb_conf_set;
	funcP->conf_getall = hashTb_conf_getall;
	funcP->conf_clear = hashTb_conf_clear;

	for (i = 0; i < CONFIG_HASH_SIZE; i++) {
		INIT_LIST(&gConfigHashTable[i]);
	}

	return HASHTB_RET_SUCCESS;
}

int hash_config(char *key){
	int i, v = 0;
	char *c;

	for (i = 31, c = key; i != 0 && *c != '\0'; i--, c++) {
		v += *c;
	}
	//printf("[%s] v = %d \n",key,v % CONFIG_HASH_SIZE);
	return (v % CONFIG_HASH_SIZE);
}

struct config *find_config(char *key){
	struct list_head *head = &gConfigHashTable[hash_config(key)];
	struct list_head *fList;

	LIST_CHECK(head,NULL);
	LIST_FOR_EACH(fList, head) {
		LIST_CHECK(fList,NULL);
		LIST_CHECK_KEY(((struct config *)fList)->key);
		if (!strcmp(key, ((struct config *)fList)->key)) {
			return ((struct config *)fList);
		}
	}
	return NULL;
}

/****************************************
 * Function name: find_uncompleted_config
 * Description:
 * 	find and reply the config node pointer which key attr was not initialized.
 * Parameter:
 * 	key: key string of dedicated configuration
 *
 * Return:
 * 	struct list_head*
 *
 * **************************************
 */

static struct list_head *find_uncompleted_config(char *key){
	struct list_head *head = &gConfigHashTable[hash_config(key)];
	struct list_head *fList;

	LIST_CHECK(head,NULL);
	LIST_FOR_EACH(fList, head) {
		LIST_CHECK(fList,NULL);
		if (((struct config *)fList)->stat == NOT_COMPLETED) {
			return fList;
		}
	}
	return NULL;
}

static int find_hash_entry_idx(char *key){
	int idx = 0;
	struct list_head *head = &gConfigHashTable[hash_config(key)];
	struct list_head *fList;

	LIST_CHECK(head,0);
	LIST_FOR_EACH(fList, head) {
		LIST_CHECK(fList,idx);
		idx++;
	}
	return idx;
}

static char *hashTb_conf_get(char *key){
	struct config *pConf;

	SYNC_FLAG_NOT_YET_CHECK

	pConf = find_config(key);
	if (!pConf) {
		DEBUG_PRINT("get Empty (key:%s idx:%d)\n",key,hash_config(key));
		return NULL;
	}
	DEBUG_PRINT("key:%s (%d_%d)value:%s\n",key,hash_config(key),pConf->idx,pConf->value);
	return (pConf->value);
}

/****************************************
 * Function name: hashTb_initNode_set
 * Description:
 * 	Set the init node in hash TB w/o key string
 * Parameter:
 * 	key: hash key (get from hash function)
 * 	idx: the node idx in each hash entry
 * 	value: the value of dedicated configuration
 * Return:
 * 	1:HASHTB_RET_SUCCESS
 * 	0:HASHTB_RET_FAIL
 *
 * **************************************
 */
static int hashTb_initNode_set(int key,int idx, char *value){
	size_t vLen;
	char *linebuf;
	struct config *pConf;

	vLen = strlen(value);

	pConf = MALLOC(sizeof(struct config));
	if (!pConf) {
		DEBUG_PRINT("Can't malloc for runtime configuration !!");
		return HASHTB_RET_FAIL;
	}

	linebuf = MALLOC(vLen+1);
	if (!linebuf) {
		kfree(pConf);
		DEBUG_PRINT("Can't malloc for line of runtime configuration !!");
		return HASHTB_RET_FAIL;
	}
	memset(linebuf,'\0',ALIGN_32B(vLen+1));

	INIT_LIST(&pConf->next);
	pConf->key = NULL;
	pConf->value = linebuf;
	pConf->idx = idx;
	pConf->stat = NOT_COMPLETED;
	strcpy(pConf->value, value);
	list_add_tail(&pConf->next, &gConfigHashTable[key]);

	return HASHTB_RET_SUCCESS;
}

//char tmpMemPool[256*1024];
//int bufIdx = 0;
static int hashTb_conf_runtime_set(char *key, char *value){
	struct list_head *fList;
	size_t kLen, vLen;
	struct config *pConf;

	kLen = strlen(key);
	vLen = strlen(value);

	if((fList = find_uncompleted_config(key))!=NULL){
		((struct config *)fList)->key = MALLOC(kLen+1);
		if(!((struct config *)fList)->key){
			return HASHTB_RET_FAIL;
		}
		//((struct config *)fList)->key = &tmpMemPool[bufIdx];
		//bufIdx = bufIdx + kLen+1;
		memset(((struct config *)fList)->key,'\0',ALIGN_32B(kLen+1));
		strcpy(((struct config *)fList)->key, key);
#if 1
		DEBUG_PRINT("set completed key:%s ->%d_%d (value:%s)",((struct config *)fList)->key,hash_config(key),\
				((struct config *)fList)->idx,((struct config *)fList)->value);
#endif
		((struct config *)fList)->stat = INIT_COMPLETED;
	}
	else{
		/*
		 * If SYNC_FLAG_DONE, it presents that the configuration would like to be set after
		 * Hash table initialization. It also means that the configuration
		 * doesn't exist in Hash table by default. Per our Config Shrink mechaniasm,this kind of
		 * configuration have to be written to flash in plan text. Return fail and do
		 * the alternative path to memory.
		 */
#if 1
		SYNC_FLAG_DONE_CHECK
#endif
		pConf = MALLOC(sizeof(struct config));
		if (!pConf) {
			DEBUG_PRINT("Can't malloc for runtime configuration !!");
			return HASHTB_RET_FAIL;
		}

		pConf->key = MALLOC(kLen+1);
		if (!pConf->key) {
			kfree(pConf);
			DEBUG_PRINT("Can't malloc for line of runtime configuration !!");
			return HASHTB_RET_FAIL;
		}
		memset(pConf->key,'\0',ALIGN_32B(kLen+1));
		strcpy(pConf->key, key);

		pConf->value = MALLOC(vLen+1);
		if (!pConf->value) {
			kfree(pConf);
			DEBUG_PRINT("Can't malloc for line of runtime configuration !!");
			return HASHTB_RET_FAIL;
		}
		memset(pConf->value,'\0',ALIGN_32B(vLen+1));
		strcpy(pConf->value, value);

		INIT_LIST(&pConf->next);
		pConf->idx = find_hash_entry_idx(key);
		pConf->stat = INIT_COMPLETED;
		DEBUG_PRINT("%s:k:%s v:%s (idx:%d ridx:%d)\n",__func__,pConf->key,pConf->value,hash_config(pConf->key),pConf->idx);

		list_add_tail(&pConf->next, &gConfigHashTable[hash_config(pConf->key)]);
	}
	return HASHTB_RET_SUCCESS;
}


static int hashTb_conf_set(char *key, char *value){
	//char *reline;
	//size_t vLen;

	struct config *pConf = find_config(key);

	if (!pConf) {
		/*
		 * runtime configuration,
		 * does not belong to non-volitile configurations
		 */
		return (hashTb_conf_runtime_set(key, value));
	}
	//kLen = strlen(key);
	//vLen = strlen(value);

	if (strcmp(pConf->value, value) == 0) {
		return HASHTB_RET_SUCCESS;
	}
	//memset(pConf->value,'\0',ALIGN_32B(vLen+1));
	//strcpy(pConf->value, value);
	kfree(pConf->value);
	pConf->value = kstrdup(value, GFP_KERNEL);
	//printk("%s end\n",__func__);
	return HASHTB_RET_SUCCESS;
}

#if 1
static int hashTb_header_identify(char *data, int *retLen){
	char *key,*value,*p,*q;

	if((!data) || (!retLen))
		return HASHTB_RET_FAIL;

	key = value = NULL;
	p = data;
	*retLen = 0;
	if (NULL == (q = strchr(p, '='))) {
		DEBUG_PRINT("parsed failed - cannot find '='\n");
		return HASHTB_RET_FAIL;
	}
	*q = '\0'; //strip '='
	key = p;//kstrdup(p, GFP_KERNEL);
	p = q + 1; //value
	if (NULL == (q = strchr(p, '\0'))) {
		DEBUG_PRINT("parsed failed - cannot find '\\0'\n");
		return HASHTB_RET_FAIL;
	}
	value = p;//kstrdup(p, GFP_KERNEL);
	if (!strcmp(key, CONFIG_SHRINK_HEAD_IDENTIFIER)) {
		if (!strcmp(value, "1")){
			DEBUG_PRINT("IDENTIFIED !! \n");
			*retLen = strlen(CONFIG_SHRINK_HEAD_IDENTIFIER) + 3;//CONF_SHRINK=1\0
			return HASHTB_RET_SUCCESS;
		}
	}
	return HASHTB_RET_FAIL;
}
#endif

static int hashTb_conf_init(char **data,int len){

	int l,header_exist,hashKey,idx;
	char *key,*value,*p,*q;

	l = header_exist = hashKey = idx = 0;
	key = value = p = q = NULL;

	if(!(*data))
		return HASHTB_RET_FAIL;

	p = *data;
#if 1
	/*
	 * identify if shrinked config existed in flash or not.
	 * If it has not, it has to establish HASH strcutures in memory and wait to commit
	 * */
	if(hashTb_header_identify(p,&l)){
		header_exist = 1;
		p += l;
	}
	DEBUG_PRINT("header %d! - len:%d nowp:%d ori len:%d\n",header_exist,l,p - (*data) + 1,len);
#endif
	while((p - (*data) + 1) < len){
		if (NULL == (q = strchr(p, '='))) {
			DEBUG_PRINT("parsed failed - cannot find '='\n");
			break;
		}
		*q = '\0'; //strip '='
		key = p;//kstrdup(p, GFP_KERNEL);
		p = q + 1; //value
		if (NULL == (q = strchr(p, '\0'))) {
			DEBUG_PRINT("parsed failed - cannot find '\\0'\n");
			break;
		}
		value = p;//kstrdup(p, GFP_KERNEL);
		if(header_exist){
			if (!strcmp(key, CONFIG_SHRINK_TAIL_IDENTIFIER)) {
				if (!strcmp(value, "1")){
					DEBUG_PRINT("TAIL IDENTIFIED !! \n");
					p = q + 1; //next entry
					*data = p;
					SYNC_FLAG_SET(1)
					return HASHTB_RET_SUCCESS;
				}
			}
			sscanf(key,"%x_%x",&hashKey,&idx);

			hashTb_initNode_set(hashKey,idx,value);
		}
		else{
			hashTb_conf_set(key,value);
		}
		p = q + 1; //next entry
		if (*p == '\0') {
			//end of env
			//DEBUG_PRINT("%c,%c,%c",*p,*(p+1),*(p+2));
			break;
		}
	}
	//if(p == *data){
		/*there are no any config in flash*/
	//	return HASHTB_RET_FAIL;
	//}
#ifdef HASH_STAT_DBG
	memset(&gHashStat,0,sizeof(hash_statistic_t));
#endif
	return HASHTB_RET_SUCCESS;
}
static int hashTb_conf_clear(void){
	int i,idx;
	struct list_head *head,*fList;
	char *key,*value,*p;
	static int oneShot = 0;

	/*
	 * dont clear the hash after the 1st time called by libnvram when it can not get WebInit.
	 * cuz the hash table have been established and waited for loading default setting.
	 *
	 * */
	if(!oneShot){
		if(gSyncFlag)
			oneShot = 1;
		return HASHTB_RET_FAIL;
	}
	i = idx = 0;
	head = fList = NULL;
	key = value = p = NULL;
	DEBUG_PRINT("hashTb_conf_clear \n");
	for(i = 0;i < CONFIG_HASH_SIZE; i++){
		head = &gConfigHashTable[i];
#if 0
		LIST_FOR_EACH(fList, head) {
			LIST_CHECK(fList);
			if(!fList)
				continue;

			list_del(fList);
			key = ((struct config *)fList)->key;
			value = ((struct config *)fList)->value;
			if(key)
				kfree(key);
			if(value)
				kfree(value);
			if(fList)
				kfree(fList);
		}
#else
		while(((head)->next) && ((head)->next != head)){
			fList = (head)->next;
			list_del(fList);
			key = ((struct config *)fList)->key;
			value = ((struct config *)fList)->value;
			if(key)
				kfree(key);
			if(value)
				kfree(value);
			if(fList)
				kfree(fList);
		}
#endif
	}
	SYNC_FLAG_SET(0)
	DEBUG_PRINT("Done \n");
	return HASHTB_RET_SUCCESS;
}
static int hashTb_conf_getall(char **data,int maxLen,int target){


	int i,l,idx,confLen;
	struct list_head *head,*fList;
	char *key,*value,*p;

	i = l = idx = confLen= 0;
	head = fList = NULL;
	key = value = p = NULL;

	if(!(*data))
		return HASHTB_RET_FAIL;

#ifdef HASH_STAT_DBG
	memset(&gHashStat,0,sizeof(gHashStat));
#endif
	//memset(*data,0,len);
	p = *data;

	DEBUG_PRINT(":Get All!!");
	if(target == HASH_2_FLASH){
		l = strlen(CONFIG_SHRINK_HEAD_IDENTIFIER) + 3;
		snprintf(p,l,"%s=1",CONFIG_SHRINK_HEAD_IDENTIFIER);
		DEBUG_PRINT("(C) %s - Len:%d\n",p,maxLen);
		p += l;
		confLen += l;
	}
	else{
		SYNC_FLAG_NOT_YET_CHECK
	}

	for(i = 0;i < CONFIG_HASH_SIZE; i++){
		head = &gConfigHashTable[i];
		LIST_FOR_EACH(fList, head) {
			if(!fList)
				break;
			key = ((struct config *)fList)->key;
			idx = ((struct config *)fList)->idx;
			value = ((struct config *)fList)->value;

			if(!key){
				continue;
			}
			if (p - (*data) + 2 >= maxLen) {
				DEBUG_PRINT("BLK SIZE is not enough!");
				*data = p;
				return HASHTB_RET_FAIL;
			}
#ifdef HASH_STAT_DBG
			if(target == HASH_2_FLASH){
				gHashStat.confCnt++;
				gHashStat.oriUsedMem += (strlen(key)+strlen(value)+2);
			}
#endif
			switch(target){
			case HASH_2_MEM:
				l = strlen(key)+strlen(value)+2;
				snprintf(p, l, "%s=%s", key, value);
				//DEBUG_PRINT("(M):%s (hash key:%d idx:%d(key:%s value:%s) now:%d)\n",p,i,idx,key,value,p - (*data) + 2);
				break;
			case HASH_2_FLASH:
				l = (7+strlen(value));
				snprintf(p, l, "%02x_%02x=%s",i, idx, value);
				//DEBUG_PRINT("(C):%s (hash key:%d idx:%d(key:%s value:%s) now:%d (len:%d))\n",p,i,idx,key,value,p - (*data) + 2,len);
				break;
			default:
				return HASHTB_RET_FAIL;
			}
			confLen += l;
			p += l;
		}
#ifdef HASH_STAT_DBG
		if(target == HASH_2_FLASH)
			gHashStat.idxRecord[i] = idx;
#endif
	}
	if(target == HASH_2_FLASH){
		l = strlen(CONFIG_SHRINK_TAIL_IDENTIFIER) + 3;
		snprintf(p,l,"%s=1",CONFIG_SHRINK_TAIL_IDENTIFIER);
		p += l;
		confLen += l;
		DEBUG_PRINT("(C) %s - actual Len:%x maxLen:%x\n",p,confLen,maxLen);
		*data = p;
		SYNC_FLAG_SET(1)
		return HASHTB_RET_SUCCESS;
	}

#ifdef HASH_STAT_DBG
	if(target == HASH_2_FLASH){
		gHashStat.hashUsedMem = (int)(p - (*data));
		hashStatShow();
	}
#endif
	//*p = '\0'; //ending null
	*data = p;
	return HASHTB_RET_SUCCESS;
}

#ifdef HASH_STAT_DBG
static void hashStatShow(void){
	int i;
	unsigned int maxIdx,minIdx,maxCnt,minCnt;
	maxIdx = maxCnt = i = minCnt = 0;
	minIdx = 0xffffffff;

	for(i = 0;i < CONFIG_HASH_SIZE;i++){
		if(maxIdx < gHashStat.idxRecord[i])
			maxIdx = gHashStat.idxRecord[i];
		if(minIdx > gHashStat.idxRecord[i])
			minIdx = gHashStat.idxRecord[i];
	}
	for(i = 0;i < CONFIG_HASH_SIZE;i++){
		if(maxIdx ==  gHashStat.idxRecord[i])
			maxCnt++;
		if(minIdx == gHashStat.idxRecord[i])
			minCnt++;
	}

	DEBUG_PRINT("<HASH Status>");
	DEBUG_PRINT("conf Cnt:%d",gHashStat.confCnt);
	DEBUG_PRINT("used Mem size:%d bytes (ori:%d B)",gHashStat.hashUsedMem,gHashStat.oriUsedMem);
	DEBUG_PRINT("maximum %d elements in %d keys",maxIdx,maxCnt);
	DEBUG_PRINT("minimum %d elements in %d keys",minIdx,minCnt);
}
#endif

