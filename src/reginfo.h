#ifndef REGINFO_H
#define REGINFO_H

#include "common.h"
#include "utils.h"

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

typedef struct RegInfo REG_INFO, * PREG_INFO;

struct RegInfoStatic {

	REG_NOTIFY_CLASS RegNotifyClass;

	WCHAR CompleteName[MAX_PATH];
	ULONG CompleteNameSize;

	WCHAR Data[MAX_DATA];
	ULONG DataSize;
	ULONG DataType;
};


typedef struct RegInfoStatic REG_INFO_STATIC, * PREG_INFO_STATIC;

/*
 *  RegInfoAlloc() - Allocate a 'REG_INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'REG_INFO' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
extern PREG_INFO
RegInfoAlloc(
    _PoolType_ POOL_TYPE PoolType
);

extern PREG_INFO
RegInfoGet(
	_PoolType_ POOL_TYPE PoolType,
	_In_ PREG_INFO_DATA RegInfoData
);

extern NTSTATUS
RegInfoInit(
    _Inout_ PREG_INFO RegInfo,
    _In_ PREG_INFO_DATA RegInfoData
);

extern void
RegInfoCopy(
	_Inout_ PREG_INFO_STATIC Dest,
	_In_ PREG_INFO Src
);

extern void
RegInfoDeInit(
    _Inout_ PREG_INFO RegInfo
);

extern void 
RegInfoFree(
	_Inout_ PREG_INFO* RegInfo
);
#endif  /* REGINFO_H */
