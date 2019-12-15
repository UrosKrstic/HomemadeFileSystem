#include "fs.h"
#include "KernelFS.h"
#include <iostream>

KernelFS * FS::myImpl = nullptr;


char FS::mount(Partition* partition) {
	//one time initialization of global structures
	InitOnceExecuteOnce(&kernel_fs::initOnceVariable, kernel_fs::InitFunction, nullptr, nullptr);

	std::cout << "Ulazi u kriticnu sekciju nit id = " << GetCurrentThreadId() << "\n";
	EnterCriticalSection(&kernel_fs::fsLock);
	std::cout << "Usla u kriticnu sekciju nit id = " << GetCurrentThreadId() << "\n";

	while (KernelFS::isPartitionMounted()) {
		std::cout << "(mount)Bice zablokirana nit sa id = " << GetCurrentThreadId() << "\n";
		SleepConditionVariableCS(&kernel_fs::isMountedCond, &kernel_fs::fsLock, INFINITE);
		std::cout << "(mount)Odblokirana nit, id = " << GetCurrentThreadId() << "\n";
	}
	std::cout << "radi mount, nit id = " << GetCurrentThreadId() << "\n";
	myImpl = new KernelFS();
	char returnValue = myImpl->mount(partition);

	LeaveCriticalSection(&kernel_fs::fsLock);
	std::cout << "Izasla iz kriticne sekcije nit id = " << GetCurrentThreadId() << "\n";
	return returnValue;
}

char FS::unmount() {
	EnterCriticalSection(&kernel_fs::fsLock);

	char returnValue = 0;
	if (KernelFS::isPartitionMounted()) {

		while (myImpl->openFiles()) {
			SleepConditionVariableCS(&kernel_fs::openFilesCond, &kernel_fs::fsLock, INFINITE);
		}

		returnValue = myImpl->unmount();
		delete myImpl;
	}

	LeaveCriticalSection(&kernel_fs::fsLock);
	return returnValue;
}

char FS::format() {
	EnterCriticalSection(&kernel_fs::fsLock);
	myImpl->setFormatingInProgress(true);
	char returnValue = 0;
	if (KernelFS::isPartitionMounted()) {
		
		while (myImpl->openFiles()) {
			SleepConditionVariableCS(&kernel_fs::openFilesCond, &kernel_fs::fsLock, INFINITE);
		}
		
		returnValue = myImpl->format();
	}
	myImpl->setFormatingInProgress(false);
	LeaveCriticalSection(&kernel_fs::fsLock);
	return returnValue;
}

FileCnt FS::readRootDir() {
	return KernelFS::isPartitionMounted() ? myImpl->getFileCount() : -1;
}

char FS::doesExist(char* fname) {
	if (KernelFS::isPartitionMounted()) {
		return myImpl->doesExist(std::string(fname));
	}
	return 0;
}

File * FS::open(char * fname, char mode) {
	EnterCriticalSection(&kernel_fs::fsLock);
	auto* returnValue = myImpl->open(fname, mode);
	LeaveCriticalSection(&kernel_fs::fsLock);
	return returnValue;
	
}

char FS::deleteFile(char * fname) {
	EnterCriticalSection(&kernel_fs::fsLock);
	auto returnValue = myImpl->deleteFile(fname);
	LeaveCriticalSection(&kernel_fs::fsLock);
	return returnValue;
	
}




