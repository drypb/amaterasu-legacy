
#include "amaterasu.h"
#include "reginfo.h"


PREG_INFO RegInfoAlloc(_PoolType_ POOL_TYPE PoolType) {

	PREG_INFO RegInfo;

	RegInfo = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof * RegInfo, 'rifo');
	if (!RegInfo) {
		Assert(RegInfo);
		return NULL;
	}

	RtlZeroMemory(RegInfo, sizeof * RegInfo);

	RegInfo->PoolType = POOL_FLAG_NON_PAGED;

	return RegInfo;
}

static inline void GetRegBinaryData(PUNICODE_STRING Data, PREG_SET_VALUE_KEY_INFORMATION RegStruct) {

	PVOID buffer = ExAllocatePoolWithTag(NonPagedPool, RegStruct->DataSize, 'buf');
	if (!buffer) {
		return;
	}

	RtlCopyMemory(buffer, RegStruct->Data, RegStruct->DataSize);

	BinaryDataToUnicodeSTR(buffer, Data, RegStruct->DataSize);

	ExFreePoolWithTag(buffer, 'buf');
}

static inline void GetRegWORDData(PUNICODE_STRING Data, PREG_SET_VALUE_KEY_INFORMATION RegStruct) {
	PVOID buffer = ExAllocatePoolWithTag(NonPagedPool, RegStruct->DataSize, 'buf');
	if (!buffer) {
		return;
	}

	RtlCopyMemory(buffer, RegStruct->Data, RegStruct->DataSize);

 	WORDDataToUnicodeSTR(buffer, Data, RegStruct->DataSize);

	ExFreePoolWithTag(buffer, 'buf');
}

static NTSTATUS GetKeyData(PREG_INFO RegInfo, PREG_SET_VALUE_KEY_INFORMATION RegStruct) {

	UNICODE_STRING Data;
	InitUnicodeString(&Data);

	RegInfo->DataType = RegStruct->Type;
	RegInfo->DataSize = RegStruct->DataSize;
	DbgPrint("Entrou no GetKeyData()\n");
	switch (RegStruct->Type) {
	case REG_SZ:
		RtlCopyMemory(Data.Buffer, RegStruct->Data, RegStruct->DataSize);
		break;
	case REG_EXPAND_SZ:
		RtlCopyMemory(Data.Buffer, RegStruct->Data, RegStruct->DataSize);
		break;
	case REG_MULTI_SZ: {
		ULONG min = min(Data.Length, RegStruct->DataSize);
		RtlCopyMemory(Data.Buffer, RegStruct->Data, min);
		break;
	}
	case REG_BINARY:
		GetRegBinaryData(&Data, RegStruct);
		break;
	case REG_DWORD:
		GetRegWORDData(&Data, RegStruct);
		break;
	case REG_QWORD:
		GetRegWORDData(&Data, RegStruct);
		break;

	}

	UnicodeStrToWSTR(RegInfo->PoolType, &Data, &RegInfo->Data, &RegInfo->DataSize);

	ExFreePoolWithTag(Data.Buffer, 'buf');

	DbgPrint("Saiu do GetKeyData()\n\n");
	return STATUS_SUCCESS;
}

static NTSTATUS AcquireCompleteName(PREG_INFO RegInfo, PVOID Object, POBJECT_NAME_INFORMATION* ObjectName) {
	DbgPrint("entrou em %s\n", __func__);
	NTSTATUS Status;
	ULONG ObjNameLen = 0, ReturnLen = 0;
	POBJECT_NAME_INFORMATION auxObjName = NULL;

	Assert(RegInfo);
	Assert(Object);

	Status = STATUS_UNSUCCESSFUL;
	if (!RegInfo || !Object) {
		return Status;
	}


	KIRQL irql = KeGetCurrentIrql();

	*ObjectName = NULL;


	Status = ObQueryNameString(Object, NULL, 0, &ObjNameLen);
	if (Status = STATUS_INFO_LENGTH_MISMATCH) {
		ObjNameLen += sizeof(OBJECT_NAME_INFORMATION);
		auxObjName = ExAllocatePool2(POOL_FLAG_NON_PAGED, ObjNameLen, 'obj');
		if (!auxObjName) {
			Status = STATUS_UNSUCCESSFUL;
		}
		else {
			Status = ObQueryNameString(Object, auxObjName, ObjNameLen, &ObjNameLen);
			if (!NT_SUCCESS(Status)) {
				ExFreePoolWithTag(auxObjName, 'obj');
				return STATUS_UNSUCCESSFUL;
			}
			*ObjectName = auxObjName;
		}
	}
	else if (NT_SUCCESS(Status)) {
		ObjNameLen = sizeof(OBJECT_NAME_INFORMATION);
		auxObjName = (POBJECT_NAME_INFORMATION)ExAllocatePool2(POOL_FLAG_NON_PAGED, ObjNameLen, 'obj');
		if (auxObjName != NULL) {
			RtlZeroMemory(auxObjName, ObjNameLen);
			*ObjectName = auxObjName;
			Status = STATUS_SUCCESS;
		}
	}


	//DbgPrint("%s:Antes CmCallbackGetKeyObjectID()\n", __func__);
	////try
	////{
	//	DbgPrint("ObQueryNameString para pegar o tamanho\n")
	//	//Status = ObQueryNameString(*Object, NULL, 0, &ReturnLen1);
	//	DbgPrint("ObQueryNameString para pegar o tamanho dps\n")

	//DbgPrint("Return len 1 %lu\n", ReturnLen1);

	//	//Status = CmCallbackGetKeyObjectIDEx(&Amaterasu.Cookie, Object, NULL, Out,0);
	//	DbgPrint("ObQueryNameString \n")
	//	Status = ObQueryNameString(Object, (PVOID)Out,Out->MaximumLength,&ReturnLen);
	//	DbgPrint("ObQueryNameString dps \n")
	///*}
	//except(REGISTRY_FILTER_DRIVER_EXCEPTION) {
	//	DbgPrint("REGISTRY_FILTER_DRIVER_EXCEPTION %x\n", Status);
	//	DbgPrint("reg path : %ws\n", (*Out)->Buffer);
	//};*/

	//DbgPrint("reg path : %ws\n", Out->Buffer);
	//DbgPrint("Return len %lu\n", ReturnLen);

	//DbgPrint("%s:Depois CmCallbackGetKeyObjectID()\n", __func__);
	//if (!NT_SUCCESS(Status)) {
	//	Assert(NT_SUCCESS(Status), "CmCallbackGetKeyObjectID()");
	//	return Status;
	//}
	//else {
	//	DbgPrint("deu boa o reg path : %ws\n", Out->Buffer);

	//}

	return Status;
}

