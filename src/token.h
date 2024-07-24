#ifndef TOKEN_INFO_H
#define TOKEN_INFO_H

#include "common.h"

#define MAX_PRIV_COUNT 35

struct Privileges {
	DWORD PrivilegeCount;
	LUID_AND_ATTRIBUTES Privileges[MAX_PRIV_COUNT];
};

typedef struct Privileges PRIVILEGES, * PPRIVILEGES;

struct TokenInfo {
	PTOKEN_TYPE Type;
	PTOKEN_PRIVILEGES Privileges;
	PTOKEN_ELEVATION TokenElevation;
	PSECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
};

typedef struct TokenInfo TOKEN_INFO, * PTOKEN_INFO;

struct TokenInfoStatic {
	TOKEN_TYPE Type;
	PRIVILEGES Privileges;
	TOKEN_ELEVATION TokenElevation;
	SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
};

typedef struct TokenInfoStatic TOKEN_INFO_STATIC, * PTOKEN_INFO_STATIC;

extern NTSTATUS
TokenInfoGet(
	_Inout_ PTOKEN_INFO TokenInfo,
	_In_ HANDLE PID
);

extern void 
TokenInfoCopy(
	_Out_ PTOKEN_INFO_STATIC Dest,
	_In_ PTOKEN_INFO Src
);

#endif  /* TOKEN_INFO_H */
