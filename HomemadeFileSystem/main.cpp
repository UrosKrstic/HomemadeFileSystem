#include <windows.h>
#include <iostream>
#include "fs.h"
#include "part.h"

#define signal(x) ReleaseSemaphore(x,1,NULL)
#define wait(x) WaitForSingleObject(x,INFINITE)

HANDLE nit1, nit2;
DWORD ThreadID;

DWORD WINAPI run1() {
	std::cout << "nit1 pocela sa radom, id = " << GetCurrentThreadId() << "\n";
	FS::mount(nullptr);
	std::cout << "nit1 uspesno izvrsila mount\n";
	Sleep(5000);
	FS::unmount();
	std::cout << "nit1 uspesno izvrsila unmount\n";
	return 0;
}

DWORD WINAPI run2() {
	std::cout << "nit2 pocela sa radom, id = " << GetCurrentThreadId() << "\n";
	FS::mount(nullptr);
	std::cout << "nit2 uspesno izvrsila mount\n";
	Sleep(5000);
	FS::unmount();
	std::cout << "nit2 uspesno izvrsila unmount\n";
	return 0;
}


int main(int argc, char* argv[]) {
	/*nit1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)run1, NULL, 0, &ThreadID);
	nit2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)run2, NULL, 0, &ThreadID);
	std::cout << "napravljene niti\n";
	HANDLE lpHandles[2] = { nit1, nit2 };
	WaitForMultipleObjects(2, lpHandles, TRUE, INFINITE);
	CloseHandle(nit1);
	CloseHandle(nit2);*/
	auto*partition = new Partition(const_cast<char *>("p1.ini"));

	std::cout << "Poceo sa mount\n";
	FS::mount(partition);

	Sleep(5000);
	
	std::cout << "Poceo sa unmount\n";
	FS::unmount();
	

	delete partition;
	
}
