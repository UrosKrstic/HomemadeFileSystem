#include "kernel_fs.h"
#include <algorithm>
#include <iostream>


INIT_ONCE kernel_fs::initOnceVariable = INIT_ONCE_STATIC_INIT;
CRITICAL_SECTION kernel_fs::fsLock;
CONDITION_VARIABLE kernel_fs::isMountedCond;
CONDITION_VARIABLE kernel_fs::openFilesCond;

bool KernelFS::partitionMounted = false;
ClusterNo KernelFS::bitVectorClusterNo = 0;
ClusterNo KernelFS::rootDirFirstLevelIndexClusterNo = 1;


BOOL CALLBACK kernel_fs::InitFunction(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *lpContext) {
	InitializeConditionVariable(&kernel_fs::isMountedCond);
	InitializeConditionVariable(&kernel_fs::openFilesCond);
	InitializeCriticalSection(&kernel_fs::fsLock);
	std::cout << "Uspesno inicijalizovano\n";
	return true;
}

char KernelFS::mount(Partition* partition) {
	this->partition = partition;
	partitionMounted = true;
	partition->readCluster(bitVectorClusterNo, bitVector);
	partition->readCluster(rootDirFirstLevelIndexClusterNo, firstLvlIndex);

	return 1;
}

char KernelFS::unmount() {
	partition = nullptr;
	partitionMounted = false;
	std::cout << "Budi niti na mount cond\n";
	WakeAllConditionVariable(&kernel_fs::isMountedCond);
	return 1;
}

char KernelFS::format() {
	std::fill_n(bitVector, ClusterSize, 0);
	//memset(bitVector, 0, ClusterSize);
	//TODO: reset root dir
	return 1;
}

