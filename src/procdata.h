#ifndef PROC_DATA_H
#define PROC_DATA_H

#include "common.h"
#include "token.h"

struct IDENTIFIER {

    HANDLE PPID;

    union {
        HANDLE PID;
        HANDLE TID;
        HANDLE ID;
    } Id;

    BOOLEAN isThread;
    BOOLEAN Active;
};

typedef struct IDENTIFIER IDENTIFIER, * PIDENTIFIER;

struct ProcData {

    POOL_TYPE PoolType;

    IDENTIFIER Ids;
    TOKEN_INFO TokenInfo;

    WCHAR ParentName[MAX_PATH];
    WCHAR ChildName[MAX_PATH];
};

typedef struct ProcData PROC_DATA, * PPROC_DATA;

struct ProcDataStatic {

    IDENTIFIER Ids;
    TOKEN_INFO_STATIC TokenInfo;

    WCHAR ParentName[MAX_PATH];
    WCHAR ChildName[MAX_PATH];
};

typedef struct ProcDataStatic PROC_DATA_STATIC, * PPROC_DATA_STATIC;

/*
 *  ProcDataAlloc() - Allocate a 'PROC_DATA' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'PROC_DATA' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
extern PPROC_DATA
ProcDataAlloc(
    _In_ POOL_TYPE PoolType
);

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
extern PPROC_DATA
ProcDataGet(
    _In_ POOL_TYPE PoolType,
    _In_ PIDENTIFIER Data
);

/*
 *  ProcInitInfo() - Initializes a 'PROC_DATA' struct.
 *
 *  @ProcData: Pointer to the 'PROC_DATA' structure to be initialized.
 *  @Data: Pointer to a 'IDENTIFIR' structure containing process
 *         related data.
 *
 *  Return:
 *    - Status code indicating the result of the initialization.
 *    - Upon success, returns 'STATUS_SUCCESS'.
 *    - The appropriate error code in case of failure.
 */
extern NTSTATUS
ProcDataInit(
    _Out_ PPROC_DATA ProcData,
    _In_ PIDENTIFIER IDs
);

/*
 *  ProcDataFree() - Deallocates the memory associated with a 'PROC_DATA'
 *                   struct dynamically allocated.
 *
 *  @ProcData: Pointer to a reference of a 'PROC_DATA' structure.
 */
extern void
ProcDataFree(
    _Inout_ PPROC_DATA* ProcData
);

/*
 *  ProcDataCopy() -
 *
 *  @Dest:
 *  @Src:
 */
extern void
ProcDataCopy(
    _Out_ PPROC_DATA_STATIC Dest,
    _In_ PPROC_DATA Src
);

#endif  /* PROC_H */
