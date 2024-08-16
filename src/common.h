#ifndef COMMON_H
#define COMMON_H

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include <wdm.h>
#include <ntstrsafe.h>
#include <ntifs.h>

#define _PoolType_ __drv_strictTypeMatch(__drv_typeExpr)
#define MAX_STR_SIZE  sizeof(WCHAR) * NTSTRSAFE_UNICODE_STRING_MAX_CCH
#define MAX_PATH 260
#define MAX_DATA 2048

#define DbgPrint(fmt, ...)                                                                  \
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, fmt, __VA_ARGS__);


#define DbgPrintSt(fmt, ...)                                                                \
	DbgPrint(fmt##" -- %x\n", __VA_ARGS__)


#define Debug(fmt, ...)			                                                        	\
DbgPrintEx(						                                                        	\
	DPFLTR_IHVDRIVER_ID,			                                                        \
	DPFLTR_ERROR_LEVEL,				                                                        \
	fmt"\n",					                                                        	\
	__VA_ARGS__						                                                        \
)

#define AssertExpr(expr, fmt, ...)							                                \
((expr)													                                	\
	? (void)0											                                	\
	: Debug(												                                \
		"Assertion failed - %s:%s:%d: "#expr" - "fmt,	                                	\
		__FILE__,											                                \
		__func__,										                                	\
		__LINE__,										                                	\
		__VA_ARGS__											                                \
	)														                                \
)

#define Assert(expr, ...) AssertExpr(expr, __VA_ARGS__)

#ifdef LINEDEBUG
#define line() Debug("%s:%d\n", __func__, __LINE__)
#else
#define line() (void)0
#endif

#endif  /* COMMON_H */
