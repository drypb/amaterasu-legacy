
#include "token.h"

/*
 *  AcquireTokenInfo() - Acquires various token information.
 *  
 *  @TokenHandle: Handle to the token.
 *  @TokenInfo: Pointer to a TOKEN_INFO structure to receive the token information.
 *
 *  This function queries the specified token for its type, privileges, elevation,
 *  and impersonation level, and stores this information in the provided TOKEN_INFO
 *  structure. It returns an NTSTATUS code indicating success or failure.
 *
 *  Return: 
 *    -
 *    -
 */
static NTSTATUS AcquireTokenInfo(_In_ PACCESS_TOKEN pToken, _Inout_ PTOKEN_INFO TokenInfo) {

	NTSTATUS Status;

	line();
	__try {
		Status = SeQueryInformationToken(pToken, TokenType, &TokenInfo->Type);
		line();
		if (!NT_SUCCESS(Status)) {
			line();
			return Status;
		}
	} except(EXCEPTION_CONTINUE_EXECUTION) {
		line();
		return STATUS_UNSUCCESSFUL;
	}

	__try {
		line();
		Status = SeQueryInformationToken(pToken, TokenPrivileges, &TokenInfo->Privileges);
		line();
		if (!NT_SUCCESS(Status)) {
			line();
			return Status;
		}
	} except(EXCEPTION_CONTINUE_EXECUTION) {
		//line();
		//ExFreePool(TokenInfo->Type);
		line();
		return STATUS_UNSUCCESSFUL;
	}

	__try {
		line();
		Status = SeQueryInformationToken(pToken, TokenElevation, &TokenInfo->TokenElevation);
		line();
		if (!NT_SUCCESS(Status)) {
			line();
			return Status;
		}
	} except(EXCEPTION_CONTINUE_EXECUTION) {
		//line();
		//ExFreePool(TokenInfo->Type);
		//line();
		//ExFreePool(TokenInfo->TokenElevation);
		//line();
		return STATUS_UNSUCCESSFUL;
	}

	__try {
		line();
		if (*TokenInfo->Type == TokenImpersonation) {
			line();
			Status = SeQueryInformationToken(pToken, TokenImpersonation, &TokenInfo->ImpersonationLevel);
			line();
			if (!NT_SUCCESS(Status)) {
				line();
				return Status;
			}
		}
		line();
	} except(EXCEPTION_CONTINUE_EXECUTION) {
		//line();
		//ExFreePool(TokenInfo->Type);
		//line();
		//ExFreePool(TokenInfo->TokenElevation);
		//line();
		//ExFreePool(TokenInfo->ImpersonationLevel);
		line();
		return STATUS_UNSUCCESSFUL;
	}

	return Status;
}

/*
 *  TokenInfoGet() - Retrieves token information for a specified process.
 *
 *  @TokenInfo: Pointer to a TOKEN_INFO structure to receive the token information.
 *  @PID: Handle to the process.
 *
 *  This function opens the process token for the specified process and retrieves
 *  various token information by calling AcquireTokenInfo. It returns an NTSTATUS
 *  code indicating success or failure.
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS TokenInfoGet(_Inout_ PTOKEN_INFO TokenInfo, _In_ HANDLE PID) {

	NTSTATUS Status;
	PACCESS_TOKEN pToken;
	PEPROCESS eProc;

	line();
	Status = PsLookupProcessByProcessId(PID, &eProc);
	line();
	if (!NT_SUCCESS(Status)) {
		line();
		return Status;
	}
		
	line();
	pToken = PsReferencePrimaryToken(eProc);
	line();
	if (pToken) {
		line();
	    Status = AcquireTokenInfo(pToken, TokenInfo);
		line();
	}

	line();
	ObDereferenceObject(eProc);
	line();

	return Status;
}

/*
 *  TokenInfoCopy() -
 *
 *  @Dest:
 *  @Src:
 */
void TokenInfoCopy(_Out_ PTOKEN_INFO_STATIC Dest, _In_ PTOKEN_INFO Src) {

    ULONG PrivSize;

	__try {
		line();
		if (Dest && Src) {

			line();
			PrivSize = Src->Privileges->PrivilegeCount * sizeof * Src->Privileges->Privileges;
			line();
			Dest->Privileges.PrivilegeCount = Src->Privileges->PrivilegeCount;

			line();
			RtlCopyMemory(&Dest->Type, Src->Type, sizeof * Src->Type);
			line();
			RtlCopyMemory(&Dest->TokenElevation, Src->TokenElevation, sizeof * Src->TokenElevation);
			line();
			RtlCopyMemory(Dest->Privileges.Privileges, Src->Privileges->Privileges, PrivSize);
			line();

			if (*Src->Type == TokenImpersonation) {
				line();
				RtlCopyMemory(&Dest->ImpersonationLevel, Src->ImpersonationLevel, sizeof * Src->ImpersonationLevel);
				line();
			}
		}
	} except(EXCEPTION_CONTINUE_EXECUTION) {
		line();
		return;
	}
	line();
}
