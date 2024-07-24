
#include "reginfo.h"

/*
 *  RegInfoAlloc() - Allocate a 'REG_INFO' structure.
 *
 *  @PoolType: The type of memory pool to allocate from (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'REG_INFO' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
PREG_INFO RegInfoAlloc(_PoolType_ POOL_TYPE PoolType) {

	PREG_INFO RegInfo;

	RegInfo = ExAllocatePool2(PoolType , sizeof * RegInfo, 'rifo');
	if (!RegInfo) {
		return NULL;
	}

	RtlZeroMemory(RegInfo, sizeof * RegInfo);

    /*
     *  Store the pool type used for the allocation in 'RegInfo' to ensure
     *  correct memory handling, regardless of whether the caller routine is
     *  paged or nonpaged.
     */
	RegInfo->PoolType = POOL_FLAG_NON_PAGED;

	return RegInfo;
}

/*
 *  RegInfoGet() -
 *
 *  @PoolType:
 *  @ReInfoData:
 *
 *  Return:
 *    -
 *    -
 */
PREG_INFO RegInfoGet(_PoolType_ POOL_TYPE PoolType, _In_ PREG_INFO_DATA RegInfoData) {

	PREG_INFO RegInfo;
	NTSTATUS Status;
	
	RegInfo = RegInfoAlloc(PoolType);
	if (!RegInfo) {
		return NULL;
	}

	Status = RegInfoInit(RegInfo, RegInfoData);
	if (!NT_SUCCESS(Status)) {
		RegInfoFree(&RegInfo);
	}

	return RegInfo;
}

/*
 *  GetRegBinaryData() -
 *
 *  @PoolType:
 *  @Data:
 *  @RegStruct:
 *
 *  Return:
 *    -
 *    -
 */
static NTSTATUS GetRegBinaryData(_PoolType_ POOL_TYPE PoolType, _Inout_ PUNICODE_STRING Data, _In_ PREG_SET_VALUE_KEY_INFORMATION RegStruct) {

    NTSTATUS Status;
    PVOID Buffer;

    Status = STATUS_UNSUCCESSFUL;
	Buffer = ExAllocatePool2(PoolType, RegStruct->DataSize, 'buf');
	if (!Buffer) {
		return Status;
	}

	RtlCopyMemory(Buffer, RegStruct->Data, RegStruct->DataSize);
	Status = BinaryDataToUnicodeSTR(Buffer, Data, RegStruct->DataSize);
	ExFreePoolWithTag(Buffer, 'buf');
    if(!NT_SUCCESS(Status)) {
        ExFreePoolWithTag(Data->Buffer, 'wstr');
    }

    return Status;
}

/*
 *  GetRegWORDData() -
 *
 *  @PoolType:
 *  @Data:
 *  @RegStruct:
 *
 *  Return:
 *    -
 *    -
 */
static NTSTATUS GetRegWORDData(_PoolType_ POOL_TYPE PoolType, _Inout_ PUNICODE_STRING Data, _In_ PREG_SET_VALUE_KEY_INFORMATION RegStruct) {

    NTSTATUS Status;
    PVOID Buffer;

    Status = STATUS_UNSUCCESSFUL;
	Buffer = ExAllocatePool2(PoolType, RegStruct->DataSize, 'buf');
	if (!Buffer) {
		return Status;
	}

	RtlCopyMemory(Buffer, RegStruct->Data, RegStruct->DataSize);
 	Status = WORDDataToUnicodeSTR(Buffer, Data, RegStruct->DataSize);
	ExFreePoolWithTag(Buffer, 'buf');
    if(!NT_SUCCESS(Status)) {
        ExFreePoolWithTag(Data->Buffer, 'wstr');
    }

    return Status;
}

/*
 *  GetKeyData() -
 *
 *  @RegInfo:
 *  @RegStruct:
 *
 *  Return:
 *    -
 *    -
 */
static NTSTATUS GetKeyData(_Inout_ PREG_INFO RegInfo, _In_ PREG_SET_VALUE_KEY_INFORMATION RegStruct) {

    NTSTATUS Status;
	UNICODE_STRING Data;

	Status = InitUnicodeString(&Data);
    if(!NT_SUCCESS(Status)) {
        return Status;
    }

	RegInfo->DataType = RegStruct->Type;
	RegInfo->DataSize = RegStruct->DataSize;

	switch (RegStruct->Type) {
	    case REG_SZ:
	    case REG_EXPAND_SZ:
		    RtlCopyMemory(Data.Buffer, RegStruct->Data, RegStruct->DataSize);
		    break;

	    case REG_MULTI_SZ: 
		    RtlCopyMemory(Data.Buffer, RegStruct->Data, min(Data.Length, RegStruct->DataSize));
		    break;

	    case REG_BINARY:
	    case REG_DWORD :
	    case REG_QWORD :
		    GetRegWORDData(RegInfo->PoolType ,&Data, RegStruct);
		    break;
	}

	UnicodeStrToWSTR(RegInfo->PoolType, &Data, &RegInfo->Data, &RegInfo->DataSize);
	ExFreePoolWithTag(Data.Buffer, 'buf');

	return STATUS_SUCCESS;
}

/*
 *  AcquireCompleteName() -
 *
 *  @RegInfo:
 *  @Object:
 *  @ObjectName:
 *
 *  Return:
 *    -
 *    -
 */
