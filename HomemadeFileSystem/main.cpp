//#pragma warning(disable : 4996)
//#include"testprimer.h"
//
//using namespace std;
//
//HANDLE nit1, nit2;
//DWORD ThreadID;
//
//HANDLE semMain = CreateSemaphore(NULL, 0, 32, NULL);
//HANDLE sem12 = CreateSemaphore(NULL, 0, 32, NULL);
//HANDLE sem21 = CreateSemaphore(NULL, 0, 32, NULL);
//HANDLE mutex = CreateSemaphore(NULL, 1, 32, NULL);
//
//Partition *partition;
//
//char *ulazBuffer;
//int ulazSize;
//
//int main() {
//	clock_t startTime, endTime;
//	cout << "Pocetak testa!" << endl;
//	startTime = clock();//pocni merenje vremena
//
//	{//ucitavamo ulazni fajl u bafer, da bi nit 1 i 2 mogle paralelno da citaju
//		FILE *f = fopen("ulaz.dat", "rb");
//		if (f == 0) {
//			cout << "GRESKA: Nije nadjen ulazni fajl 'ulaz.dat' u os domacinu!" << endl;
//			system("PAUSE");
//			return 0;//exit program
//		}
//		ulazBuffer = new char[32 * 1024 * 1024];//32MB
//		ulazSize = fread(ulazBuffer, 1, 32 * 1024 * 1024, f);
//		fclose(f);
//	}
//
//	nit1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)nit1run, NULL, 0, &ThreadID); //kreira i startuje niti
//	nit2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)nit2run, NULL, 0, &ThreadID);
//
//	for (int i = 0; i < 2; i++) wait(semMain);//cekamo da se niti zavrse
//	delete[] ulazBuffer;
//	endTime = clock();
//	cout << "Kraj test primera!" << endl;
//	cout << "Vreme izvrsavanja: " << ((double)(endTime - startTime) / ((double)CLOCKS_PER_SEC / 1000.0)) << "ms!" << endl;
//	CloseHandle(mutex);
//	CloseHandle(semMain);
//	CloseHandle(sem12);
//	CloseHandle(sem21);
//	CloseHandle(nit1);
//	CloseHandle(nit2);
//	return 0;
//}

#define _CRT_SECURE_NO_WARNINGS

#include "file.h"
#include <Windows.h>
#include <iostream>
#include <queue>
#include <list>
#include <cstdlib>
#include "fs.h"
#include <ctime>
#include "part.h"
#include <string>

#define MAX_SIZE 10
#define signalS(x) ReleaseSemaphore(x,1,NULL)
#define waitS(x) WaitForSingleObject(x,INFINITE)

using namespace std;

HANDLE semMain = CreateSemaphore(NULL, 0, 10, NULL);
HANDLE semS = CreateSemaphore(NULL, 1, 10, NULL);
HANDLE sem12 = CreateSemaphore(NULL, 0, 10, NULL);
HANDLE sem13 = CreateSemaphore(NULL, 0, 10, NULL);
HANDLE semaphor = CreateSemaphore(NULL, 0, 10, NULL);


CRITICAL_SECTION mut;
Partition *p = nullptr;


volatile int counter = 0;

DWORD WINAPI basic_operation_tester()
{
	cout << "Basic test started!" << endl;
	p = new Partition((char*)"p1.ini");

	FS::mount(p);
	FS::format();


	File* file = nullptr;
	char* checker = nullptr;
	for (int i = 1; i < 151; i++)
	{
		string ext = ".dat";
		string a = "/name";
		string add(to_string(i));
		a += add += ext;

		char* name = new char[a.length() + 1];
		name[a.length()] = '\0';
		for (int i = 0; i < a.length(); i++)
			name[i] = a[i];
		file = FS::open((char*)a.c_str(), 'w');

		if (file == nullptr)
		{
			cout << "Neuspesno otvaranje fajla " << name << endl;
			break;
		}
		else
		{
			cout << "Uspesno otvaranje fajla " << name << endl;
			if (i == 25) {
				char buffer[601];
				buffer[600] = '\0';
				memset(buffer, 'a', 600);

				for (int i = 0; i < 5; i++) {
					char val = file->write(600, buffer);
					if (val == 0)
					{
						cout << "Upis neuspesan!" << endl;
						break;
					}
				}
				checker = name;
			}
			delete file;
			cout << "Zatvaranje fajla uspesno odradjeno" << endl;

		}
	}

	cout << "Broj fajlova na particij je: " << FS::readRootDir() << endl;
	cout << "Fajl " << checker << ((FS::doesExist(checker) == 0) ? "ne postoji" : "postoji") << " na particiji" << endl;
	FS::unmount();
	delete p;
	signalS(semMain);
	return 0;
}
int main()
{
	unsigned long threadId;
	srand((unsigned int)time(NULL));
	clock_t start = clock();

	HANDLE thr1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)basic_operation_tester, NULL, 0, &threadId);

	InitializeCriticalSection(&mut);

	for (int i = 0; i < 1; i++)
		waitS(semMain);

	CloseHandle(semMain);
	CloseHandle(semS);
	CloseHandle(sem12);
	CloseHandle(sem13);
	CloseHandle(semaphor);
	//CloseHandle(thr1);
	DeleteCriticalSection(&mut);

	clock_t end = clock();
	cout << "Test finished!" << endl;
	double tm = (end - start) * 1.0 * (1.0 * (CLOCKS_PER_SEC) / 1000);
	cout << "Time: " << tm << "ms!" << endl;
	system("pause");
	return 0;
}

