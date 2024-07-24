#ifndef FILE_INFO_H
#define FILE_INFO_H

#include "common.h"
#include "utils.h"

struct FileInfo {

    POOL_TYPE PoolType;

    WCHAR Name[MAX_PATH];
    WCHAR Path[MAX_PATH];
};

typedef struct FileInfo FILE_INFO, * PFILE_INFO;

struct FileInfoStatic {

    WCHAR Name[MAX_PATH];
    WCHAR Path[MAX_PATH];
};

typedef struct FileInfoStatic FILE_INFO_STATIC, * PFILE_INFO_STATIC;

/*
 *  FileInfoAlloc() - Allocate a 'FILE_INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'FILE_INFO' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
extern PFILE_INFO
FileInfoAlloc(
    _PoolType_ POOL_TYPE PoolType
);

/*
 *  FileInfoGet() - Allocates and initializes a 'FILE_INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *  @Data: Pointer to a FLT_CALLBACK_DATA object containing file-related data.
 *
 *  'FileInfoGet()' is a wrapper for 'FileInfoAlloc()' and 'FileInfoInit()',
 *  providing a simple interface to obtain an allocated and initialized
 *  'FILE_INFO' structure.
 *
 *  Returns:
 *    - Pointer to the allocated 'FILE_INFO' structure on success.
 *    - 'NULL' if memory allocation or initialization fails.
 */
extern PFILE_INFO
FileInfoGet(
    _PoolType_ POOL_TYPE PoolType,
    _In_ PFLT_CALLBACK_DATA Data
);

/*
 *  FileInfoInit() - Initializes a 'FILE_INFO' structure allocate by 'FileInfoAlloc()'
 *                   based on 'FLT_CALLBACK_DATA'.
 *
 *  @Info: Pointer to the 'FILE_INFO' structure to be initialized.
 *  @Data: Pointer to the 'FLT_CALLBACK_DATA' structure containing file-related
 *         and process related data.
 *
 *  Return:
 *    - Status code indicating the result of the initialization.
 *    - Upon success, returns 'STATUS_SUCCESS'.
 *    - The appropriate error code in case of failure.
 */
extern NTSTATUS
FileInfoInit(
    _Out_ PFILE_INFO FileInfo,
    _In_ PFLT_CALLBACK_DATA Data
);

/*
 *  FileInfoFree() - Deallocates the memory associated with a 'FILE_INFO'
 *                   struct dynamically allocated.
 *
 *  @FileInfo: Pointer to a reference of a 'FILE_INFO' structure.
 */
extern void
FileInfoFree(
    _Inout_ PFILE_INFO* FileInfo
);


/*
 *  FileInfoCopy() -
 *
 *  @Dest:
 *  @Src:
 *
 *  Return:
 *    -
 *    -
 */
extern void
FileInfoCopy(
    _Out_ PFILE_INFO_STATIC Dest,
    _In_ PFILE_INFO Src
);

#endif  /* FILE_INFO_H */
