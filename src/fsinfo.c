
#include "amaterasu.h"

#define MajorFunction(Data) (Data->Iopb->MajorFunction)

/*
 *  FsInfoAlloc() - Allocate a 'FS_INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'FS_INFO' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
PFS_INFO FsInfoAlloc(_PoolType_ POOL_TYPE PoolType) {

    PFS_INFO FsInfo;

    FsInfo = ExAllocatePool2(PoolType, sizeof * FsInfo, 'fsif');
    if (!FsInfo) {
        return NULL;
    }

    RtlZeroMemory(FsInfo, sizeof * FsInfo);

    /*
     *  Store the pool type used for the allocation in 'FsInfo' to ensure
     *  correct memory handling, regardless of whether the caller routine is
     *  paged or nonpaged.
     */
    FsInfo->PoolType = PoolType;

    return FsInfo;
}

/*
 *  FsInfoGet() - Allocates and initializes a 'FS_INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *  @Data: Pointer to a FLT_CALLBACK_DATA object.
 *
 *  'FsInfoGet()' is a wrapper for 'FsInfoAlloc()' and 'FsInfoInit()',
 *  providing a simple interface to obtain an allocated and initialized
 *  'FS_INFO' structure.
 *
 *  Returns:
 *    - Pointer to the allocated 'FS_INFO' structure on success.
 *    - 'NULL' if memory allocation or initialization fails.
 */
PFS_INFO FsInfoGet(_PoolType_ POOL_TYPE PoolType, _In_ PVOID Data) {

    PFS_INFO FsInfo;
    NTSTATUS Status;

    FsInfo = FsInfoAlloc(PoolType);
    if (!FsInfo) {
        return NULL;
    }

    Status = FsInfoInit(FsInfo, Data);
    if (!NT_SUCCESS(Status)) {
        FsInfoFree(&FsInfo);
    }

    return FsInfo;
}

/*
 *  FsInfoInit() - Initializes a 'FS_INFO' structure allocate by 'FsInfoAlloc()'
 *                 based on 'FLT_CALLBACK_DATA'.
 *
 *  @FsInfo: Pointer to the 'FS_INFO' structure to be initialized.
 *  @Data: Pointer to the 'FLT_CALLBACK_DATA' structure containing file-related
 *         and process related data.
 *
 *  Return:
 *    - Status code indicating the result of the initialization.
 *    - Upon success, returns 'STATUS_SUCCESS'.
 *    - The appropriate error code in case of failure.
 */
NTSTATUS FsInfoInit(_Out_ PFS_INFO FsInfo, _In_ PFLT_CALLBACK_DATA Data) {


    FsInfo->MjFunc = MajorFunction(Data);
    FsInfo->ProcInfo = ProcInfoGet(FsInfo->PoolType, Data);
    if (!FsInfo->ProcInfo) {
        return STATUS_UNSUCCESSFUL;
    }
    
    FsInfo->FileInfo = FileInfoGet(FsInfo->PoolType ,Data);
    if (!FsInfo->FileInfo) {
        ProcInfoFree(&FsInfo->ProcInfo);
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

/*
 *  FsInfoDeInit() - Deinitializes and frees the internal resources
 *                   associated with a 'FS_INFO' structure.
 *
 *  @FsInfo: Pointer to the 'FS_INFO' structure to be deinitialized.
 */
void FsInfoDeInit(_Inout_ PFS_INFO FsInfo) {

    if (FsInfo) {
        FileInfoFree(&FsInfo->FileInfo);
        ProcInfoFree(&FsInfo->ProcInfo);
    }
}

/*
 *  FsInfoFree() - Deallocates the memory associated with a 'FS_INFO'
 *               struct dynamically allocated.
 *
 *  @FsInfo: Pointer to a reference of a 'FS_INFO' structure.
 */
void FsInfoFree(_Inout_ PFS_INFO* FsInfo) {

    if (FsInfo && *FsInfo) {
        FsInfoDeInit(*FsInfo);
        ExFreePoolWithTag(*FsInfo, 'fsif');

        /*
         *  In order to avoid a dangling pointer, after deallocating the
         *  'FS_INFO' structure, we set the reference to the 'FS_INFO'
         *  struct to NULL.
         */
        *FsInfo = NULL;
    }
}

/*
 *  FsInfoCopy() -
 *
 *  @Dest:
 *  @Src:
 *
 *  Return:
 *    -
 *    -
 */
void FsInfoCopy(_Out_ PFS_INFO_STATIC Dest, _In_ PFS_INFO Src) {

    if (Dest && Src) {
        Dest->MjFunc = Src->MjFunc;
        FileInfoCopy(&Dest->FileInfo, Src->FileInfo);
        ProcInfoCopy(&Dest->ProcInfo, Src->ProcInfo);
    }
}
