#include "fs.h"
#include "KernelFS.h"
#include <iostream>

KernelFS * FS::myImpl = nullptr;


char FS::mount(Partition* partition) {
	InitOnceExecuteOnce(&kernel_fs::initOnceVariable, kernel_fs::InitFunction, nullptr, nullptr);

	EnterCriticalSection(&kernel_fs::fsLock);
	while (KernelFS::isPartitionMounted() && GetCurrentThreadId() != myImpl->getThreadID()) {
		SleepConditionVariableCS(&kernel_fs::isMountedCond, &kernel_fs::fsLock, INFINITE);
	}
	myImpl = new KernelFS();
	char returnValue = myImpl->mount(partition);
	LeaveCriticalSection(&kernel_fs::fsLock);
	return returnValue;
}

char FS::unmount() {
	InitOnceExecuteOnce(&kernel_fs::initOnceVariable, kernel_fs::InitFunction, nullptr, nullptr);
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
	InitOnceExecuteOnce(&kernel_fs::initOnceVariable, kernel_fs::InitFunction, nullptr, nullptr);
	EnterCriticalSection(&kernel_fs::fsLock);
	char returnValue = 0;
	if (KernelFS::isPartitionMounted()) {
		myImpl->setFormatingInProgress(true);
		while (myImpl->openFiles()) {
			SleepConditionVariableCS(&kernel_fs::openFilesCond, &kernel_fs::fsLock, INFINITE);
		}
		returnValue = myImpl->format();
		myImpl->setFormatingInProgress(false);
	}
	LeaveCriticalSection(&kernel_fs::fsLock);
	return returnValue;
}

FileCnt FS::readRootDir() {
	return KernelFS::isPartitionMounted() ? myImpl->getFileCount() : -1;
}

char FS::doesExist(char* fname) {
	if (KernelFS::isPartitionMounted()) {
		std::string fpath(fname);
		return myImpl->doesExist(fpath);
	}
	return 0;
}

File * FS::open(char * fname, char mode) {
	InitOnceExecuteOnce(&kernel_fs::initOnceVariable, kernel_fs::InitFunction, nullptr, nullptr);
	EnterCriticalSection(&kernel_fs::fsLock);
	File * returnValue = nullptr;
	if (KernelFS::isPartitionMounted()) {
		returnValue = myImpl->open(fname, mode);
	}
	LeaveCriticalSection(&kernel_fs::fsLock);
	return returnValue;
	
}

char FS::deleteFile(char * fname) {
	InitOnceExecuteOnce(&kernel_fs::initOnceVariable, kernel_fs::InitFunction, nullptr, nullptr);
	EnterCriticalSection(&kernel_fs::fsLock);
	char returnValue = 0;
	if (KernelFS::isPartitionMounted()) {
		returnValue = myImpl->deleteFile(fname);
	}
	LeaveCriticalSection(&kernel_fs::fsLock);
	return returnValue;
	
}




