#include "FCB.h"


FCB::FCB(unsigned secondLvlIndex, unsigned dataClusterIndex, unsigned rowInDataCluster, KernelFS::FCBData &fcbData, Partition * part) {
	this->secondLvlIndex = secondLvlIndex;
	this->dataClusterIndex = dataClusterIndex;
	this->rowInDataCluster = rowInDataCluster;
	strcpy(name, fcbData.name);
	strcpy(ext, fcbData.ext);
	fileSize = fcbData.fileSize;
	this->part = part;
	if (fcbData.firstIndexClusterNo != 0) {
		FLICluster = new FirstLevelIndexCluster(fcbData.firstIndexClusterNo, part, true); // TODO: PROMENITI NA FALSE
	}
}

KernelFile * FCB::createKernelFile(char mode) {
	return nullptr;
}
