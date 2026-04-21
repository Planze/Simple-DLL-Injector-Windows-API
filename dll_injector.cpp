#include <iostream>
#include <string>
#include <Windows.h>
#include <TlHelp32.h>

/*
Simple DLL injector I made for modding purposes. 

If the game has anticheat use only in offline mode. Otherwise you'll get banned.
*/

bool DllInjection(const std::string& dllPath,const std::string& processName) {

	
	int pathSize = dllPath.size() + 1;

	PROCESSENTRY32 searchProcess;
	searchProcess.dwSize = sizeof(PROCESSENTRY32);
	
	HANDLE hProcessesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (!Process32First(hProcessesSnapshot, &searchProcess)) {
		std::cerr << "Failed to create process snapshot!";
		CloseHandle(hProcessesSnapshot);
		return false;
	}

	DWORD processId = 0;

	while (true) {
		if (std::string(searchProcess.szExeFile) == processName) {
			processId = searchProcess.th32ProcessID;
			break;
		}
		else {
			if (!Process32Next(hProcessesSnapshot, &searchProcess)) {
				std::cerr << "Failed to get first process";
				CloseHandle(hProcessesSnapshot);
				return false;
			}
		}
	}
	CloseHandle(hProcessesSnapshot);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, processId);
	if (!hProcess) {
		std::cerr << "Failed to open process";
		return false;
	}


	LPVOID dllPathMemory = VirtualAllocEx(hProcess, nullptr, pathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!dllPathMemory) {
		std::cerr << "Failed to alloc memory";
		return false;
	}


	if(!WriteProcessMemory(hProcess, dllPathMemory, dllPath.c_str(), pathSize, 0)){
		std::cerr << "Failed to write to memory";
		VirtualFreeEx(hProcess, dllPathMemory, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}


	HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
	if (!hKernel32) {
		std::cerr << "Failed to get handle to hKernel32";
		VirtualFreeEx(hProcess, dllPathMemory, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;

	}

	LPTHREAD_START_ROUTINE pLoadLibraryA = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryA");

	HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, pLoadLibraryA, dllPathMemory, 0, 0);
	if (!hThread) {
		std::cerr << "Failed to create thread";
		VirtualFreeEx(hProcess, dllPathMemory, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}

	WaitForSingleObject(hThread, INFINITE);
	VirtualFreeEx(hProcess, dllPathMemory, 0, MEM_RELEASE);

	CloseHandle(hProcess);
	
	CloseHandle(hThread);

	std::cout << "DLL INJECTED SUCCESFULLY";
	return true;

}


int main() {

	// Example usage

	std::string dllPath = "Epic.dll";
	std::string executable = "windows.exe";

	DllInjection(executable, dllPath);

	return 0;
}