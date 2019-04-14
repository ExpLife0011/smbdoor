#include <ntddk.h>
#include <intrin.h>

#include "smbdoor.h"

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD SmbDoorUnload;

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGE, SmbDoorUnload )
#endif

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	DriverObject->DriverUnload = &SmbDoorUnload;

	return SmbDoorRegisterSrvNet();
}

VOID SmbDoorUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);

	SmbDoorDeregisterSrvNet();
	return;
}