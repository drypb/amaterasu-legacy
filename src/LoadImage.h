#ifndef LOADIMAGE_H
#define LOADIMAGE_H

#include "common.h"
#include "utils.h"

struct LoadImageData {
    PUNICODE_STRING FullImageName;
    HANDLE ProcessId;
    PIMAGE_INFO ImageInfo;
};

typedef struct LoadImageData LOAD_IMAGE_DATA, * PLOAD_IMAGE_DATA;

struct LoadImageInfo {
    IMAGE_INFO ImageInfo;           // Image information structure
    HANDLE PID;                     // Process ID
    ULONG FullImageNameSize;        // Size of the full image name
    ULONG FileNameSize;             // Size of the file name
    WCHAR FullImageName[MAX_PATH];  // Pointer to the full image name
    WCHAR FileName[MAX_PATH];       // Pointer to the file name (if extended info is present)
    POOL_TYPE PoolType;             // Type of memory pool
};

typedef struct LoadImageInfo LOAD_IMAGE_INFO, * PLOAD_IMAGE_INFO;

struct LoadImageInfoStatic {
    IMAGE_INFO ImageInfo;           // Image information structure
    HANDLE PID;                     // Process ID
    ULONG FullImageNameSize;        // Size of the full image name
    ULONG FileNameSize;             // Size of the file name
    WCHAR FullImageName[MAX_PATH];  // Pointer to the full image name
    WCHAR FileName[MAX_PATH];       // Pointer to the file name (if extended info is present)
};

typedef struct LoadImageInfoStatic LOAD_IMAGE_INFO_STATIC, * PLOAD_IMAGE_INFO_STATIC;

/*
 *  LoadImageInfoAlloc() - Allocate a 'LOAD_IMAGE_INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'LOAD_IMAGE_INFO' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
extern PLOAD_IMAGE_INFO 
LoadImageInfoAlloc(
    _In_ POOL_TYPE PoolType
);

extern PLOAD_IMAGE_INFO
LoadImageInfoGet(
    _In_ POOL_TYPE PoolType,
    _In_ PLOAD_IMAGE_DATA LoadData
);

extern NTSTATUS
LoadImageInfoInit(
    _In_ PLOAD_IMAGE_INFO LoadImageInfo,
    _In_ PIMAGE_INFO ImageInfo,
    _In_ PUNICODE_STRING FullImageName
);

extern void
LoadImageInfoDeInit(
    _In_ PLOAD_IMAGE_INFO LoadImageInfo
);

extern void
LoadImageCopy(
    _Out_ PLOAD_IMAGE_INFO_STATIC Dest,
    _In_ PLOAD_IMAGE_INFO Src
);

extern void
LoadImageInfoFree(
    _Inout_ PLOAD_IMAGE_INFO* LoadImageInfo
);

#endif  /* LOADIMAGE_H */
