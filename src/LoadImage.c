
#include "loadimage.h"

/*
 *  LoadImageInfoAlloc() - Allocate a 'LOAD_IMAGE_INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'LOAD_IMAGE_INFO' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
PLOAD_IMAGE_INFO LoadImageInfoAlloc(_In_ POOL_TYPE PoolType) {

    PLOAD_IMAGE_INFO LoadImageInfo;

    LoadImageInfo = ExAllocatePool2(PoolType, sizeof *LoadImageInfo, 'load');
    if (!LoadImageInfo) {
        return NULL;
    }

    RtlZeroMemory(LoadImageInfo, sizeof *LoadImageInfo);

    /*
     *  Store the pool type used for the allocation in 'LoadImageInfo' to ensure
     *  correct memory handling, regardless of whether the caller routine is
     *  paged or nonpaged.
     */
    LoadImageInfo->PoolType = PoolType;

    return LoadImageInfo;
}

/*
 *  LoadImageInfoGet() - Allocate and initialize a 'LOAD_IMAGE_INFO' structure
 *
 *  @PoolType: The type of pool memory to allocate.
 *  @ImageInfo: Pointer to the IMAGE_INFO structure.
 *  @FullImageName: Pointer to the full image name.
 *  @PID: Process ID.
 *
 *  Return:
 *    -
 *    -
 */
PLOAD_IMAGE_INFO LoadImageInfoGet(_In_ POOL_TYPE PoolType, _In_ PLOAD_IMAGE_DATA LoadData) {

    NTSTATUS Status;
    PLOAD_IMAGE_INFO LoadImageInfo;

    LoadImageInfo = LoadImageInfoAlloc(PoolType);
    if (!LoadImageInfo) {
        return NULL;
    }

    LoadImageInfo->PID = LoadData->ProcessId;
    Status = LoadImageInfoInit(LoadImageInfo, LoadData->ImageInfo, LoadData->FullImageName);
    if (!NT_SUCCESS(Status)) {
        LoadImageInfoFree(&LoadImageInfo);
    }

    return LoadImageInfo;
}

/*
 *  GetExtendedInfo() - Get the extended information from an IMAGE_INFO structure
 *
 *  @LoadImageInfo: Pointer to the LOAD_IMAGE_INFO structure
 *  @ImageInfo: Pointer to the IMAGE_INFO structure
 *
 *  Return:
 *    - 
 *    -
 */
NTSTATUS GetExtendedInfo(_In_ PLOAD_IMAGE_INFO LoadImageInfo, _In_ PIMAGE_INFO ImageInfo) {

    NTSTATUS Status;
    PIMAGE_INFO_EX ImageInfoEx;
    PUNICODE_STRING FileName;

    ImageInfoEx = CONTAINING_RECORD(ImageInfo, IMAGE_INFO_EX, ImageInfo);
    FileName = &ImageInfoEx->FileObject->FileName;

    LoadImageInfo->FileNameSize = MAX_PATH;
    Status = UnicodeStrToStaticWSTR(LoadImageInfo->FileName, FileName, &LoadImageInfo->FileNameSize);
    if (!NT_SUCCESS(Status)) {
        ExFreePoolWithTag(LoadImageInfo->FullImageName, 'wstr');
    }

    return Status;
}

/*
 *  InitLoadImageInfoFields() - Initialize the fields of a LOAD_IMAGE_INFO structure
 *
 *  @LoadImageInfo: Pointer to the LOAD_IMAGE_INFO structure
 *  @ImageInfo: Pointer to the IMAGE_INFO structure
 *  @FullImageName: Pointer to the full image name
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS InitLoadImageInfoFields(_In_ PLOAD_IMAGE_INFO LoadImageInfo, _In_ PIMAGE_INFO ImageInfo, _In_ PUNICODE_STRING FullImageName) {

    NTSTATUS Status;

    LoadImageInfo->FullImageNameSize = MAX_PATH;
    Status = UnicodeStrToStaticWSTR(LoadImageInfo->FullImageName, FullImageName, &LoadImageInfo->FullImageNameSize);
    if (!NT_SUCCESS(Status)) {
        ExFreePoolWithTag(LoadImageInfo->FullImageName, 'wstr');
        return Status;
    }

    if(ImageInfo->ExtendedInfoPresent) {
        Status = GetExtendedInfo(LoadImageInfo, ImageInfo);
    }

    return Status;
}

/*
 *  LoadImageInfoInit() - Initialize a LOAD_IMAGE_INFO structure
 *
 *  @LoadImageInfo: Pointer to the LOAD_IMAGE_INFO structure
 *  @ImageInfo: Pointer to the IMAGE_INFO structure
 *  @FullImageName: Pointer to the full image name
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS LoadImageInfoInit(_In_ PLOAD_IMAGE_INFO LoadImageInfo, _In_ PIMAGE_INFO ImageInfo, _In_ PUNICODE_STRING FullImageName) {

    NTSTATUS Status;

    Status = InitLoadImageInfoFields(LoadImageInfo, ImageInfo, FullImageName);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    RtlCopyMemory(&LoadImageInfo->ImageInfo, ImageInfo, sizeof *ImageInfo);

    return Status;
}

/*
 *  LoadImageCopy() -
 *
 *  @Dest:
 *  @Src:
 */
void LoadImageCopy(_Out_ PLOAD_IMAGE_INFO_STATIC Dest, _In_ PLOAD_IMAGE_INFO Src) {

    if (Dest && Src) {

        Dest->PID = Src->PID;
        Dest->FullImageNameSize = Src->FullImageNameSize;
        Dest->FileNameSize = Src->FileNameSize;

        RtlCopyMemory(&Dest->ImageInfo, &Src->ImageInfo, sizeof Src->ImageInfo);
        RtlCopyMemory(Dest->FullImageName, Src->FullImageName, sizeof Dest->FullImageName);
        RtlCopyMemory(Dest->FileName, Src->FileName, sizeof Dest->FileName);
    }
}

/*
 *   LoadImageInfoDeInit() - Deinitialize a LOAD_IMAGE_INFO structure
 *
 *   @LoadImageInfo: Pointer to the LOAD_IMAGE_INFO structure
 */
void LoadImageInfoDeInit(_In_ PLOAD_IMAGE_INFO LoadImageInfo) {

    if (LoadImageInfo) {
        ExFreePoolWithTag(LoadImageInfo->FullImageName, 'wstr');
        ExFreePoolWithTag(LoadImageInfo->FileName, 'wstr');
    }
}

/*
 *   LoadImageInfoFree() - Free a LOAD_IMAGE_INFO structure
 *
 *  @LoadImageInfo: Pointer to the pointer of the LOAD_IMAGE_INFO structure
 */
void LoadImageInfoFree(_Inout_ PLOAD_IMAGE_INFO* LoadImageInfo) {

    if(LoadImageInfo && *LoadImageInfo) {
        LoadImageInfoDeInit(*LoadImageInfo);
        ExFreePoolWithTag(*LoadImageInfo, 'load');
        *LoadImageInfo = NULL;
    }

}
