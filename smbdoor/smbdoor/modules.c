#include <ntddk.h>
#include <ntimage.h>

#include "smbdoor.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SmbDoorGetRoutineByName )
#pragma alloc_text( PAGE, SmbDoorGetModuleByName )
#endif


NTSTATUS SmbDoorGetRoutineByName(
	_In_ PVOID DriverImageBase,
	_In_ LPCSTR FunctionName,
	_Out_ PVOID *RoutineAddress
)
/*++

Routine Description:

This routine will find a function export of a loaded module.

Arguments:

DriverImageBase - a pointer to the module base address.

FunctionName - a string containing the export function name.

RoutineAddress - Supplies the address of a variable that will receive
the address of the exported routine.

Return Value:

STATUS_SUCCESS if found; an error otherwise.

--*/
{
	ULONG dirSize;
	PIMAGE_EXPORT_DIRECTORY pExportDir;
	PULONG names;
	PUSHORT ordinals;
	PULONG functions;
	UNICODE_STRING RtlImageString = { 0 };

	PAGED_CODE();

	*RoutineAddress = NULL;

	RtlInitUnicodeString(&RtlImageString, L"RtlImageDirectoryEntryToData");
	RTLIMAGEDIRECTORYENTRYTODATA FnRtlImageDirectoryEntryToData = MmGetSystemRoutineAddress(&RtlImageString);

	pExportDir = (PIMAGE_EXPORT_DIRECTORY)FnRtlImageDirectoryEntryToData(DriverImageBase, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &dirSize);
	names = (PULONG)SMBDOOR_CONVERT_RVA(DriverImageBase, pExportDir->AddressOfNames);
	ordinals = (PUSHORT)SMBDOOR_CONVERT_RVA(DriverImageBase, pExportDir->AddressOfNameOrdinals);
	functions = (PULONG)SMBDOOR_CONVERT_RVA(DriverImageBase, pExportDir->AddressOfFunctions);

	for (ULONG i = 0; i < pExportDir->NumberOfNames; ++i)
	{
		LPCSTR name = (LPCSTR)SMBDOOR_CONVERT_RVA(DriverImageBase, names[i]);
		if (0 == strcmp(FunctionName, name))
		{
			*RoutineAddress = SMBDOOR_CONVERT_RVA(DriverImageBase, functions[ordinals[i]]);
			return STATUS_SUCCESS;
		}
	}

	return STATUS_NOT_FOUND;
}

// Not an accurate stricmp! Works fine for our needs
inline BOOLEAN xstricmp(LPCSTR s1, LPCSTR s2)
{
	for (ULONG i = 0; 0 == ((s1[i] ^ s2[i]) & 0xDF); ++i)
	{
		if (0 == s1[i])
		{
			return TRUE;
		}
	}

	return FALSE;
}

NTSTATUS SmbDoorGetModuleByName(
	_In_ LPCSTR driverName,
	_Out_ PVOID *ImageBase
)
{
	NTSTATUS status;
	ULONG size = 0;
	UNICODE_STRING RtlQueryString = { 0 };

	PAGED_CODE();

	*ImageBase = NULL;

	RtlInitUnicodeString(&RtlQueryString, L"RtlQueryModuleInformation");
	RTLQUERYMODULEINFORMATION FnRtlQueryModuleInformation = MmGetSystemRoutineAddress(&RtlQueryString);

	if (NULL == FnRtlQueryModuleInformation)
	{
		return STATUS_NOT_IMPLEMENTED;
	}

	status = FnRtlQueryModuleInformation(&size, sizeof(RTL_MODULE_EXTENDED_INFO), NULL);

	if (NT_SUCCESS(status))
	{
		PRTL_MODULE_EXTENDED_INFO pDrivers = (PRTL_MODULE_EXTENDED_INFO)ExAllocatePoolWithTag(PagedPool, size, SMBDOOR_POOL_TAG);

		if (NULL == pDrivers)
		{
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		status = FnRtlQueryModuleInformation(&size, sizeof(RTL_MODULE_EXTENDED_INFO), pDrivers);

		if (NT_SUCCESS(status))
		{
			ULONG i;
			status = STATUS_NOT_FOUND;

			for (i = 0; i < size / sizeof(RTL_MODULE_EXTENDED_INFO); ++i)
			{
				if (xstricmp(driverName, &pDrivers[i].FullPathName[pDrivers[i].FileNameOffset]))
				{
					*ImageBase = pDrivers[i].ImageBase;
					status = STATUS_SUCCESS;
					break;
				}
			}

		}

		ExFreePoolWithTag(pDrivers, SMBDOOR_POOL_TAG);
	}

	return status;
}