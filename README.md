# Warlord

Warlord is DLL injection tool written in C++ and x64 Assembly. It demonstrates user-mode hook evasion by dynamically retrieving System service numbers (SSNs) 
and executing direct native system calls rather then relying on high level Windows API functions that can easily be hooked.

# Technical details and Overview

Modern Endpoint detection and Response (EDR) solutions and anti-virus products frequently place inline hooks (such as jmp instructions) inside usermode DLLs (e.g. ntdll.dll, kernel32.dll etc.)
to monitor API calls associated with process injection and other malicious activity.

Warlord evades user-mode hooks by executing the following workflow:

1. Dynamic SSN Retrieval: Parses ntdll.dll in memory to extract the System Service Number (SSN) for specific native API functions (NtAllocateVirtualMemory, NtWriteVirtualMemory, and NtCreateThreadEx).
2. Direct Syscall Stubs: Executes custom x64 Assembly stubs (stub.asm) using the retrieved SSNs to perform syscall operations directly from the executable space, bypassing hooked user-mode wrappers in ntdll.dll.
3. Remote Injection:

- Opens a handle to the target process (OpenProcess).

- Allocates memory in the target process using NtAllocateVirtualMemory via direct syscall.

- Writes the DLL payload path into the target process memory using NtWriteVirtualMemory via direct syscall.

- Creates a remote thread targeting kernel32!LoadLibraryW using NtCreateThreadEx via direct syscall to execute the payload.

# Usage:

1.Run Warlord from an elevated command prompt process (also works without elevation if the other process also isnt elevated) and get the PID
2. Warlord.exe <path_to_dll> <pid_of_target_process>

# Disclaimer

This repository and its source code are intended exclusively for educational purposes, defensive security research, and authorized red team testing. The author assumes no liability for misuse, unauthorized access, or damage caused by the use of this software. Always ensure you have explicit written permission before executing injection techniques against any target system.