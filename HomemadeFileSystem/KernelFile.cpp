#include "KernelFile.h"
#include "FirstLevelIndexCluster.h"
#include "PartitionError.h"
#include "ClusterFullException.h"


KernelFile::KernelFile(FCB * myFCB, char mode, FirstLevelIndexCluster * fli) {
	this->currentSize = myFCB->fcbData->fileSize;
	this->currentPos = mode != 'a' ? 0 : currentSize;
	this->mode = mode;
	this->myFCB = myFCB;
	fliCluster = fli;
	if (fli != nullptr) {
		BytesCnt bytesCnt = 0;
		for (int i = 0; i < fliCluster->getCurrentSize_32b(); i++) {
			for (int j = 0; j < (*fliCluster)[i].getCurrentSize_32b(); j++) {
				byteCntToDataCluster[bytesCnt] = &(*fliCluster)[i][j];

				bytesCnt += ClusterSize;
			}
		}
	}
}

KernelFile::~KernelFile() {
	if (mode == 'r') {
		myFCB->readCount--;
		if (myFCB->readCount == 0) {
			myFCB->currentMode = FCB::idle;
			WakeConditionVariable(&myFCB->writeCond);
			WakeAllConditionVariable(&myFCB->readCond);
		}
	}
	else {
		myFCB->currentMode = FCB::idle;
		WakeAllConditionVariable(&myFCB->readCond);
		WakeConditionVariable(&myFCB->writeCond);
	}
	//SAVE SHIT GOD DAAAAMN TODO: DO IT M8
}


char KernelFile::write(BytesCnt cnt, char * buffer) {
	if (mode == 'r' || cnt + currentPos > FirstLevelIndexCluster::getMaxFileSize()) return 0;
	if (fliCluster == nullptr) {
		try {
			auto cNo = myFCB->bitVector.getFreeClusterNumberForUse();
			if (cNo == 0) return 0;
			fliCluster = new FirstLevelIndexCluster(cNo, myFCB->part, false, false);
			fliCluster->initDataWithZeros();
		}
		catch(PartitionError&) {
			return 0;
		}
	}
	auto oldCurrentPos = currentPos;
	auto buffSize = cnt;
	BytesCnt start = 0;
	while (cnt > 0) {
		BytesCnt dataClusterStartByte = currentPos / ClusterSize * ClusterSize;
		if (byteCntToDataCluster[dataClusterStartByte] == nullptr) {
			if (fliCluster->getCurrentSize_32b() == 0) {
				try {
					auto cNo = myFCB->bitVector.getFreeClusterNumberForUse();
					if (cNo == 0) return 0;
					auto sli = fliCluster->addSecondLevelIndexCluster(cNo);
					sli.initDataWithZeros();
					cNo = myFCB->bitVector.getFreeClusterNumberForUse();
					if (cNo == 0) return 0;
					byteCntToDataCluster[dataClusterStartByte] = &sli.addDataCluster(cNo);
				}
				catch (PartitionError&) {
					return 0;
				}
			}
			else {
				try {
					auto sli = (*fliCluster)[fliCluster->getCurrentSize_32b() - 1];
					auto cNo = myFCB->bitVector.getFreeClusterNumberForUse();
					if (cNo == 0) return 0;
					bool sliFull = false;
					try {
						byteCntToDataCluster[dataClusterStartByte] = &sli.addDataCluster(cNo);
					}
					catch(ClusterFullException&) {
						sliFull = true;
					}
					if (sliFull) {
						try {
							fliCluster->addSecondLevelIndexCluster(cNo);
							cNo = myFCB->bitVector.getFreeClusterNumberForUse();
							if (cNo == 0) return 0;
							sli = (*fliCluster)[fliCluster->getCurrentSize_32b() - 1];
							byteCntToDataCluster[dataClusterStartByte] = &sli.addDataCluster(cNo);
						}
						catch (ClusterFullException&) {
							std::vector<unsigned long> v = { cNo };
							myFCB->bitVector.freeUpClusters(v);
							return 0;
						}
					}
				}
				catch (PartitionError&) {
					return 0;
				}
			}
			if (byteCntToDataCluster[dataClusterStartByte] != nullptr) {
				auto * dataCluster = byteCntToDataCluster[dataClusterStartByte];
				auto* data = dataCluster->loadData();
				unsigned int startPos = currentPos % ClusterSize;
				unsigned int writeSize = min(cnt, ClusterSize - startPos);
				memcpy(data + startPos, buffer + start, writeSize);
				start += writeSize;
				cnt -= writeSize;
				currentPos += writeSize;
				if (currentPos > currentSize) currentSize = currentPos;
			}
		}
	}


	return 1;;
}

BytesCnt KernelFile::read(BytesCnt cnt, char * buffer)  {	
	if (eof() || mode != 'r') return 0;

	auto oldCurrentPos = currentPos;
	auto buffSize = cnt;
	BytesCnt start = 0;
	while (cnt > 0) {
		BytesCnt dataClusterStartByte = currentPos / ClusterSize * ClusterSize;
		if (byteCntToDataCluster[dataClusterStartByte] != nullptr) {
			auto * dataCluster = byteCntToDataCluster[dataClusterStartByte];
			auto* data = dataCluster->loadData();
			unsigned int startPos = currentPos % ClusterSize;
			unsigned int writeSize = min(cnt, ClusterSize - startPos);
			if (dataClusterStartByte == currentSize / ClusterSize * ClusterSize) writeSize = min(writeSize, currentSize / ClusterSize * ClusterSize - startPos);
			memcpy(data + startPos, buffer + start, writeSize);
			start += writeSize;
			cnt -= writeSize;
			currentPos += writeSize;
			if (currentPos >= currentSize) break;
		}
	}

	return start;
}

char KernelFile::seek(BytesCnt newPos) {
	char returnVal = 0;
	if (newPos < currentSize) {
		currentPos = newPos;
		return 1;
	}
	else {
		return 0;
	}
}


char KernelFile::truncate()
{
	return 0;
}


