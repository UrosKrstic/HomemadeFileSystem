#include "FCB.h"
#include  "KernelFile.h"
#include "FirstLevelIndexCluster.h"
#include "RootDirMemoryHandler.h"
#include "PartitionError.h"
#include "file.h"


FCB::FCB(FCBIndex& fcbInd, FCBData * data, Partition * p, BitVector& bitV, KernelFS& kerFS, RootDirMemoryHandler* root) : fcbIndex(fcbInd), part(p), bitVector(bitV), kernelFS(kerFS), rootDir(root) {
	fileSize = data->fileSize;
	fliNo = data->firstIndexClusterNo;
	InitializeCriticalSection(&criticalSection);
	InitializeCriticalSection(&blockedThreadCritSection);
	InitializeConditionVariable(&readCond);
	InitializeConditionVariable(&writeCond);
	InitializeConditionVariable(&noBlockedThreads);

}
FCB::FCB(FCBIndex&& fcbInd, FCBData * data, Partition * p, BitVector& bitV, KernelFS& kerFS, RootDirMemoryHandler* root) : fcbIndex(fcbInd), part(p), bitVector(bitV), kernelFS(kerFS), rootDir(root) {
	fileSize = data->fileSize;
	fliNo = data->firstIndexClusterNo;
	InitializeCriticalSection(&criticalSection);
	InitializeCriticalSection(&blockedThreadCritSection);
	InitializeConditionVariable(&readCond);
	InitializeConditionVariable(&writeCond);
	InitializeConditionVariable(&noBlockedThreads);
}

FCB::~FCB() {
	DeleteCriticalSection(&criticalSection);
	if (fliCluster != nullptr) {
		fliCluster->saveToDrive();
	}
	deleted = true;
	WakeAllConditionVariable(&readCond);
	WakeAllConditionVariable(&writeCond);
	EnterCriticalSection(&blockedThreadCritSection);
	while (blockedThreadCount > 0) {
		SleepConditionVariableCS(&noBlockedThreads, &blockedThreadCritSection, INFINITE);
	}
	LeaveCriticalSection(&blockedThreadCritSection);
	DeleteCriticalSection(&blockedThreadCritSection);
}


File * FCB::createFileInstance(char mode) {
	EnterCriticalSection(&criticalSection);
	File * file = nullptr;
	while (mode == 'r' && currentMode != idle && currentMode != reading) {
		blockedThreadCount++;
		SleepConditionVariableCS(&readCond, &criticalSection, INFINITE);
		blockedThreadCount--;
		if (blockedThreadCount == 0) WakeAllConditionVariable(&noBlockedThreads);
		if (deleted) return nullptr;
	}

	while (mode != 'r' && currentMode != idle) {
		blockedThreadCount++;
		SleepConditionVariableCS(&writeCond, &criticalSection, INFINITE);
		blockedThreadCount--;
		if (blockedThreadCount == 0) WakeAllConditionVariable(&noBlockedThreads);
		if (deleted) return nullptr;
	}
	try {
		if (fliNo != 0 && loadFLI) {
			loadFLI = false;
			fliCluster = new FirstLevelIndexCluster(fliNo, part, false, true);
			fliCluster->loadSLIClusters();
		}

		if (mode == 'r') { currentMode = reading; readCount++; }
		if (mode == 'w') currentMode = writing;
		if (mode == 'a') currentMode = append;

		auto * kernelFile = new KernelFile(this, mode, fliCluster);
		file = new File();
		file->myImpl = kernelFile;
		numberOfOpenFiles++;
		kernelFS.openFileCount++;
	}
	catch(PartitionError&) {
		delete fliCluster;
		fliCluster = nullptr;
	}
	LeaveCriticalSection(&criticalSection);
	return file;
}

void FCB::clearClusters() {
	std::vector<BytesCnt> cNoVector;
	try {
		if (fliCluster == nullptr && fliNo != 0) {
			fliCluster = new FirstLevelIndexCluster(fliNo, part, false, true);
			fliCluster->loadSLIClusters();
		}
		if (fliCluster != nullptr) {
			cNoVector.push_back(fliCluster->getClusterNumber());
			for (unsigned i = 0; i < fliCluster->getCurrentSize_32b(); i++) {
				cNoVector.push_back((*fliCluster)[i].getClusterNumber());
				for (unsigned j = 0; j < (*fliCluster)[i].getCurrentSize_32b(); j++) {
					cNoVector.push_back((*fliCluster)[i][j].getClusterNumber());
				}
				(*fliCluster)[i].getDataClusters().clear();
			}
			fliCluster->getSecondLevelIndexClusters().clear();
 			bitVector.freeUpClusters(cNoVector);
			delete fliCluster;
			fliNo = 0;
			fileSize = 0;
			fliCluster = nullptr;
		}
	}
	catch(PartitionError&) {}
}

void FCB::saveToDrive() {
	if (fliCluster != nullptr) fliCluster->saveToDrive();
}

void FCB::setFCBDataToFree() {
	auto* fcbData = reinterpret_cast<FCBData*>(rootDir->getDCFromCacheAndUpdateCache(fcbIndex.secondLvlIndex, fcbIndex.dataClusterIndex));
	fcbData[fcbIndex.rowInDataCluster].name[0] = 0;
}

void FCB::updateFCBData() {
	auto* fcbData = reinterpret_cast<FCBData*>(rootDir->getDCFromCacheAndUpdateCache(fcbIndex.secondLvlIndex, fcbIndex.dataClusterIndex));
	fcbData[fcbIndex.rowInDataCluster].fileSize = fileSize;
	fcbData[fcbIndex.rowInDataCluster].firstIndexClusterNo = fliNo;
}

