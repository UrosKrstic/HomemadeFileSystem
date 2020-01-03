#pragma warning(disable : 4996)
#include"testprimer.h"

using namespace std;

HANDLE nit1, nit2;
DWORD ThreadID;

HANDLE semMain = CreateSemaphore(NULL, 0, 32, NULL);
HANDLE sem12 = CreateSemaphore(NULL, 0, 32, NULL);
HANDLE sem21 = CreateSemaphore(NULL, 0, 32, NULL);
HANDLE mutex = CreateSemaphore(NULL, 1, 32, NULL);

Partition *partition;

char *ulazBuffer;
int ulazSize;

int main() {
	clock_t startTime, endTime;
	cout << "Pocetak testa!" << endl;
	startTime = clock();//pocni merenje vremena

	{//ucitavamo ulazni fajl u bafer, da bi nit 1 i 2 mogle paralelno da citaju
		FILE *f = fopen("ulaz.dat", "rb");
		if (f == 0) {
			cout << "GRESKA: Nije nadjen ulazni fajl 'ulaz.dat' u os domacinu!" << endl;
			system("PAUSE");
			return 0;//exit program
		}
		ulazBuffer = new char[32 * 1024 * 1024];//32MB
		ulazSize = fread(ulazBuffer, 1, 32 * 1024 * 1024, f);
		fclose(f);
	}

	nit1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)nit1run, NULL, 0, &ThreadID); //kreira i startuje niti
	nit2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)nit2run, NULL, 0, &ThreadID);

	for (int i = 0; i < 2; i++) wait(semMain);//cekamo da se niti zavrse
	delete[] ulazBuffer;
	endTime = clock();
	cout << "Kraj test primera!" << endl;
	cout << "Vreme izvrsavanja: " << ((double)(endTime - startTime) / ((double)CLOCKS_PER_SEC / 1000.0)) << "ms!" << endl;
	CloseHandle(mutex);
	CloseHandle(semMain);
	CloseHandle(sem12);
	CloseHandle(sem21);
	CloseHandle(nit1);
	CloseHandle(nit2);
	return 0;
}

//#include <windows.h>
//#include <iostream>
//#include "fs.h"
//#include "part.h"
//#include "file.h"
//
//using namespace std;
//
//char * str_creator(int size, char fill) {
//	char * buffer = new char[size];
//	for (int i = 0; i < size; i++) buffer[i] = fill;
//	return buffer;
//}
//
//void func1() {
//	char * fpath = const_cast<char*>("/fajl1.txt");
//
//	File* file = FS::open(fpath, 'a');
//
//	char * buffer = str_creator(4000, 'c');
//	file->write(4000, buffer);
//
//	file->seek(2100);
//
//	delete[] buffer;
//	buffer = str_creator(20, 'd');
//	file->write(20, buffer);
//
//	delete file;
//
//	file = FS::open(fpath, 'r');
//
//	delete[] buffer;
//	buffer = str_creator(10, 'b');
//
//	file->read(10, buffer);
//
//	cout << buffer << endl;
//
//	delete[] buffer;
//}
//
//void func2() {
//	char * fpath = const_cast<char*>("/funk.txt");
//	File* file = FS::open(fpath, 'w');
//	int size = 514 * 2048;
//	char* buffer = str_creator(size, 'f');
//	file->write(size, buffer);
//	delete[] buffer;
//	delete file;
//
//	buffer = str_creator(size, 'g');
//	file = FS::open(fpath, 'r');
//	file->read(size, buffer);
//	delete file;
//	delete[] buffer;
//}
//
//void func3() {
//	char * fpath = const_cast<char*>("/fajl1.txt");
//	File* file = FS::open(fpath, 'a');
//
//	file->seek(file->getFileSize() - 5000);
//	file->truncate();
//
//
//	delete file;
//}
//
//void func4() {
//	FS::deleteFile(const_cast<char*>("/fajl1.txt"));
//	if (FS::doesExist(const_cast<char*>("/fajl1.txt"))) cout << "NO" << endl;
//}
//
//void func5() {
//	char * fpath = const_cast<char*>("/funk.txt");
//	File* file = FS::open(fpath, 'w');
//	int size = 516 * 2048;
//	char* buffer = str_creator(size, 'f');
//	file->write(size, buffer);
//	delete[] buffer;
//	file->seek(0);
//	buffer = str_creator(size, 'g');
//	file->write(size / 2, buffer);
//	delete[] buffer;
//	file->seek(file->getFileSize());
//	buffer = str_creator(2000, 'h');
//	file->write(2000, buffer);
//	delete file;
//	delete[] buffer;
//}
//
//
//int main(int argc, char* argv[]) {
//	auto * partition = new Partition(const_cast<char *>("p1.ini"));
//
//	FS::mount(partition);
//	FS::format();
//	func2();
//
//	FS::unmount();
//
//	delete partition;
//}