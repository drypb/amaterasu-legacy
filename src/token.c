
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

	Status = SeQueryInformationToken(pToken, TokenType, &TokenInfo->Type);
	if (!NT_SUCCESS(Status)) {
		return Status;
	}

	Status = SeQueryInformationToken(pToken, TokenPrivileges, &TokenInfo->Privileges);
	if (!NT_SUCCESS(Status)) {
		return Status;
	}

	Status = SeQueryInformationToken(pToken, TokenElevation, &TokenInfo->TokenElevation);
	if (!NT_SUCCESS(Status)) {
		return Status;
	}

	if (*TokenInfo->Type == TokenImpersonation) {
		Status = SeQueryInformationToken(pToken, TokenImpersonation, &TokenInfo->ImpersonationLevel);
		if (!NT_SUCCESS(Status)) {
			return Status;
		}
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

	Status = PsLookupProcessByProcessId(PID, &eProc);
	if (!NT_SUCCESS(Status)) {
		return Status;
	}
		
	pToken = PsReferencePrimaryToken(eProc);
	if (pToken) {
	    Status = AcquireTokenInfo(pToken, TokenInfo);
	}

	ObDereferenceObject(eProc);

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

    if(Dest && Src) {

        PrivSize = Src->Privileges->PrivilegeCount * sizeof *Src->Privileges->Privileges;
	    Dest->Privileges.PrivilegeCount = Src->Privileges->PrivilegeCount;

	    RtlCopyMemory(&Dest->Type, Src->Type, sizeof *Src->Type);
	    RtlCopyMemory(&Dest->TokenElevation, Src->TokenElevation, sizeof *Src->TokenElevation);
	    RtlCopyMemory(Dest->Privileges.Privileges, Src->Privileges->Privileges, PrivSize);

        if(*Src->Type == TokenImpersonation) {
            RtlCopyMemory(&Dest->ImpersonationLevel, Src->ImpersonationLevel, sizeof *Src->ImpersonationLevel);
        }
    }
}
