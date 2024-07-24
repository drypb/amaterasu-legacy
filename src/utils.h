#ifndef UTILS_H
#define UTILS_H

#include "common.h"
#include "fileinfo.h"

extern NTSTATUS
UnicodeStrToWSTR(
    _In_ POOL_TYPE PoolType,
    _In_ PUNICODE_STRING Src,
    _Out_ PWSTR* Dest,
    _Out_ PULONG pSize
);

extern NTSTATUS
InitUnicodeString(
    PUNICODE_STRING Str
);

extern NTSTATUS
BinaryDataToUnicodeSTR(
    PVOID BinData,
    PUNICODE_STRING UnicodeStr,
    ULONG DataSize
);

extern NTSTATUS
WORDDataToUnicodeSTR(
    PVOID WordData,
    PUNICODE_STRING UnicodeStr,
    ULONG DataSize
);

extern NTSTATUS
UnicodeStrToStaticWSTR(
    _Inout_ WCHAR Dest[MAX_PATH],
    _In_ PUNICODE_STRING Src,
    _Inout_ PULONG pSize
);

/*
 *  GetImageName() -
 *
 *  @ID:
 *  @ImageBuf:
 *
 *  Return:
 *    -
 *    -
 */
extern NTSTATUS 
GetImageName(
    _In_ HANDLE ID, 
    _Out_ PWSTR ImageBuf
);

#endif  /* UTILS_H */
