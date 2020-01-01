#include "FCB.h"
#include  "KernelFile.h"
#include "FirstLevelIndexCluster.h"
#include "PartitionError.h"
#include "file.h"


FCB::FCB(FCBIndex& fcbInd, FCBData * data, Partition * p, BitVector& bitV, KernelFS& kerFS, DataCluster& dc) : fcbIndex(fcbInd), fcbData(data), part(p), bitVector(bitV), kernelFS(kerFS), myDC(dc) {
	InitializeCriticalSection(&criticalSection);
	InitializeConditionVariable(&readCond);
	InitializeConditionVariable(&writeCond);
}
FCB::FCB(FCBIndex&& fcbInd, FCBData * data, Partition * p, BitVector& bitV, KernelFS& kerFS, DataCluster& dc) : fcbIndex(fcbInd), fcbData(data), part(p), bitVector(bitV), kernelFS(kerFS), myDC(dc) {
	InitializeCriticalSection(&criticalSection);
	InitializeConditionVariable(&readCond);
	InitializeConditionVariable(&writeCond);
}

FCB::~FCB() {
	DeleteCriticalSection(&criticalSection);
	if (fliCluster != nullptr) {
		fliCluster->saveToDrive();
	}
}


File * FCB::createFileInstance(char mode) {
	EnterCriticalSection(&criticalSection);
	File * file = nullptr;
	while (mode == 'r' && currentMode != idle && currentMode != reading) {
		SleepConditionVariableCS(&readCond, &criticalSection, INFINITE);
	}

	while (mode != 'r' && currentMode != idle) {
		SleepConditionVariableCS(&writeCond, &criticalSection, INFINITE);
	}
	try {
		if (fcbData->firstIndexClusterNo != 0 && loadFLI) {
			loadFLI = false;
			fliCluster = new FirstLevelIndexCluster(fcbData->firstIndexClusterNo, part, false, true);
			fliCluster->loadSLIClusters();
		}

		if (mode == 'r') { currentMode = reading; readCount++; }
		if (mode == 'w') currentMode = writing;
		if (mode == 'a') currentMode = append;

		auto * kernelFile = new KernelFile(this, mode, fliCluster);
		file = new File();
		file->myImpl = kernelFile;
	}
	catch(PartitionError&) {}
	LeaveCriticalSection(&criticalSection);
	return file;
}

void FCB::clearClusters() {
	std::vector<BytesCnt> cNoVector;
	try {
		if (fliCluster == nullptr && fcbData->firstIndexClusterNo != 0) {
			fliCluster = new FirstLevelIndexCluster(fcbData->firstIndexClusterNo, part, false, true);
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
			fcbData->firstIndexClusterNo = 0;
			fcbData->fileSize = 0;
			myDC.setDirty();
			fliCluster = nullptr;
		}
	}
	catch(PartitionError&) {}
}

void FCB::saveToDrive() {
	if (fliCluster != nullptr) fliCluster->saveToDrive();
}

