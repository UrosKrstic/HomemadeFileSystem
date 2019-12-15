#include "KernelFS.h"
#include <iostream>
#include "FCB.h"


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
	bitVector = new BitVector(bitVectorClusterNo, partition);
	rootDirFirstLvlIndex = new FirstLevelIndexCluster(rootDirFirstLevelIndexClusterNo, partition, true);

	for (unsigned i = 0; i < rootDirFirstLvlIndex->getCurrentSize_32b(); i++) {
		for (unsigned j = 0; j < rootDirFirstLvlIndex[i].getCurrentSize_32b(); j++) {
			unsigned fileNum = ClusterSize / sizeof(FCBData);
			auto *fcbData = reinterpret_cast<FCBData*>(rootDirFirstLvlIndex[i][j].getData());
			for (unsigned k = 0; k < fileNum; k++) {
				if (fcbData[k].allZeros()) break;
				else if (!fcbData[k].isEmpty()) {
					std::string fullName = std::string("/") + std::string(fcbData[k].name);
					fullName.erase(fullName.find_first_of(std::string(" ")));
					std::string ext(std::string(".") + std::string(fcbData[k].ext));
					ext.resize(4);
					fullName += ext;
					fileNameToFCBmap[fullName] = new FCB(i, j, k, fcbData[k]);
				}
			}
		}
	}
	fileCount = fileNameToFCBmap.size();

	return 1;
}

char KernelFS::unmount() {
	partition = nullptr;
	partitionMounted = false;
	//TODO: PRESNIMITI SVE NA PARTICIJU 
	delete bitVector;
	delete rootDirFirstLvlIndex;
	std::cout << "Budi niti na mount cond\n";
	WakeAllConditionVariable(&kernel_fs::isMountedCond);
	return 1;
}

char KernelFS::format() {
	bitVector->format();
	//TODO: reset root dir
	return 1;
}

char KernelFS::doesExist(std::string fname) {
	return fileNameToFCBmap[fname] != nullptr;
}

File * KernelFS::open(char * fname, char mode) {
	
}

char KernelFS::deleteFile(char * fname) {
	return 0;
}

