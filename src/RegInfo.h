#ifndef REG_H
#define REG_H

#include "common.h"
#include "utils.h"


/*
close, query , open , set, delete
*/


struct RegInfoData {
	REG_NOTIFY_CLASS RegNotifyClass;
	PVOID RegStruct;
};

typedef struct RegInfoData REG_INFO_DATA, * PREG_INFO_DATA;

struct RegInfo {

	REG_NOTIFY_CLASS RegNotifyClass;
	
	PWSTR CompleteName;
	ULONG CompleteNameSize;
	
	PWSTR Data;
	ULONG DataSize;
	ULONG DataType;

	POOL_TYPE PoolType;
};

struct RegInfoStatic {

	REG_NOTIFY_CLASS RegNotifyClass;

	WCHAR CompleteName[MAX_PATH];
	ULONG CompleteNameSize;

	WCHAR Data[MAX_DATA];
	ULONG DataSize;
	ULONG DataType;
};

typedef struct RegInfo REG_INFO, * PREG_INFO;

typedef struct RegInfoStatic REG_INFO_STATIC, * PREG_INFO_STATIC;

PREG_INFO RegInfoAlloc(_PoolType_ POOL_TYPE PoolType);

PREG_INFO RegInfoGet(
	_PoolType_ POOL_TYPE PoolType,
	PREG_INFO_DATA RegInfoData
);

extern NTSTATUS RegInfoInit(PREG_INFO RegInfo, REG_NOTIFY_CLASS RegOperation, PVOID RegStruct);

void RegInfoCopy(
	PREG_INFO_STATIC StaticRegInfo,
	PREG_INFO
);

void RegInfoFree(
	PREG_INFO* RegInfo
);

#endif
 

