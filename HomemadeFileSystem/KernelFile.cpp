#include "KernelFile.h"
#include "FirstLevelIndexCluster.h"
#include "PartitionError.h"
#include "ClusterFullException.h"
#include "NoFreeClustersException.h"
#include <ostream>
#include <iostream>

KernelFile::KernelFile(FCB * myFCB, char mode, FirstLevelIndexCluster * fli) {
	this->currentSize = myFCB->fileSize;
	this->currentPos = mode != 'a' ? 0 : currentSize;
	this->mode = mode;
	this->myFCB = myFCB;
	fliCluster = fli;
	/*if (fli != nullptr) {
		BytesCnt bytesCnt = 0;
		for (unsigned i = 0; i < fliCluster->getCurrentSize_32b(); i++) {
			for (unsigned j = 0; j < (*fliCluster)[i].getCurrentSize_32b(); j++) {
				byteCntToDataCluster[bytesCnt] = new DataClusterWithReferenceBit(&(*fliCluster)[i][j]);
				bytesCnt += ClusterSize;
			}
		}
	}*/
}

KernelFile::~KernelFile() {
	myFCB->numberOfOpenFiles--;
	myFCB->kernelFS.openFileCount--;
	cache.clear();
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
		if (fliCluster != nullptr)
			fliCluster->saveToDrive();
		myFCB->fileSize = currentSize;
		myFCB->fliCluster = fliCluster;
		myFCB->updateFCBData();
		WakeAllConditionVariable(&myFCB->readCond);
		WakeConditionVariable(&myFCB->writeCond);
	}
	if (myFCB->kernelFS.openFileCount == 0) WakeAllConditionVariable(&kernel_fs::openFilesCond);
}

//char * KernelFile::getDataFromCacheAndUpdateCache(BytesCnt dataClusterStartByte, DataClusterWithReferenceBit* currDC) {
//	auto currentIter = cache.find(dataClusterStartByte);
//	if (currentIter == cache.end()) {
//		if (cache.size() >= cacheSize) {
//			auto minIter = cache.begin();
//			if (minIter->first == dataClusterStartByte) ++minIter;
//			int minRef = minIter->second->referenceCount;
//			for (auto iter = cache.begin(); iter != cache.end(); ++iter) {
//				if (minRef == 1) break;
//				if (iter->first != dataClusterStartByte && iter->second->referenceCount < minRef) {
//					minIter = iter;
//					minRef = iter->second->referenceCount;
//				}
//			}
//			minIter->second->dataCluster->saveToDrive();
//			cache.erase(minIter);
//		}
//		cache[dataClusterStartByte] = currDC;
//		currDC->referenceCount = 0;
//		if (mode != 'r') currDC->dataCluster->setDirty();
//		return currDC->dataCluster->loadData();
//	}
//	currentIter->second->referenceCount++;
//	return currentIter->second->dataCluster->getData();
//}

//char * KernelFile::getDataFromCacheAndUpdateCache(DataCluster * dc) {
//	auto iter = cache.find(dc);
//	if (iter == cache.end()) {
//		auto dcRef = new DataClusterWithReferenceBit(dc);
//		dcRef->referenceCount = 1;
//		dc->setDirty();
//		if (cache.size() >= cacheSize) {
//			auto minIter = cache.begin();
//			int minRef = minIter->second->referenceCount;
//			for (auto i = cache.begin(); i != cache.end(); ++i) {
//				if (i->second->referenceCount == 1) break;
//				if (i->second->referenceCount < minRef) {
//					minIter = i;
//					minRef = i->second->referenceCount;
//				}
//			}
//			minIter->first->saveToDrive();
//			//delete minIter->second;
//			cache.erase(minIter);
//		}
//		cache[dc] = dcRef;
//		return dc->loadData();
//	}
//	else {
//		dc->setDirty();
//		iter->second->referenceCount++;
//		return dc->getData();
//	}
//}

