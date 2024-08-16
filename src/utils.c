
#include "utils.h"

/*
 *  InitUnicodeString() -
 *
 *  @Str:
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS InitUnicodeString(_Inout_ PUNICODE_STRING Str) {

    if(Str) {
        Str->Buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, MAX_STR_SIZE, 'buf');
        if(Str->Buffer) {
            Str->Length = 0;
            Str->MaximumLength = MAX_STR_SIZE;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_UNSUCCESSFUL;
}

/*
 *  UnicodeStrToWSTR() -
 *
 *  @PoolType:
 *  @Src:
 *  @Dest:
 *  @pSize:
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS UnicodeStrToWSTR(_In_ POOL_TYPE PoolType, _In_ PUNICODE_STRING Src, _Out_ PWSTR* Dest, _Out_ PULONG pSize) {

    ULONG Size;
    PWSTR Wstr;
    SIZE_T Ind;
    __try {
        /*
         *  As 'UNICODE_STRING::Length' does not include the '/0', we
         *  add '1' to it.
         */
        Size = Src->Length + sizeof * Wstr;
        Wstr = ExAllocatePool2(PoolType, Size, 'wstr');
        if (!Wstr) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        /* null terminate the string. */
        Ind = Size / sizeof * Wstr;
        Wstr[Ind - 1] = 0;
        RtlCopyMemory(Wstr, Src->Buffer, Src->Length);

        *pSize = Size;
        *Dest = Wstr;

        return STATUS_SUCCESS;
    }except(EXCEPTION_CONTINUE_EXECUTION) {
        return STATUS_UNSUCCESSFUL;
    }
}

/*
 *  UnicodeSTrToStaticWSTR() -
 *
 *  @Dest:
 *  @Src:
 *  @pSize:
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS UnicodeStrToStaticWSTR(_Inout_ WCHAR Dest[MAX_PATH], _In_ PUNICODE_STRING Src, _Inout_ PULONG pSize) {

    ULONG Min;
    __try {
        Min = min(MAX_PATH - 1, Src->Length / sizeof(WCHAR));

        RtlZeroMemory(Dest, MAX_PATH * sizeof * Dest);
        RtlCopyMemory(Dest, Src->Buffer, Min * sizeof(WCHAR));

        *pSize = Min;

        return STATUS_SUCCESS;
    } except(EXCEPTION_CONTINUE_EXECUTION){
            return STATUS_UNSUCCESSFUL;
    }
}

/*
 *  BinaryDataToUnicodeSTR() -
 *
 *  @BinData:
 *  @UnicodeStr:
 *  @DataSize:
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS BinaryDataToUnicodeSTR(_In_ PVOID BinData, _Inout_ PUNICODE_STRING UnicodeStr, _In_ ULONG DataSize) {

    NTSTATUS Status;
    UNICODE_STRING hexValue;

    Status = STATUS_UNSUCCESSFUL;

    if(BinData && UnicodeStr) {
        Status = InitUnicodeString(&hexValue);
        if(!NT_SUCCESS(Status)) {
            return Status;
        }

        for (ULONG i = 0; i < DataSize; ++i) {
            RtlIntegerToUnicodeString(((UCHAR*)BinData)[i], 16, &hexValue);
            RtlUnicodeStringCat(UnicodeStr, &hexValue);
            RtlUnicodeStringCatString(UnicodeStr, L" ");
        }

        ExFreePool(hexValue.Buffer);
    }

    return Status;
}

/*
 *  WORDDataToUnicodeSTR() -
 *
 *  @WordData:
 *  @UnicodeStr:
 *  @DataSize:
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS WORDDataToUnicodeSTR(_In_ PVOID WordData, _Inout_ PUNICODE_STRING UnicodeStr, _In_ ULONG DataSize) {

    NTSTATUS Status;
    UNICODE_STRING hexValue;

    Status = STATUS_UNSUCCESSFUL;

    if(WordData && UnicodeStr) {
        Status = InitUnicodeString(&hexValue);
        if(!NT_SUCCESS(Status)) {
            return Status;
        }

        for (ULONG i = DataSize; i > 0; --i) {
            RtlIntegerToUnicodeString(((UCHAR*)WordData)[i], 16, &hexValue);
            RtlUnicodeStringCat(UnicodeStr, &hexValue);
            RtlUnicodeStringCatString(UnicodeStr, L" ");
        }

        ExFreePoolWithTag(hexValue.Buffer, 'buf');
    }

    return Status;
}


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
NTSTATUS GetImageName(_In_ HANDLE ID, _Out_ PWSTR ImageBuf) {

    NTSTATUS Status;
    PEPROCESS eProc;
    UNICODE_STRING ImageName;
    PUNICODE_STRING pImageName; // Callers of RtlCopyMemory can be running at any IRQL if the sour
    ULONG ImageNameSize;

    line();
    pImageName = &ImageName;
    line();
    Status     = InitUnicodeString(pImageName);
    line();
    if(!NT_SUCCESS(Status)) {
        line();
        return Status;
    }

    line();
    Status = PsLookupProcessByProcessId(ID, &eProc);
    line();
    if (!NT_SUCCESS(Status)) {
        line();
        return Status;
    }

    line();
    Status = SeLocateProcessImageName(eProc, &pImageName);
    line();
    if (!NT_SUCCESS(Status)) {
        line();
        ObDereferenceObject(eProc);
        line();
        return Status;
    }


    line();
    ImageNameSize = MAX_PATH;
    line();
    UnicodeStrToStaticWSTR(ImageBuf, pImageName, &ImageNameSize);
    line();
    ExFreePoolWithTag(ImageName.Buffer, 'buf');
    RtlFreeUnicodeString(pImageName);
    line();
    ObDereferenceObject(eProc);
    line();

    return Status;
}

