
#include "fileinfo.h"

/*
 *  FileInfoAlloc() - Allocate a 'FILE_INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'FILE_INFO' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
PFILE_INFO FileInfoAlloc(_PoolType_ POOL_TYPE PoolType) {

    PFILE_INFO FileInfo;

    FileInfo = ExAllocatePool2(PoolType, sizeof * FileInfo, 'file');
    if (!FileInfo) {
        return NULL;
    }

    RtlZeroMemory(FileInfo, sizeof * FileInfo);

    /*
     *  Store the pool type used for the allocation in 'FileInfo' to ensure
     *  correct memory handling, regardless of whether the caller routine is
     *  paged or nonpaged.
     */
    FileInfo->PoolType = PoolType;

    return FileInfo;
}

/*
 *  FileInfoGet() - Allocates and initializes a 'FILE_INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *  @Data: Pointer to a FLT_CALLBACK_DATA object containing file-related data.
 *
 *  'GetFileInfo()' is a wrapper for 'FileInfoAlloc()' and 'FileInfoInit()',
 *  providing a simple interface to obtain an allocated and initialized
 *  'FILE_INFO' structure.
 *
 *  Returns:
 *    - Pointer to the allocated 'FILE_INFO' structure on success.
 *    - 'NULL' if memory allocation or initialization fails.
 */
PFILE_INFO FileInfoGet(_PoolType_ POOL_TYPE PoolType, _In_ PFLT_CALLBACK_DATA Data) {

    NTSTATUS Status;
    PFILE_INFO FileInfo;

    FileInfo = FileInfoAlloc(PoolType);
    if (!FileInfo) {
        return NULL;
    }

    Status = FileInfoInit(FileInfo, Data);
    if (!NT_SUCCESS(Status)) {
        FileInfoFree(&FileInfo);
        return NULL;
    }

    return FileInfo;
}

/*
 *  InitFileInfoFields() -
 *
 *  @FileInfo:
 *  @NameInfo:
 *
 *  Return:
 *    -
 *    -
 */
static NTSTATUS InitFileInfoFields(_Out_ PFILE_INFO FileInfo, _In_ PFLT_FILE_NAME_INFORMATION NameInfo) {

    ULONG Len;

    Len = 0;
    UnicodeStrToStaticWSTR(FileInfo->Path, &NameInfo->Name, &Len);
    UnicodeStrToStaticWSTR(FileInfo->Name, &NameInfo->FinalComponent, &Len);

    return STATUS_SUCCESS;
}

/*
 *  GetNameInfo() -
 *
 *  @NameInfo:
 *  @Data:
 *  @QueryMethod:
 *
 *  Return:
 *    -
 *    -
 */
static NTSTATUS GetNameInfo(_Out_ PFLT_FILE_NAME_INFORMATION* NameInfo, _In_ PFLT_CALLBACK_DATA Data) {

    NTSTATUS Status;
    ULONG QueryMethod;

    QueryMethod = FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP;

    Status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | QueryMethod, NameInfo);
    if (!NT_SUCCESS(Status)) {
        /*
         *  If it was not possible to get the "normalized" name, we try
         *  to get the "opened" name. The opened name refers to the name of the file
         *  as it was specified when the file was opened, including the full path
         *  if available.
         */
        Status = FltGetFileNameInformation(Data, FLT_FILE_NAME_OPENED | QueryMethod, NameInfo);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    Status = FltParseFileNameInformation(*NameInfo);
    if (!NT_SUCCESS(Status)) {
        FltReleaseFileNameInformation(*NameInfo);
    }

    return Status;
}

/*
 *  FileInfoInit() - Initializes a 'FILE_INFO' structure allocate by 'FileInfoAlloc()'
 *                   based on 'FLT_CALLBACK_DATA'.
 *
 *  @FileInfo: Pointer to the 'FILE_INFO' structure to be initialized.
 *  @Data: Pointer to the 'FLT_CALLBACK_DATA' structure containing file-related
 *         and process related data.
 *
 *  Return:
 *    - Status code indicating the result of the initialization.
 *    - Upon success, returns 'STATUS_SUCCESS'.
 *    - The appropriate error code in case of failure.
 */
NTSTATUS FileInfoInit(_Out_ PFILE_INFO FileInfo, _In_ PFLT_CALLBACK_DATA Data) {

    NTSTATUS Status;
    PFLT_FILE_NAME_INFORMATION NameInfo;

    Status = GetNameInfo(&NameInfo, Data);
    if (!NT_SUCCESS(Status)) {
        Assert(NT_SUCCESS(Status), "by GetNameInfo().");
        return Status;
    }

    Status = InitFileInfoFields(FileInfo, NameInfo);
    if (!NT_SUCCESS(Status)) {
        Assert(NT_SUCCESS(Status), "by InitFileInfoFields().");
    }

    /* As 'NameInfo' is no longer necessary, we deallocate it. */
    FltReleaseFileNameInformation(NameInfo);

    return Status;
}

/*
 *  FileInfoFree() - Deallocates the memory associated with a 'FILE_INFO'
 *                   struct dynamically allocated.
 *
 *  @FileInfo: Pointer to a reference of a 'FILE_INFO' structure.
 */
void FileInfoFree(_Inout_ PFILE_INFO* FileInfo) {

    if (FileInfo && *FileInfo) {
        ExFreePoolWithTag(*FileInfo, 'file');

        /*
         *  In order to avoid a dangling pointer, after deallocating the
         *  'FILE_INFO' structure, we set the reference to the 'FILE_INFO'
         *  struct to NULL.
         */
        *FileInfo = NULL;
    }
}

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
void FileInfoCopy(_Out_ PFILE_INFO_STATIC Dest, _In_ PFILE_INFO Src) {

    if (Dest && Src) {
        RtlCopyMemory(Dest->Name, Src->Name, sizeof Dest->Name);
        RtlCopyMemory(Dest->Path, Src->Path, sizeof Dest->Path);
    }
}
