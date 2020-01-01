#include <windows.h>
#include <iostream>
#include "fs.h"
#include "part.h"
#include "file.h"
using namespace std;


HANDLE nit1, nit2, nit3;
DWORD ThreadID;

auto * partition = new Partition(const_cast<char *>("p1.ini"));

char * str_creator(int size, char fill) {
	char * buffer = new char[size];
	for (int i = 0; i < size; i++) buffer[i] = fill;
	return buffer;
}

DWORD WINAPI reader() {


	char * buffer = new char[5];
	unsigned cnt = 0;
	File * file = nullptr;
	do {
		file = FS::open(const_cast<char*>("/fajl.txt"), 'r');
		if (file == nullptr) Sleep(2000);
	} while (file == nullptr);
	file->read(5, buffer);
	cout << "Citalac procitao:\n";
	cout << buffer << endl;
	delete[] buffer;
	delete file;
	Sleep(5000);
	return 0;
}

DWORD WINAPI writer() {
	File * file = FS::open(const_cast<char*>("/fajl.txt"), 'w');

	char * buffer = str_creator(5, 'a');

	file->write(5, buffer);
	cout << "Pisac upisao podatke\n";
	delete[] buffer;
	Sleep(10000);
	delete file;
	return 0;
}


int main(int argc, char* argv[]) {
	FS::mount(partition);
	nit1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)reader, NULL, 0, &ThreadID);
	nit3 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)reader, NULL, 0, &ThreadID);
	nit2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)writer, NULL, 0, &ThreadID);
	HANDLE lpHandles[3] = { nit1, nit2, nit3 };
	WaitForMultipleObjects(3, lpHandles, TRUE, INFINITE);
	CloseHandle(nit1);
	CloseHandle(nit2);
	FS::unmount();
	delete partition;
}