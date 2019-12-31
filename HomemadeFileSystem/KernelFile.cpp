#include "KernelFile.h"
#include "FirstLevelIndexCluster.h"
#include "PartitionError.h"
#include "ClusterFullException.h"
#include "NoFreeClustersException.h"


KernelFile::KernelFile(FCB * myFCB, char mode, FirstLevelIndexCluster * fli) {
	this->currentSize = myFCB->fcbData->fileSize;
	this->currentPos = mode != 'a' ? 0 : currentSize;
	this->mode = mode;
	this->myFCB = myFCB;
	fliCluster = fli;
	if (fli != nullptr) {
		BytesCnt bytesCnt = 0;
		for (unsigned i = 0; i < fliCluster->getCurrentSize_32b(); i++) {
			for (unsigned j = 0; j < (*fliCluster)[i].getCurrentSize_32b(); j++) {
				byteCntToDataCluster[bytesCnt] = new DataClusterWithReferenceBit(&(*fliCluster)[i][j]);
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
		fliCluster->saveToDrive();
		myFCB->fcbData->fileSize = currentSize;
		myFCB->fliCluster = fliCluster;
		myFCB->myDC.setDirty();

	}
	//SAVE SHIT GOD DAAAAMN TODO: DO IT M8
}


char KernelFile::write(BytesCnt cnt, char * buffer) {
	if (mode == 'r' || cnt + currentPos > FirstLevelIndexCluster::getMaxFileSize()) return 0;
	if (fliCluster == nullptr) {
		BytesCnt cNo = 0;
		try {
			cNo = myFCB->bitVector.getFreeClusterNumberForUse();
			if (cNo == 0) return 0;
			fliCluster = new FirstLevelIndexCluster(cNo, myFCB->part, false, false);
			fliCluster->initDataWithZeros();
			myFCB->fcbData->firstIndexClusterNo = cNo;
			myFCB->myDC.setDirty();
		}
		catch(PartitionError&) {
			std::vector<unsigned long> v = { cNo };
			myFCB->bitVector.freeUpClusters(v);
			return 0;
		}
	}
	auto oldCurrentPos = currentPos;
	auto buffSize = cnt;
	BytesCnt start = 0;
	while (cnt > 0) {
		try {
			BytesCnt dataClusterStartByte = currentPos / ClusterSize * ClusterSize;
			if (byteCntToDataCluster[dataClusterStartByte] == nullptr) {
				if (fliCluster->getCurrentSize_32b() == 0) {
					auto cNo = myFCB->bitVector.getFreeClusterNumberForUse();
					auto& sli = fliCluster->addSecondLevelIndexCluster(cNo);
					cNo = myFCB->bitVector.getFreeClusterNumberForUse();
					byteCntToDataCluster[dataClusterStartByte] = new DataClusterWithReferenceBit(sli.addCluster(cNo));
				}
				else {
					auto& sli = (*fliCluster)[fliCluster->getCurrentSize_32b() - 1];
					auto cNo = myFCB->bitVector.getFreeClusterNumberForUse();
					bool sliFull = false;
					try {
						byteCntToDataCluster[dataClusterStartByte] = new DataClusterWithReferenceBit(sli.addCluster(cNo));
					}
					catch (ClusterFullException&) {
						sliFull = true;
					}
					if (sliFull) {
						try {
							fliCluster->addSecondLevelIndexCluster(cNo);
							cNo = myFCB->bitVector.getFreeClusterNumberForUse();
							auto& s = (*fliCluster)[fliCluster->getCurrentSize_32b() - 1];
							byteCntToDataCluster[dataClusterStartByte] = new DataClusterWithReferenceBit(s.addCluster(cNo));
						}
						catch (ClusterFullException&) {
							std::vector<unsigned long> v = { cNo };
							myFCB->bitVector.freeUpClusters(v);
							return 0;
						}
					}
				}
			}

			auto * dataClusterWithRefBit = byteCntToDataCluster[dataClusterStartByte];
			dataClusterWithRefBit->isReferenced = true;

			if (cache[dataClusterStartByte] == nullptr) {
				if (cache.size() >= cacheSize) {
					for (auto iter = cache.begin(); iter != cache.end(); ++iter) {
						if (iter->first != dataClusterStartByte) {
							iter->second->dataCluster->saveToDrive();
							cache.erase(iter);
							break;
						}
					}
				}
				cache[dataClusterStartByte] = dataClusterWithRefBit;
				dataClusterWithRefBit->dataCluster->loadData();
			}

			char * data = dataClusterWithRefBit->dataCluster->getData();
			dataClusterWithRefBit->dataCluster->setDirty();
			unsigned int startPos = currentPos % ClusterSize;
			unsigned int writeSize = min(cnt, ClusterSize - startPos);
			memcpy(data + startPos, buffer + start, writeSize);
			start += writeSize;
			cnt -= writeSize;
			currentPos += writeSize;
			if (currentPos > currentSize) currentSize = currentPos;
		}
		catch(NoFreeClustersException&) {
			currentPos = oldCurrentPos;
			//TODO: ADD TRUNCATION TO OLD POSITION
			return 0;
		}
		catch (PartitionError&) {
			currentPos = oldCurrentPos;
			//TODO: ADD TRUNCATION TO OLD POSITION
			return 0;
		}
	}
	return 1;
}

BytesCnt KernelFile::read(BytesCnt cnt, char * buffer)  {	
	if (eof() || mode != 'r') return 0;
	auto oldCurrentPos = currentPos;
	auto buffSize = cnt;
	BytesCnt start = 0;
	while (cnt > 0) {
		BytesCnt dataClusterStartByte = currentPos / ClusterSize * ClusterSize;
		if (byteCntToDataCluster[dataClusterStartByte] != nullptr) {
			auto * dataClusterWithRefBit = byteCntToDataCluster[dataClusterStartByte];
			dataClusterWithRefBit->isReferenced = true;

			if (cache[dataClusterStartByte] == nullptr) {
				if (cache.size() >= cacheSize) {
					for (auto iter = cache.begin(); iter != cache.end(); ++iter) {
						if (iter->first != dataClusterStartByte) {
							iter->second->dataCluster->saveToDrive();
							cache.erase(iter);
							break;
						}
					}
				}
				cache[dataClusterStartByte] = dataClusterWithRefBit;
			}
			char * data = dataClusterWithRefBit->dataCluster->loadData();
			unsigned int startPos = currentPos % ClusterSize; 
			unsigned int writeSize = min(cnt, ClusterSize - startPos);
			if (dataClusterStartByte == currentSize / ClusterSize * ClusterSize) writeSize = min(writeSize, currentSize % ClusterSize - startPos);
			memcpy(buffer + start, data + startPos, writeSize);
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


char KernelFile::truncate() {
	int dcIndex = currentPos / ClusterSize;
	int sliIndex = dcIndex / 512;
	dcIndex = dcIndex % 512;
	dcIndex++;
	if (dcIndex == 512) {
		dcIndex = 0;
		sliIndex++;
	}
	std::vector<ClusterNo> cNoVec;
	auto& sliClusters = fliCluster->getSecondLevelIndexClusters();
	for (auto iter  = sliClusters.begin() + dcIndex; iter < sliClusters.end(); ++iter) {
		
	}



	return 0;
}


