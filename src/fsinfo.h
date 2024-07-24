#ifndef FSINFO_H
#define FSINFO_H

#include "fileinfo.h"
#include "procinfo.h"

struct FsInfo {

    POOL_TYPE   PoolType;

    UCHAR       MjFunc;
    PFILE_INFO  FileInfo;
    PPROC_INFO  ProcInfo;
};

typedef struct FsInfo FS_INFO, * PFS_INFO;

struct FsInfoStatic {

    UCHAR       MjFunc;
    FILE_INFO_STATIC  FileInfo;
    PROC_INFO_STATIC  ProcInfo;
};

typedef struct FsInfoStatic FS_INFO_STATIC, * PFS_INFO_STATIC;

/*
 *  FsInfoAlloc() - Allocate a 'FS_INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'FS_INFO' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
extern PFS_INFO
FsInfoAlloc(
    _PoolType_ POOL_TYPE PoolType
);

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
extern PFS_INFO
FsInfoGet(
    _PoolType_ POOL_TYPE PoolType,
    _In_ PVOID Data
);

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
extern NTSTATUS
FsInfoInit(
    _Out_ PFS_INFO FsInfo,
    _In_ PFLT_CALLBACK_DATA Data
);

/*
 *  FsInfoDeInit() - Deinitializes and frees the internal resources
 *                   associated with a 'FS_INFO' structure.
 *
 *  @FsInfo: Pointer to the 'FS_INFO' structure to be deinitialized.
 */
extern void
FsInfoDeInit(
    _Inout_ PFS_INFO FsInfo
);

/*
 *  FsInfoFree() - Deallocates the memory associated with a 'FS_INFO'
 *                 struct dynamically allocated.
 *
 *  @FsInfo: Pointer to a reference of a 'FS_INFO' structure.
 */
extern void
FsInfoFree(
    _Inout_ PFS_INFO* FsInfo
);

/*
 *  FsInfoCopy() -
 *
 *  @Dest:
 *  @Src:
 *
 *  Retu
 */
extern void
FsInfoCopy(
    _Out_ PFS_INFO_STATIC Dest,
    _In_ PFS_INFO Src
);

#endif  /* FSFS_INFO_H */
