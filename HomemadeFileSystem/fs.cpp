#include "fs.h"
#include "kernel_fs.h"


char FS::mount(Partition* partition) {
	EnterCriticalSection(&fsLock);
	InitOnceExecuteOnce(&initOnceVariable, CondVarInitFunction, nullptr, nullptr);

	while (KernelFS::isPartitionMounted()) {
		SleepConditionVariableCS(&isMountedCond, &fsLock, INFINITE);
	}
	myImpl = new KernelFS();
	char returnValue = myImpl->mount(partition);

	LeaveCriticalSection(&fsLock);
	return returnValue;
}

char FS::unmount() {
	EnterCriticalSection(&fsLock);

	char returnValue = 0;
	if (KernelFS::isPartitionMounted()) {

		while (myImpl->openFiles()) {
			SleepConditionVariableCS(&openFilesCond, &fsLock, INFINITE);
		}

		returnValue = myImpl->unmount();
		delete myImpl;
	}

	LeaveCriticalSection(&fsLock);
	return returnValue;
}

char FS::format() {
	EnterCriticalSection(&fsLock);
	myImpl->setFormatingInProgress(true);
	char returnValue = 0;
	if (KernelFS::isPartitionMounted()) {
		
		while (myImpl->openFiles()) {
			SleepConditionVariableCS(&openFilesCond, &fsLock, INFINITE);
		}
		
		myImpl->format();
	}
	myImpl->setFormatingInProgress(false);
	LeaveCriticalSection(&fsLock);
	return returnValue;
}


