#include "FCB.h"
#include  "KernelFile.h"
#include "FirstLevelIndexCluster.h"


FCB::FCB(FCBIndex fcbIndex, FCBData * fcbData, Partition * part) {
	this->fcbIndex = fcbIndex;
	this->fcbData = fcbData;
	this->part = part;
	if (fcbData->firstIndexClusterNo != 0) {
		FLICluster = new FirstLevelIndexCluster(fcbData->firstIndexClusterNo, part, true); // TODO: PROMENITI NA FALSE
	}
}

KernelFile * FCB::createKernelFile(char mode) {
	numberOfOpenFiles++;
	if (mode == 'w' || mode == 'r')
		return new KernelFile(fcbData->fileSize, 0, this);
	else
		return new KernelFile(fcbData->fileSize, fcbData->fileSize, this);
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
