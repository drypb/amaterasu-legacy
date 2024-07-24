#ifndef INFO_LIST_H
#define INFO_LIST_H

#include "common.h"
#include "info.h"

#define MAX_RECORDS 1024

struct InfoList {

    ULONG          MaxRecords;
    volatile ULONG RecordsAllocated;
    KSPIN_LOCK     Lock;

    LIST_ENTRY Head;

    POOL_TYPE PoolType;

}; 

typedef struct InfoList INFO_LIST, * PINFO_LIST;

/*
 *  InfoListAlloc() - Allocate a 'INFO_LIST' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'INFO_LIST' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
extern PINFO_LIST
InfoListAlloc(
    _PoolType_ POOL_TYPE PoolType
);

/*
 *  InfoListGet() - Allocates and initializes a 'INFO_LIST' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *  @MaxRecords: 
 *
 *  'InfoListGet()' is a wrapper for 'InfoListAlloc()' and 
 *  'InfoListInit()', providing a simple interface to obtain an 
 *  allocated and initialized 'INFO_LIST' structure.
 *
 *  Returns:
 *    - Pointer to the allocated 'INFO_LIST' structure on success.
 *    - 'NULL' if memory allocation or initialization fails.
 */
extern PINFO_LIST
InfoListGet(
    _PoolType_ POOL_TYPE PoolType,
    _In_ ULONG MaxRecords
);

/*
 *  InfoListInit() - Initializes a 'INFO_LIST' structure.
 *
 *  @InfoList: pointer to a 'INFO_LIST' structure to be initialized.
 *  @MaxRecords: Maximum number of records that the list can hold.
 */
extern void
InfoListInit(
    _Inout_ PINFO_LIST InfoList,
    _In_ ULONG MaxRecords    
);

/*
 *  InfoListFree() -
 *
 *  @InfoList:
 */
extern void
InfoListFree(
    _Inout_ PINFO_LIST* InfoList
);

/*
 *  InfoListAppend() -
 *
 *  @InfoList:
 *  @Data:
 *
 *  Return:
 *    -
 *    -
 */
extern NTSTATUS
InfoListAppend(
    _Inout_ PINFO_LIST InfoList,
    _In_ PVOID Data,
    _In_ INFO_TYPE InfoType
);

/*
 *  InfoListRemoveTail() -
 *
 *  @InfoList:
 *
 *  Return:
 *    -
 *    -
 */
extern PINFO
InfoListRemoveTail(
    _Inout_ PINFO_LIST InfoListRemoveTail
);

#endif  /* INFO_LIST_H */
