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
	myFCB->numberOfOpenFiles--;
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
}

char * KernelFile::getDataFromCacheAndUpdateCache(BytesCnt dataClusterStartByte) {
	if (cache.find(dataClusterStartByte) == cache.end()) {
		if (cache.size() >= cacheSize) {
			auto minIter = cache.begin();
			if (minIter->first == dataClusterStartByte) ++minIter;
			int minRef = minIter->second->referenceCount;
			for (auto iter = cache.begin(); iter != cache.end(); ++iter) {
				if (minRef == 1) break;
				if (iter->first != dataClusterStartByte && iter->second->referenceCount < minRef) {
					minIter = iter;
					minRef = iter->second->referenceCount;
				}
			}
			minIter->second->dataCluster->saveToDrive();
			cache.erase(minIter);
		}
		cache[dataClusterStartByte] = byteCntToDataCluster[dataClusterStartByte];
		cache[dataClusterStartByte]->referenceCount = 0;
		cache[dataClusterStartByte]->dataCluster->loadData();
	}
	cache[dataClusterStartByte]->referenceCount++;
	return cache[dataClusterStartByte]->dataCluster->getData();
}

char KernelFile::write(BytesCnt cnt, char * buffer) {
	if (mode == 'r' || cnt + currentPos > FirstLevelIndexCluster::getMaxFileSize()) return 0;
	if (fliCluster == nullptr) {
		BytesCnt cNo = 0;
		try {
			cNo = myFCB->bitVector.getFreeClusterNumberForUse();
			if (cNo == 0) return 0;
			myFCB->fliCluster = fliCluster = new FirstLevelIndexCluster(cNo, myFCB->part, false, false);
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
	int cao = 0;
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
			cao++;
			char * data = getDataFromCacheAndUpdateCache(dataClusterStartByte);
			cache[dataClusterStartByte]->dataCluster->setDirty();
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
			char * data = getDataFromCacheAndUpdateCache(dataClusterStartByte);
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
	if (newPos <= currentSize) {
		currentPos = newPos;
		return 1;
	}
	else {
		return 0;
	}
}

char KernelFile::eof() {
	return currentPos < currentSize ? 0 : 2;
}


char KernelFile::truncate() {
	if (fliCluster == nullptr) return 1;
	unsigned dcIndex = currentPos / ClusterSize;
	unsigned sliIndex = dcIndex / 512;
	dcIndex = dcIndex % 512;
	unsigned fullSLIStart = sliIndex + 1;
	
	std::vector<ClusterNo> cNoVec;
	auto& sliClusters = fliCluster->getSecondLevelIndexClusters();
	for (unsigned i = fullSLIStart; i < sliClusters.size(); i++) {
		cNoVec.push_back(sliClusters[i]->getClusterNumber());
		auto& dataClusters = sliClusters[i]->getDataClusters();
		for (unsigned j = 0; j < dataClusters.size(); j++) {  // NOLINT
			cNoVec.push_back(dataClusters[j]->getClusterNumber());
		}
		delete sliClusters[i];
	}
	sliClusters.erase(sliClusters.begin() + fullSLIStart, sliClusters.end());

	auto& dataClusters = sliClusters[sliIndex]->getDataClusters();
	for (unsigned i = dcIndex + 1; i < dataClusters.size(); i++) {
		cNoVec.push_back(dataClusters[i]->getClusterNumber());
		delete dataClusters[i];
	}
	dataClusters.erase(dataClusters.begin() + dcIndex + 1, dataClusters.end()); 

	if (currentPos % ClusterSize == 0) {
		cNoVec.push_back(dataClusters[dcIndex]->getClusterNumber());
		delete dataClusters[dcIndex];
		dataClusters.erase(dataClusters.begin() + dcIndex, dataClusters.end());
	}
	if (dataClusters.empty()) {
		cNoVec.push_back(sliClusters[sliIndex]->getClusterNumber());
		delete sliClusters[sliIndex];
		sliClusters.erase(sliClusters.begin() + sliIndex, sliClusters.end());
	}
	else {
		sliClusters[sliIndex]->refreshIndexData();
	}

	if (sliClusters.empty()) {
		cNoVec.push_back(fliCluster->getClusterNumber());
		delete fliCluster;
		myFCB->fliCluster = nullptr;
		myFCB->fcbData->firstIndexClusterNo = 0;
	}
	else {
		fliCluster->refreshIndexData();
	}
	myFCB->bitVector.freeUpClusters(cNoVec);
	currentSize = currentPos;
	myFCB->fcbData->fileSize = currentSize;
	myFCB->myDC.setDirty();

	return 0;
}