char * KernelFile::getDataFromCacheAndUpdateCache(DataCluster * dc, bool isSmallData) {
	if (dc->getData() == nullptr) {
		dc->setDirty();
		if (cache.size() >= cacheSize) {
			if (!isSmallData) {
				auto minIter = cache.begin();
				auto minCount = (*minIter)->getReferenceCount();
				for (auto iter = cache.begin(); iter != cache.end(); ++iter) {
					auto count = (*iter)->getReferenceCount();
					if (count < minCount) {
						minIter = iter;
						minCount = count;
					}
					if (count == 1) break;
				}
				(*minIter)->saveToDrive();
			}
			else {
				auto iter = cache.erase(cache.begin());
				(*iter)->saveToDrive();
			}
		}
		cache.push_back(dc);
		dc->resetReferenceCount();
	}
	dc->setDirty();
	dc->addReference();
	return dc->loadData();
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
			myFCB->fliNo = cNo;
		}
		catch(PartitionError&) {
			std::vector<unsigned long> v = { cNo };
			myFCB->bitVector.freeUpClusters(v);
			return 0;
		}
	}
	BytesCnt oldCurrentPos = currentPos;
	BytesCnt start = 0;
	auto& sliClusters = fliCluster->getSecondLevelIndexClusters();
	bool isSmallData = cnt <= defaultCacheSize;
	if (cnt >= 512 * ClusterSize) cacheSize = cnt / (ClusterSize * 10);

	while (cnt > 0) {
		try {
			/*BytesCnt dataClusterStartByte = currentPos / ClusterSize * ClusterSize;
			auto currentDataCluster = byteCntToDataCluster[dataClusterStartByte];
			if (currentDataCluster == nullptr) {
				if (fliCluster->getCurrentSize_32b() == 0) {
					auto cNo = myFCB->bitVector.getFreeClusterNumberForUse();
					auto& sli = fliCluster->addSecondLevelIndexCluster(cNo);
					cNo = myFCB->bitVector.getFreeClusterNumberForUse();
					currentDataCluster = new DataClusterWithReferenceBit(sli.addCluster(cNo));
				}
				else {
					auto& sli = (*fliCluster)[fliCluster->getCurrentSize_32b() - 1];
					auto cNo = myFCB->bitVector.getFreeClusterNumberForUse();
					bool sliFull = false;
					try {
						currentDataCluster = new DataClusterWithReferenceBit(sli.addCluster(cNo));
					}
					catch (ClusterFullException&) {
						sliFull = true;
						delete currentDataCluster;
					}
					if (sliFull) {
						try {
							fliCluster->addSecondLevelIndexCluster(cNo);
							cNo = myFCB->bitVector.getFreeClusterNumberForUse();
							auto& s = (*fliCluster)[fliCluster->getCurrentSize_32b() - 1];
							currentDataCluster = new DataClusterWithReferenceBit(s.addCluster(cNo));
						}
						catch (ClusterFullException&) {
							std::vector<unsigned long> v = { cNo };
							myFCB->bitVector.freeUpClusters(v);
							return 0;
						}
					}
				}
				byteCntToDataCluster[dataClusterStartByte] = currentDataCluster;
			}*/
			oldCurrentPos = currentPos;
			unsigned dcIndex = currentPos / ClusterSize;
			unsigned sliIndex = dcIndex / 512;
			dcIndex %= 512;
			ClusterNo cNo = 0;
			if (sliIndex >= sliClusters.size()) {
				try {
					cNo = myFCB->bitVector.getFreeClusterNumberForUse();
					fliCluster->addSecondLevelIndexCluster(cNo);
				}
				catch (ClusterFullException&) {
					std::vector<unsigned long> v = { cNo };
					myFCB->bitVector.freeUpClusters(v);
					return 0;
				}
			}
			auto& sliCluster = sliClusters[sliIndex];
			auto& dataClusters = sliCluster->getDataClusters();
			if (dcIndex >= dataClusters.size()) {
				try {
					cNo = myFCB->bitVector.getFreeClusterNumberForUse();
					sliCluster->addCluster(cNo);
				}
				catch (ClusterFullException&) {
					//std::cout << "oops" << std::endl;
					return 0;
				}
			}
			
			char * data = getDataFromCacheAndUpdateCache(dataClusters[dcIndex], isSmallData);
			dataClusters[dcIndex]->setDirty();
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
			std::cout << "Ostao bez klastera" << std::endl;
			return 0;
		}
		catch (PartitionError&) {
			currentPos = oldCurrentPos;
			std::cout << "greska particije" << std::endl;
			return 0;
		}
	}
	return 1;
}

BytesCnt KernelFile::read(BytesCnt cnt, char * buffer)  {	
	if (eof()) return 0;
	auto oldCurrentPos = currentPos;
	BytesCnt start = 0;
	bool isSmallData = cnt <= cacheSize;
	BytesCnt currentSizeStartByte = currentSize / ClusterSize * ClusterSize;
	BytesCnt currentSizeClusterOff = currentSize % ClusterSize;
	while (cnt > 0) {
		BytesCnt dataClusterStartByte = currentPos / ClusterSize * ClusterSize;

		unsigned dcIndex = currentPos / ClusterSize;
		unsigned sliIndex = dcIndex / 512;
		dcIndex %= 512;
		
		auto& sliClusters = fliCluster->getSecondLevelIndexClusters();
		if (sliIndex >= sliClusters.size()) return 0;
		auto& dataClusters = sliClusters[sliIndex]->getDataClusters();
		if (dcIndex < dataClusters.size()) {
			char * data = getDataFromCacheAndUpdateCache(dataClusters[dcIndex], isSmallData);
			unsigned int startPos = currentPos % ClusterSize; 
			unsigned int writeSize = min(cnt, ClusterSize - startPos);
			if (dataClusterStartByte == currentSizeStartByte) writeSize = min(writeSize, currentSizeClusterOff - startPos);
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
		myFCB->fliNo = 0;
	}
	else {
		fliCluster->refreshIndexData();
	}
	myFCB->bitVector.freeUpClusters(cNoVec);
	currentSize = currentPos;
	myFCB->fileSize = currentSize;
	myFCB->updateFCBData();

	return 0;
}
