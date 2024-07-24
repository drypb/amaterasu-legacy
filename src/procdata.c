
#include "procdata.h"
#include "utils.h"

/*
 *  ProcDataAlloc() - Allocate a 'PROC_DATA' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'PROC_DATA' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
PPROC_DATA ProcDataAlloc(_In_ POOL_TYPE PoolType) {

    PPROC_DATA ProcData;

    ProcData = ExAllocatePool2(PoolType, sizeof * ProcData, 'prdt');
    if (!ProcData) {
        return NULL;
    }

    RtlZeroMemory(ProcData, sizeof * ProcData);

    /*
     *  Store the pool type used for the allocation in 'ProcData' to ensure
     *  correct memory handling, regardless of whether the caller routine is
     *  paged or nonpaged.
     */
    ProcData->PoolType = PoolType;

    return ProcData;
}

/*
 *  ProcDataGet() - Allocates and initializes a 'FILE_INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *  @Data: Pointer to a IDENTIFIR struct containing process related data.
 *
 *  'ProcDataGet()' is a wrapper for 'ProcDataAlloc()' and 'ProcDataInit()',
 *  providing a simple interface to obtain an allocated and initialized
 *  'PROC_DATA' structure.
 *
 *  Returns:
 *    - Pointer to the allocated 'PROC_DATA' structure on success.
 *    - 'NULL' if memory allocation or initialization fails.
 */
PPROC_DATA ProcDataGet(_In_ POOL_TYPE PoolType, _In_ PIDENTIFIER Data) {

    NTSTATUS Status;
    PPROC_DATA ProcData;

    ProcData = ProcDataAlloc(PoolType);
    if (!ProcData) {
        return NULL;
    }

    Status = ProcDataInit(ProcData, Data);
    if (!NT_SUCCESS(Status)) {
        ProcDataFree(&ProcData);
    }

    return ProcData;
}

/*
 *  ProcInitInfo() - Initializes a 'PROC_DATA' struct.
 *
 *  @ProcData: Pointer to the 'PROC_DATA' structure to be initialized.
 *  @Data: Pointer to a 'IDENTIFIER' structure containing process
 *         related data.
 *
 *  Return:
 *    - Status code indicating the result of the initialization.
 *    - Upon success, returns 'STATUS_SUCCESS'.
 *    - The appropriate error code in case of failure.
 */
NTSTATUS ProcDataInit(_Out_ PPROC_DATA ProcData, _In_ PIDENTIFIER Data) {

    NTSTATUS Status;

    Status = TokenInfoGet(&ProcData->TokenInfo, Data->PPID);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    RtlCopyMemory(&ProcData->Ids, Data, sizeof ProcData->Ids);

	Status = GetImageName(Data->PPID, ProcData->ParentName);
	if (NT_SUCCESS(Status)) {
        if (!ProcData->Ids.isThread) {
            Status = GetImageName(Data->Id.ID, ProcData->ChildName);
        }
	}

    return Status;
}

/*
 *  ProcDataFree() - Deallocates the memory associated with a 'PROC_DATA'
 *                   struct dynamically allocated.
 *
 *  @ProcData: Pointer to a reference of a 'PROC_DATA' structure.
 */
void ProcDataFree(_Inout_ PPROC_DATA* ProcData) {

    if (ProcData && *ProcData) {
        ExFreePoolWithTag(*ProcData, 'prdt');

        /*
         *  In order to avoid a dangling pointer, after deallocating the
         *  'PROC_DATA' structure, we set the reference to the 'PROC_DATA'
         *  struct to NULL.
         */
        *ProcData = NULL;
    }
}

/*
 *  IdentifierCopy() -
 *
 *  @Dest:
 *  @Src:
 */
static void IdentifierCopy(_Out_ PIDENTIFIER Dest, _In_ PIDENTIFIER Src) {

    if (Dest && Src) {

        RtlCopyMemory(&Dest->PPID, &Src->PPID, sizeof Dest->PPID);
        RtlCopyMemory(&Dest->Id.ID, &Src->Id.ID, sizeof Dest->Id.ID);
        RtlCopyMemory(&Dest->isThread, &Src->isThread, sizeof Dest->isThread);
        RtlCopyMemory(&Dest->Active, &Src->Active, sizeof Dest->Active);
    }
}

/*
 *  ProcDataCopy() -
 *
 *  @Dest:
 *  @Src:
 */
void ProcDataCopy(_Out_ PPROC_DATA_STATIC Dest, _In_ PPROC_DATA Src) {

    if (Dest && Src) {

        IdentifierCopy(&Dest->Ids, &Src->Ids);
        TokenInfoCopy(&Dest->TokenInfo, &Src->TokenInfo);
        RtlCopyMemory(Dest->ParentName, Src->ParentName, sizeof Dest->ParentName);
        RtlCopyMemory(Dest->ChildName, Src->ChildName, sizeof Dest->ChildName);
    }
}
