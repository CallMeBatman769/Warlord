.data
extern NtAllocateVirtualMemorySSN:DWORD
extern NtWriteVirtualMemorySSN:DWORD
extern NtCreateThreadExSSN:DWORD

.code
NtAllocateVirtualMemory proc
		mov r10, rcx
		mov eax, NtAllocateVirtualMemorySSN
		syscall
		ret
NtAllocateVirtualMemory endp

NtWriteVirtualMemory proc
		mov r10, rcx
		mov eax, NtWriteVirtualMemorySSN
		syscall
		ret
NtWriteVirtualMemory endp

NtCreateThreadEx proc
		mov r10, rcx
		mov eax, NtCreateThreadExSSN
		syscall
		ret
NtCreateThreadEx endp
end