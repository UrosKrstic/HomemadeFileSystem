#include "kernel_fs.h"
#include <algorithm>
using namespace std;

BOOL CALLBACK CondVarInitFunction(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *lpContext) {
	InitializeConditionVariable(&isMountedCond);
	InitializeConditionVariable(&openFilesCond);
	InitializeCriticalSection(&fsLock);
	return true;
}

char KernelFS::mount(Partition* partition) {
	this->partition = partition;
	return 1;
}

char KernelFS::unmount() {
	partition = nullptr;
	partitionMounted = false;
	WakeAllConditionVariable(&isMountedCond);
	return 1;
}

char KernelFS::format() {
	fill_n(bitVector, ClusterSize, 0);
	//memset(bitVector, 0, ClusterSize);
	//TODO: reset root dir
	return 1;
}

