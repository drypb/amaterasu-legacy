

#include "procinfo.h"
#include "utils.h"

/*
 *  ProcInfoAlloc() - Allocate a 'PROC_INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'PROC_INFO' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
PPROC_INFO ProcInfoAlloc(_PoolType_ POOL_TYPE PoolType) {

    PPROC_INFO ProcInfo;

    ProcInfo = ExAllocatePool2(PoolType, sizeof * ProcInfo, 'prif');
    if (!ProcInfo) {
        return NULL;
    }

    RtlZeroMemory(ProcInfo, sizeof * ProcInfo);

    /*
     *  Store the pool type used for the allocation in 'ProcInfo' to ensure
     *  correct memory handling, regardless of whether the caller routine is
     *  paged or nonpaged.
     */
    ProcInfo->PoolType = PoolType;

    return ProcInfo;
}

/*
 *  ProcInfoGet() - Allocates and initializes a 'FILE_INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *  @Data: Pointer to a FLT_CALLBACK_DATA object containing process-related data.
 *
 *  'ProcInfoGet()' is a wrapper for 'ProcInfoAlloc()' and 'ProcInfoInit()',
 *  providing a simple interface to obtain an allocated and initialized
 *  'PROC_INFO' structure.
 *
 *  Returns:
 *    - Pointer to the allocated 'PROC_INFO' structure on success.
 *    - 'NULL' if memory allocation or initialization fails.
 */
PPROC_INFO ProcInfoGet(_PoolType_ POOL_TYPE PoolType, _In_ PFLT_CALLBACK_DATA Data) {

    NTSTATUS Status;
    PPROC_INFO ProcInfo;

    ProcInfo = ProcInfoAlloc(PoolType);
    if (!ProcInfo) {
        return NULL;
    }

    Status = ProcInfoInit(ProcInfo, Data);
    if (!NT_SUCCESS(Status)) {
        ProcInfoFree(&ProcInfo);
    }

    return ProcInfo;
}

/*
 *  GetTID() -
 *
 *  @TID:
 *  @Data:
 */
static inline void GetTID(_Inout_ PHANDLE TID, _In_ PFLT_CALLBACK_DATA Data) {

    *TID = PsGetThreadId(Data->Thread);
}

/*
 *  GetSID() -
 *
 *  @SID :
 *  @Data:
 *
 *  Return:
 *    -
 *    -
 */
static inline NTSTATUS GetSID(_Inout_ PULONG SID, _In_ PFLT_CALLBACK_DATA Data) {

    /*
     *  Even though a process may not have a session ID, we consider
     *  it a success.
     */
    return FltGetRequestorSessionId(Data, SID);
}

/*
 *  GetPID() - Retrieves the Process ID.
 *
 *  @PID : Process ID return.
 *  @Data: Pointer to a 'FLT_CALLBACK_DATA' structure containing information
 *         about the callback.
 *
 *  Return:
 *    - Process ID.
 */
static inline void GetPID(_Out_ PULONG PID, _In_ PFLT_CALLBACK_DATA Data) {

    /*
     *  Gets the PID for the process associated with the thread that
     *  requested the I/O operation.
     */
    *PID = FltGetRequestorProcessId(Data);
}

/*
 *  GetIDs() -
 *
 *  @ProcInfo:
 *  @Data:
 *
 *  Return:
 *    -
 *    -
 */
static NTSTATUS GetIDs(_Inout_ PPROC_INFO ProcInfo, _In_ PFLT_CALLBACK_DATA Data) {

    NTSTATUS Status;

    GetPID(&ProcInfo->PID, Data);
    GetTID(&ProcInfo->TID, Data);

    Status = GetSID(&ProcInfo->SID, Data);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    return Status;
}

/*
 *  ProcInitInfo() - Initializes a 'PROC_INFO' structure based on
 *                   'FLT_CALLBACK_DATA'.
 *
 *  @ProcInfo: Pointer to the 'PROC_INFO' structure to be initialized.
 *  @Data: Pointer to the 'FLT_CALLBACK_DATA' structure containing process
 *         related data.
 *
 *  Return:
 *    - Status code indicating the result of the initialization.
 *    - Upon success, returns 'STATUS_SUCCESS'.
 *    - The appropriate error code in case of failure.
 */
NTSTATUS ProcInfoInit(_Inout_ PPROC_INFO ProcInfo, _In_ PFLT_CALLBACK_DATA Data) {

    NTSTATUS Status;

    Status = GetIDs(ProcInfo, Data);
    if (NT_SUCCESS(Status)) {
        Status = TokenInfoGet(&ProcInfo->TokenInfo, (HANDLE)ProcInfo->PID);
        if(NT_SUCCESS(Status)) {
            Status = GetImageName((HANDLE)ProcInfo->PID, ProcInfo->ImageName);
        }
    }

    return Status;
}

/*
 *  ProcInfoFree() - Deallocates the memory associated with a 'PROC_INFO'
 *                   struct dynamically allocated.
 *
 *  @ProcInfo: Pointer to a reference of a 'PROC_INFO' structure.
 */
void ProcInfoFree(_Inout_ PPROC_INFO* ProcInfo) {

    if (ProcInfo && *ProcInfo) {
        ExFreePoolWithTag(*ProcInfo, 'prif');

        /*
         *  In order to avoid a dangling pointer, after deallocating the
         *  'PROC_INFO' structure, we set the reference to the 'PROC_INFO'
         *  struct to NULL.
         */
        *ProcInfo = NULL;
    }
}

/*
 *  ProcInfoCopy() -
 *
 *  @Dest:
 *  @Src:
 *
 *  Return:
 *    -
 *    -
 */
void ProcInfoCopy(_Out_ PPROC_INFO_STATIC Dest, _In_ PPROC_INFO Src) {

    if (Dest && Src) {
        RtlCopyMemory(Dest->ImageName, Src->ImageName, sizeof Dest->ImageName);

        RtlCopyMemory(&Dest->PID, &Src->PID, sizeof Dest->PID);
        RtlCopyMemory(&Dest->SID, &Src->SID, sizeof Dest->SID);
        RtlCopyMemory(&Dest->TID, &Src->TID, sizeof Dest->TID);

        TokenInfoCopy(&Dest->TokenInfo, &Src->TokenInfo);
    }
}


