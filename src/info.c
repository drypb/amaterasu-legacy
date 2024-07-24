
#include "info.h"

struct BaseFuncs {

    PVOID (*get) (_In_ POOL_TYPE, _In_ PVOID);
    VOID  (*free)(_Inout_ PVOID*);
    VOID  (*copy)(_Out_ PVOID Dest, _In_ PVOID Src);

};

typedef struct BaseFuncs BASE_FUNCS, * PBASE_FUNCS;

static BASE_FUNCS funcs[] = {
    {&FsInfoGet       , &FsInfoFree       , &FsInfoCopy   },
    {&ProcDataGet     , &ProcDataFree     , &ProcDataCopy },
    {&RegInfoGet      , &RegInfoFree      , &RegInfoCopy  },
    {&LoadImageInfoGet, &LoadImageInfoFree, &LoadImageCopy}
};

/*
 *  InfoAlloc() - Allocate a 'INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'INFO' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
PINFO InfoAlloc(_PoolType_ POOL_TYPE PoolType) {

    PINFO Info;

    Info = ExAllocatePool2(PoolType, sizeof * Info, 'info');
    if (!Info) {
        return NULL;
    }

    RtlZeroMemory(Info, sizeof * Info);

    /*
     *  Store the pool type used for the allocation in 'Info' to ensure
     *  correct memory handling, regardless of whether the caller routine is
     *  paged or nonpaged.
     */
    Info->PoolType = PoolType;

    return Info;
}

/*
 *  InfoGet() - Allocates and initializes a 'INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *  @Data:
 *
 *  'InfoGet()' is a wrapper for 'InfoAlloc()' and 'InfoInit()',
 *  providing a simple interface to obtain an allocated and initialized
 *  'INFO' structure.
 *
 *  Returns:
 *    - Pointer to the allocated 'INFO' structure on success.
 *    - 'NULL' if memory allocation or initialization fails.
 */
PINFO InfoGet(_PoolType_ POOL_TYPE PoolType, _In_ PVOID Data, _In_ INFO_TYPE InfoType) {

    PINFO Info;
    NTSTATUS Status;

    Info = InfoAlloc(PoolType);
    if (!Info) {
        return NULL;
    }

    Status = InfoInit(Info, Data, InfoType);
    if (!NT_SUCCESS(Status)) {
        InfoFree(&Info);
    }

    return Info;
}

/*
 *  InitTimeFields() - Initializes a TIME_FIELDS structure with the current
 *                     local time.
 *
 *  @TimeFields: Pointer to the TIME_FIELDS structure to initialize.
 *
 */
static inline void InitTimeFields(_Out_ PTIME_FIELDS TimeFields) {

    LARGE_INTEGER SysTime;
    LARGE_INTEGER LocalTime;

    KeQuerySystemTime(&SysTime);
    ExSystemTimeToLocalTime(&SysTime, &LocalTime);
    RtlTimeToTimeFields(&LocalTime, TimeFields);
}


/*
 *  InfoInit() - Initializes a 'INFO' structure allocate by 'InfoAlloc()'
 *               based on 'Data'.
 *
 *  @Info: Pointer to the 'INFO' structure to be initialized.
 *  @Data:
 *
 *  Return:
 *    - Status code indicating the result of the initialization.
 *    - Upon success, returns 'STATUS_SUCCESS'.
 *    - The appropriate error code in case of failure.
 */
NTSTATUS InfoInit(_Out_ PINFO Info, _In_ PVOID Data, _In_ INFO_TYPE InfoType) {

    InitTimeFields(&Info->TimeFields);

    Info->InfoType = InfoType;
    Info->Info.Data = funcs[InfoType].get(Info->PoolType, Data);
    if (!Info->Info.Data) {
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

/*
 *  InfoDeInit() - Deinitializes and frees the internal resources
 *                 associated with a 'INFO' structure.
 *
 *  @Info: Pointer to the 'INFO' structure to be deinitialized.
 */
void InfoDeInit(_Inout_ PINFO Info) {

    if (Info) {
        funcs[Info->InfoType].free(&Info->Info.Data);
    }
}

/*
 *  InfoFree() - Deallocates the memory associated with a 'INFO'
 *               struct dynamically allocated.
 *
 *  @Info: Pointer to a reference of a 'INFO' structure.
 */
void InfoFree(_Inout_ PINFO* Info) {

    if (Info && *Info) {
        InfoDeInit(*Info);
        ExFreePoolWithTag(*Info, 'info');

        /*
         *  In order to avoid a dangling pointer, after deallocating the
         *  'INFO' structure, we set the reference to the 'INFO'
         *  struct to NULL.
         */
        *Info = NULL;
    }
}

/*
 *  InfoCopy() -
 *
 *  @Dest:
 *  @Src:
 *
 *  Return:
 *    -
 *    -
 */
void InfoCopy(_Inout_ PINFO_STATIC Dest, _In_ PINFO Src) {

    if (Dest && Src) {

        RtlCopyMemory(&Dest->TimeFields, &Src->TimeFields, sizeof Dest->TimeFields);
        Dest->InfoType = Src->InfoType;
        funcs[Dest->InfoType].copy(
            /*
             *  Could go wrong as we are using a placeholder
             *  for the types defined in the union.
             */
            &Dest->Info.Data,
            Src->Info.Data
        );
    }
}