static NTSTATUS AcquireCompleteName(_Inout_ PREG_INFO RegInfo, _In_ PVOID Object) {

	NTSTATUS Status;
	ULONG ObjNameLen;
    ULONG ReturnLen;
	POBJECT_NAME_INFORMATION AuxObjName;

	Status = STATUS_UNSUCCESSFUL;
	if (!Object) {
		return Status;
	}

    ObjNameLen = 0;
    ReturnLen = 0;
    AuxObjName = NULL;

	Status = ObQueryNameString(Object, NULL, 0, &ObjNameLen);
	if (Status == STATUS_INFO_LENGTH_MISMATCH) {
		ObjNameLen += sizeof *AuxObjName;
		AuxObjName = ExAllocatePool2(RegInfo->PoolType, ObjNameLen, 'obj');
		if (!AuxObjName) {
			Status = STATUS_UNSUCCESSFUL;
		} else {
			Status = ObQueryNameString(Object, AuxObjName, ObjNameLen, &ObjNameLen);
			if (!NT_SUCCESS(Status)) {
				ExFreePoolWithTag(AuxObjName, 'obj');
				return STATUS_UNSUCCESSFUL;
			}
		}
	} else {
        if (NT_SUCCESS(Status)) {
            ObjNameLen = sizeof(OBJECT_NAME_INFORMATION);
            AuxObjName = ExAllocatePool2(RegInfo->PoolType, ObjNameLen, 'obj');
            if (AuxObjName != NULL) {
                RtlZeroMemory(AuxObjName, ObjNameLen);
                Status = STATUS_SUCCESS;
            }
        }
    }

	if (AuxObjName) {
		Status = UnicodeStrToWSTR(RegInfo->PoolType, &AuxObjName->Name, &RegInfo->CompleteName, &RegInfo->CompleteNameSize);
		ExFreePoolWithTag(AuxObjName, 'obj');
		if (!NT_SUCCESS(Status)) {
			ExFreePoolWithTag(RegInfo->CompleteName, 'wstr');
			RegInfo->CompleteName = NULL;
			return Status;
		}
	}

	return Status;
}

/*
 *  SetValueInit() -
 *
 *  @RegInfo:
 *  @RegStruct:
 *
 *  Return:
 *    -
 *    -
 */
static NTSTATUS SetValueInit(_Inout_ PREG_INFO RegInfo, _In_ PREG_SET_VALUE_KEY_INFORMATION RegStruct) {

	NTSTATUS Status;

	Status = STATUS_UNSUCCESSFUL;
	if (RegInfo && RegStruct) {
        Status = AcquireCompleteName(RegInfo, RegStruct->Object);
		if (!NT_SUCCESS(Status)) {
			return Status;
		}
		// Callers of RtlCopyMemory can be running at any IRQL if the sour
		Status = GetKeyData(RegInfo, RegStruct);
	}

	return Status;
}

/*
 *  DeleteValueInit() -
 *
 *  @RegInfo:
 *  @RegStruct:
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS DeleteValueInit(_Inout_ PREG_INFO RegInfo, _In_ PREG_DELETE_VALUE_KEY_INFORMATION RegStruct) {

	NTSTATUS Status;

	Status = STATUS_UNSUCCESSFUL;

	if (RegInfo && RegStruct) {
		Status = AcquireCompleteName(RegInfo, RegStruct->Object);
		if (!NT_SUCCESS(Status)) {
			return Status;
		}

		RegInfo->Data = NULL;
		RegInfo->DataSize = 0;
		RegInfo->DataType = 0;
	}

	return Status;
}

/*
 *  RegInfoInit() -
 *
 *  @RegInfo:
 *  @RegInfoData:
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS RegInfoInit(_Inout_ PREG_INFO RegInfo, _In_ PREG_INFO_DATA RegInfoData) {

	NTSTATUS Status;
    REG_NOTIFY_CLASS RegNotifyClass;

    RegNotifyClass = RegInfoData->RegNotifyClass;
	if (RegNotifyClass == RegNtPreSetValueKey) {
		Status = SetValueInit(RegInfo, RegInfoData->RegStruct);
	} else {
		Status = DeleteValueInit(RegInfo, RegInfoData->RegStruct);
	}
	
	return Status;
}

/*
 *  RegInfoCopy() -
 *
 *  @Dest:
 *  @Src:
 */
void RegInfoCopy(_Inout_ PREG_INFO_STATIC Dest, _In_ PREG_INFO Src) {

	ULONG DataSize;

	if (Dest && Src) {

		Dest->DataSize = Src->DataSize;
		Dest->DataType = Src->DataType;
		Dest->RegNotifyClass = Src->RegNotifyClass;
		Dest->CompleteNameSize = Src->CompleteNameSize;

		DataSize = min(MAX_PATH, Src->CompleteNameSize);
		if (DataSize) {
			RtlCopyMemory(Dest->CompleteName, Src->CompleteName, DataSize * sizeof * Src->CompleteName);
		}

		DataSize = min(MAX_DATA, Src->DataSize);
		if (DataSize) {
			RtlCopyMemory(Dest->Data, Src->Data, sizeof * Src->Data * DataSize);
		}
	}
}

/*
 *  RegInfoDeInit() -
 *
 *  @RegInfo:
 */
void RegInfoDeInit(_Inout_ PREG_INFO RegInfo) {

    if (RegInfo) {
        ExFreePoolWithTag(RegInfo->Data, 'wstr');
        ExFreePoolWithTag(RegInfo->CompleteName, 'wstr');
    }
}

/*
 *  RegInfoFree() -
 *
 *  @RegInfo:
 */
void RegInfoFree(_Inout_ PREG_INFO* RegInfo) {


    if(RegInfo && *RegInfo) {
	    RegInfoDeInit(*RegInfo);
        ExFreePoolWithTag(*RegInfo, 'reg');

        /*
         *  In order to avoid a dangling pointer, after deallocating the
         *  'REG_INFO' structure, we set the reference to the 'PROC_INFO'
         *  struct to NULL.
         */
	    *RegInfo = NULL;
    }
}
