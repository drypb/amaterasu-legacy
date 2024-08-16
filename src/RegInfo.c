
#include "RegInfo.h"

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


