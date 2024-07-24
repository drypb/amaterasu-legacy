
#include "infolist.h"

/*
 *  InfoListAlloc() - Allocate a 'INFO_LIST' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'INFO_LIST' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
PINFO_LIST InfoListAlloc(_PoolType_ POOL_TYPE PoolType) {

    PINFO_LIST InfoList;

    InfoList = ExAllocatePool2(PoolType, sizeof * InfoList, 'list');
    if (!InfoList) {
        return NULL;
    }

    RtlZeroMemory(InfoList, sizeof * InfoList);

    /*
     *  Store the pool type used for the allocation in 'InfoList' to ensure
     *  correct memory handling, regardless of whether the caller routine is
     *  paged or nonpaged.
     */
    InfoList->PoolType = PoolType;

    return InfoList;
}

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
PINFO_LIST InfoListGet(_PoolType_ POOL_TYPE PoolType, _In_ ULONG MaxRecords) {

    PINFO_LIST InfoList;

    InfoList = InfoListAlloc(PoolType);
    if (!InfoList) {
        return NULL;
    }

    InfoListInit(InfoList, MaxRecords);

    return InfoList;
}

/*
 *  InfoListInit() - Initializes a 'INFO_LIST' structure.
 *
 *  @InfoList: Pointer to a 'INFO_LIST' structure to be initialized.
 *  @MaxRecords: Maximum number of records that the list can hold.
 */
void InfoListInit(_Inout_ PINFO_LIST InfoList, _In_ ULONG MaxRecords) {

    InfoList->MaxRecords = MaxRecords;

    KeInitializeSpinLock(&InfoList->Lock);
    InitializeListHead(&InfoList->Head);
}

/*
 *  InfoListFreeNodes() - Frees all the nodes in a 'INFO_LIST'.
 *
 *  @InfoList: Pointer to a 'INFO_LIST' structure.
 */
static void InfoListFreeNodes(_Inout_ PINFO_LIST InfoList) {

    PINFO Info;

    while (InfoList->RecordsAllocated) {
        Info = InfoListRemoveTail(InfoList);
        InfoListFree(&InfoList);
    }
}

/*
 *  InfoListFree() -
 *
 *  @InfoList:
 */
void InfoListFree(_Inout_ PINFO_LIST* InfoList) {

    if (InfoList) {
        InfoListFreeNodes(*InfoList);
        ExFreePoolWithTag(*InfoList, 'list');

        /*
         *  In order to avoid a dangling pointer, after deallocating the
         *  'INFO_LIST' structure, we set the reference to the 'INFO_LIST'
         *  struct to NULL.
         */
        *InfoList = NULL;
    }
}

/*
 *  AppendInfo() -
 *
 *  @InfoList:
 *  @Info:
 */
static void AppendInfo(_Inout_ PINFO_LIST InfoList, _In_ PINFO Info) {

    KIRQL OldIrql;
    PINFO RemovedInfo;

    KeAcquireSpinLock(&InfoList->Lock, &OldIrql);

    /*
     *  If there's already 'MaxRecords - 1' members allocated in the list,
     *  we remove the tail element (the oldest) and insert the new 'Info'
     *  at the head.
     */
    if (InfoList->RecordsAllocated + 1 >= InfoList->MaxRecords) {
        RemovedInfo = InfoListRemoveTail(InfoList);
        InfoFree(&RemovedInfo);
    }
    else {
        InfoList->RecordsAllocated++;
    }

    InsertHeadList(&InfoList->Head, &Info->ListEntry);
    KeReleaseSpinLock(&InfoList->Lock, OldIrql);
}

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
NTSTATUS InfoListAppend(_Inout_ PINFO_LIST InfoList, _In_ PVOID Data, _In_ INFO_TYPE InfoType) {

    PINFO Info;

    Info = InfoGet(InfoList->PoolType, Data, InfoType);
    if (!Info) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    AppendInfo(InfoList, Info);

    return STATUS_SUCCESS;
}

/*
 *  InfoListRemoveTail() -
 *
 *  @InfoList:
 *
 *  Return:
 *    -
 *    -
 */
PINFO InfoListRemoveTail(_Inout_ PINFO_LIST InfoList) {

    PINFO Info;
    KIRQL OldIrql;
    PLIST_ENTRY Entry;

    Info = NULL;
    KeAcquireSpinLock(&InfoList->Lock, &OldIrql);

    if (InfoList->RecordsAllocated) {
        InfoList->RecordsAllocated--;

        Entry = RemoveTailList(&InfoList->Head);
        Info = CONTAINING_RECORD(Entry, INFO, ListEntry);
    }

    KeReleaseSpinLock(&InfoList->Lock, OldIrql);

    return Info;
}
