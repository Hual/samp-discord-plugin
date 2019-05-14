#include <Windows.h>
#include <iostream>
#include <memory>

// Maximum processes to spawn
constexpr int numProcesses = MAXIMUM_WAIT_OBJECTS;
// Enough sleep time for Discord to update the status
constexpr unsigned long sleepTime = 5000;

// This test was made specifically to catch the "exit on DLL unload" issue
// It works by forking the process n times and each process loading the ASI then sleeping and exiting
int main(int argc, const char* argv[])
{
	if (argc == 1) {
		STARTUPINFO si{sizeof(si)};
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		auto processes = std::make_unique<HANDLE[]>(numProcesses);
		for (size_t i = 0; i < numProcesses; ++i) {
			PROCESS_INFORMATION pi{};
			CHAR args[] = "-d -n test";
			if (!CreateProcess(argv[0], args, nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi)) {
				std::cerr << "Could not fork process" << std::endl;
				return 1;
			}
			processes.get()[i] = pi.hProcess;
		}
		if (WaitForMultipleObjects(numProcesses, processes.get(), true, sleepTime+1000) != WAIT_TIMEOUT) {
			std::cout << "Test passed" << std::endl;
			return 0;
		}
		else {
			std::cerr << "Test failed" << std::endl;
			for (size_t i = 0; i < numProcesses; ++i) {
				TerminateProcess(processes.get()[i], 0);
			}
			return 1;
		}
	}
	else {
		HINSTANCE lib = LoadLibrary("samp-discord-plugin.asi");
		Sleep(sleepTime);
		return 0;
	}
}
