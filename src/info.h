#ifndef INFO_H
#define INFO_H

#include "loadimage.h"
#include "procdata.h"
#include "reginfo.h"
#include "fsinfo.h"
#include "common.h"


enum InfoType {
    INFO_FS, INFO_PROC, INFO_REG, INFO_LOAD
};

typedef enum InfoType INFO_TYPE, * PINFO_TYPE;

struct Info {

    POOL_TYPE   PoolType;
    TIME_FIELDS TimeFields;
    INFO_TYPE   InfoType;

    union {
        PFS_INFO    FsInfo;
        PREG_INFO   RegInfo;
        PPROC_DATA  ProcData;
        PLOAD_IMAGE_INFO LoadImageInfo;
        PVOID       Data;
    } Info;

    LIST_ENTRY  ListEntry;
};

typedef struct Info INFO, * PINFO;

struct InfoStatic {

    TIME_FIELDS TimeFields;
    INFO_TYPE   InfoType;

    union {
        FS_INFO_STATIC   FsInfo;
        PROC_DATA_STATIC ProcData;
        REG_INFO_STATIC  RegInfo;
        LOAD_IMAGE_INFO_STATIC LoadImageInfo;
        PVOID Data;
    } Info;
};

typedef struct InfoStatic INFO_STATIC, * PINFO_STATIC;

/*
 *  InfoAlloc() - Allocate a 'INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'INFO' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
extern PINFO
InfoAlloc(
    _PoolType_ POOL_TYPE PoolType
);

/*
 *  InfoGet() - Allocates and initializes a 'INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *  @Data:
 *  @InfoType:
 *
 *  'InfoGet()' is a wrapper for 'InfoAlloc()' and 'InfoInit()',
 *  providing a simple interface to obtain an allocated and initialized
 *  'INFO' structure.
 *
 *  Returns:
 *    - Pointer to the allocated 'INFO' structure on success.
 *    - 'NULL' if memory allocation or initialization fails.
 */
extern PINFO
InfoGet(
    _PoolType_ POOL_TYPE PoolType,
    _In_ PVOID Data,
    _In_ INFO_TYPE InfoType
);

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
extern NTSTATUS
InfoInit(
    _Out_ PINFO Info,
    _In_ PVOID Data,
    _In_ INFO_TYPE InfoType
);

/*
 *  InfoDeInit() - Deinitializes and frees the internal resources
 *                 associated with a 'INFO' structure.
 *
 *  @Info: Pointer to the 'INFO' structure to be deinitialized.
 */
extern void
InfoDeInit(
    _Inout_ PINFO Info
);

/*
 *  InfoFree() - Deallocates the memory associated with a 'INFO'
 *               struct dynamically allocated.
 *
 *  @Info: Pointer to a reference of a 'INFO' structure.
 */
extern void
InfoFree(
    _Inout_ PINFO* Info
);

/*
 *  InfoCopt() -
 *
 *  @Dest:
 *  @Src:
 */
extern void
InfoCopy(
    _Inout_ PINFO_STATIC Dest,
    _In_ PINFO Src
);

#endif  /* INFO_H */
