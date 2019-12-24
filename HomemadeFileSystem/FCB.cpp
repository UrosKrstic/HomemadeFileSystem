#include "FCB.h"
#include  "KernelFile.h"
#include "FirstLevelIndexCluster.h"
#include "PartitionError.h"
#include "file.h"


FCB::FCB(FCBIndex& fcbInd, FCBData * data, Partition * p, BitVector& bitV, KernelFS& kerFS) : fcbIndex(fcbInd), fcbData(data), part(p), bitVector(bitV), kernelFS(kerFS) {
	InitializeCriticalSection(&criticalSection);
	InitializeConditionVariable(&readCond);
	InitializeConditionVariable(&writeCond);
}
FCB::FCB(FCBIndex&& fcbInd, FCBData * data, Partition * p, BitVector& bitV, KernelFS& kerFS) : fcbIndex(fcbInd), fcbData(data), part(p), bitVector(bitV), kernelFS(kerFS) {
	InitializeCriticalSection(&criticalSection);
	InitializeConditionVariable(&readCond);
	InitializeConditionVariable(&writeCond);
}

FCB::~FCB() {
	DeleteCriticalSection(&criticalSection);
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
		if (fcbData->firstIndexClusterNo != 0) {
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


char FCB::write(BytesCnt cnt, char * buffer, unsigned int & currentPosOfFile, unsigned int & currentSizeOfFile) {
	return 0;
}

BytesCnt FCB::read(BytesCnt cnt, char * buffer, unsigned int & currentPosOfFile) {
	return 0;
}

char FCB::truncate(unsigned int & currentPosOfFile, unsigned int & currentSizeOfFile) {
	return 0;
}