NTSTATUS SetValueInit(
	PREG_INFO RegInfo,
	PREG_SET_VALUE_KEY_INFORMATION RegStruct
) {

	NTSTATUS Status;
	UNICODE_STRING QueryStr;
	InitUnicodeString(&QueryStr);

	POBJECT_NAME_INFORMATION ObjectName;

	Status = STATUS_UNSUCCESSFUL;
	if (RegInfo && RegStruct) {
		if (!RegStruct->ValueName) {
			return STATUS_UNSUCCESSFUL;
		}

		Status = AcquireCompleteName(RegInfo, RegStruct->Object, &ObjectName);
		if (!NT_SUCCESS(Status)) {
			return Status;
		}

		Status = UnicodeStrToWSTR(RegInfo->PoolType, &ObjectName->Name, &RegInfo->CompleteName, &RegInfo->CompleteNameSize);
		ExFreePoolWithTag(ObjectName, 'obj');
		if (!NT_SUCCESS(Status)) {
			ExFreePoolWithTag(RegInfo->CompleteName, 'wstr');
			RegInfo->CompleteName = NULL;
			return Status;
		}

		GetKeyData(RegInfo, RegStruct);
	}

	return Status;
}

NTSTATUS DeleteValueInit(
	PREG_INFO RegInfo,
	PREG_DELETE_VALUE_KEY_INFORMATION RegStruct
) {

	NTSTATUS Status;

	Status = STATUS_UNSUCCESSFUL;

	if (RegInfo && RegStruct) {

		Status = AcquireCompleteName(RegInfo, RegStruct->Object, NULL);
		if (!NT_SUCCESS(Status)) {
			return Status;
		}
		RegInfo->Data = NULL;
		RegInfo->DataSize = 0;
		RegInfo->DataType = 0;
	}


	return Status;
}

NTSTATUS RegInfoInit(PREG_INFO RegInfo, REG_NOTIFY_CLASS RegOperation, PVOID RegStruct) {

	NTSTATUS Status;

    DbgPrint("%s:%d: &RegStruct = %p\n", __func__, __LINE__, RegStruct);

	Status = STATUS_UNSUCCESSFUL;
	if (RegOperation == RegNtPreSetValueKey) {
		Status = SetValueInit(RegInfo, RegStruct);
	}
	else {
		Status = DeleteValueInit(RegInfo, RegStruct);
	}
	
	DbgPrint("Saiu do RegInfoInit()\n");
	return Status;
}

void RegInfoFree(PREG_INFO* RegInfo) {

	if (!RegInfo || *RegInfo == NULL)
		return;


	ExFreePoolWithTag(*RegInfo, 'reg');

	*RegInfo = NULL;
}

PREG_INFO RegInfoGet(_PoolType_ POOL_TYPE PoolType, PREG_INFO_DATA RegInfoData) {

	PREG_INFO RegInfo;
	NTSTATUS Status = STATUS_SUCCESS;

	PREG_SET_VALUE_KEY_INFORMATION RegStruct = RegInfoData->RegStruct;

	RegInfo = RegInfoAlloc(PoolType);
	if (!RegInfo) {
		return NULL;
	}

	Status = RegInfoInit(RegInfo, RegInfoData->RegNotifyClass, RegInfoData->RegStruct);
	if (!NT_SUCCESS(Status)) {
		line();
		RegInfoFree(&RegInfo);
		return NULL;
	}

	return RegInfo;
}

void RegInfoCopy(PREG_INFO_STATIC StaticRegInfo, PREG_INFO RegInfo) {

	ULONG DataSize;


	if (StaticRegInfo && RegInfo) {

		StaticRegInfo->RegNotifyClass = RegInfo->RegNotifyClass;
		StaticRegInfo->CompleteNameSize = RegInfo->CompleteNameSize;
		StaticRegInfo->DataSize = RegInfo->DataSize;
		StaticRegInfo->DataType = RegInfo->DataType;


		DataSize = min(MAX_PATH, RegInfo->CompleteNameSize);
		if (DataSize) {
			RtlCopyMemory(StaticRegInfo->CompleteName, RegInfo->CompleteName, DataSize * sizeof * RegInfo->CompleteName);
		}

		DataSize = min(MAX_DATA, RegInfo->DataSize);
		if (DataSize) {
			RtlCopyMemory(StaticRegInfo->Data, RegInfo->Data, sizeof * RegInfo->Data * DataSize);
		}
	}
}


