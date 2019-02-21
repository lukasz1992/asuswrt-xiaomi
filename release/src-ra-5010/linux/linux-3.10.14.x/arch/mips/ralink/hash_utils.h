/*
 * hash_table.h
 *
 *  Created on: 2014/5/30
 *      Author: MTK04880
 */

#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

#define HASHTB_RET_SUCCESS (1)
#define HASHTB_RET_FAIL		(0)
typedef enum{
	HASH_2_MEM = 0,
	HASH_2_FLASH,
	HASH_2_TARGET_MAX
}hash2target_e;

typedef int (*funcType1)(char *,char *);
typedef char* (*funcType2)(char *);
typedef int (*funcType3)(char **,int,int);
typedef int (*funcType4)(char **,int);
typedef int (*funcType5)(void);

typedef struct hashTb_funcSet_s{
	funcType4 conf_init;
	funcType2 conf_get;
	funcType1 conf_set;
	funcType3 conf_getall;
	funcType5 conf_clear;
}hashTb_funcSet_t;

extern int hash_funcSet_reg(hashTb_funcSet_t *funcP);

#endif /* HASH_TABLE_H_ */
