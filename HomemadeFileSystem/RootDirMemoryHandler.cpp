#include "RootDirMemoryHandler.h"
#include "FCB.h"
#include <iostream>

RootDirMemoryHandler::~RootDirMemoryHandler() {
	for (auto& fcb : nameToFCBmap) {
		delete fcb.second;
		fcb.second = nullptr;
	}
}

std::map<std::string, FCB*>* RootDirMemoryHandler::getNameToFCBMap() {
	if (!isMapCreated) {
		unsigned long i = 0, j = 0, k = 0;
		for (i = 0; i < FLICluster.getCurrentSize_32b(); i++) {
			for (j = 0; j < FLICluster[i].getCurrentSize_32b(); j++) {
				auto *fcbData = reinterpret_cast<FCB::FCBData*>(FLICluster[i][j].getData());
				for ( k = 0; k < rowCountInDataCluster; k++) {
					if (fcbData[k].allZeros()) {
						nextFreeSlot.setIndices(i, j, k);
						return &nameToFCBmap;
					}
					else if (!fcbData[k].isEmpty()) {
						std::string fullName = std::string("/") + std::string(fcbData[k].name, FNAMELEN);
						try {
							fullName.erase(fullName.find_first_of(std::string(" ")));
						} catch(std::exception&) {}
						std::string ext(std::string(".") + std::string(fcbData[k].ext, FEXTLEN));
						fullName += ext;
						//std::cout << "Puno ime starog fajla sa diska: " << fullName.c_str() << std::endl;
						nameToFCBmap[fullName] = new FCB(FCB::FCBIndex(i, j, k), &fcbData[k], part, bitVector, kernelFS, FLICluster[i][j]);
					}
					else {
						leftoverFreeFileSlots.push(FCBIndex(i, j, k));
					}
				}
			}
		}
		nextFreeSlot.setIndices(i, j, k);
	}
	return &nameToFCBmap;
}

FCB * RootDirMemoryHandler::createNewFile(std::string& fpath) {
	std::string ext = fpath.substr(fpath.size() - FEXTLEN);
	std::string name = fpath.substr(1, fpath.size() - FEXTLEN - 2);
	if (name.size() > FNAMELEN) name.erase(FNAMELEN);
	while (name.size() < FNAMELEN) name += std::string(" ");
	//std::cout << name.c_str() << "|" << ext.c_str() << std::endl;
	FCBIndex ind;
	bool newDataCluster = false, newSecondLevelIndexCluster = false;
	if (!leftoverFreeFileSlots.empty()) {
		std::cout << "Ima leftover spot\n";
		auto fcbIndex = leftoverFreeFileSlots.front();
		ind = fcbIndex;
		leftoverFreeFileSlots.pop();
		std::cout << "Spot: " << fcbIndex.sli << " " << fcbIndex.dc << " " << fcbIndex.ridc << std::endl; 
		FLICluster[fcbIndex.sli][fcbIndex.dc].setDirty();
	}
	else {
		if (FLICluster.getCurrentSize_32b() == 0) {
			unsigned int cNo = bitVector.getFreeClusterNumberForUse();
			if (cNo == 0) return nullptr;
			FLICluster.addSecondLevelIndexCluster(cNo);
			FLICluster.setDirty();
			cNo = bitVector.getFreeClusterNumberForUse();
			if (cNo == 0) return nullptr;
			FLICluster[0].addDataCluster(cNo, false);
			FLICluster[0].setDirty();
			FLICluster[0][0].setDirty();
		}
		else {
			FCBIndex old;
			try {
				old = nextFreeSlot;
				if (nextFreeSlot.ridc >= rowCountInDataCluster) {
					newDataCluster = true;
					nextFreeSlot.ridc = 0;
					nextFreeSlot.dc++;
				}

				if (nextFreeSlot.dc >= ClusterSize_32b) {
					nextFreeSlot.dc = 0;
					nextFreeSlot.sli++;
					newSecondLevelIndexCluster = true;
				}
				if (nextFreeSlot.sli == ClusterSize_32b) throw std::exception();

				if (newSecondLevelIndexCluster) {
					ClusterNo cNo = bitVector.getFreeClusterNumberForUse();
					if (cNo == 0) throw std::exception();
					FLICluster.addSecondLevelIndexCluster(cNo);
					FLICluster.setDirty();
				}
				auto& SLICluster = FLICluster[nextFreeSlot.sli];
				if (newDataCluster) {
					ClusterNo cNo = bitVector.getFreeClusterNumberForUse();
					if (cNo == 0) throw std::exception();
					SLICluster.addDataCluster(cNo, false);
					SLICluster.setDirty();
				}
				auto& DataCluster = SLICluster[nextFreeSlot.dc];
				DataCluster.setDirty();
			}
			catch(std::exception&) {
				nextFreeSlot = old;
				return nullptr;
			}
		}
		ind = nextFreeSlot;
		nextFreeSlot.ridc++;
	}
	try {
		auto * fcbData = reinterpret_cast<FCB::FCBData*>(FLICluster[ind.sli][ind.dc].getData());
		memcpy(fcbData[ind.ridc].name, name.c_str(), FNAMELEN);
		memcpy(fcbData[ind.ridc].ext, ext.c_str(), FEXTLEN);
		fcbData[ind.ridc].fileSize = 0;
		fcbData[ind.ridc].firstIndexClusterNo = 0;
		FCB* fcb = new FCB(FCB::FCBIndex(ind.sli, ind.dc, ind.ridc), &fcbData[ind.ridc], part, bitVector, kernelFS, FLICluster[ind.sli][ind.dc]);
		nameToFCBmap[fpath] = fcb;
		return fcb;
	}
	catch (std::exception&) { std::cout << "Error: vector error at addNewFile()" << std::endl; return nullptr; }
	
}

void RootDirMemoryHandler::deleteFile(std::string& fpath) {
	if (nameToFCBmap.find(fpath) != nameToFCBmap.end()) {
		auto * fcb = nameToFCBmap[fpath];
		nameToFCBmap.erase(fpath);
		fcb->setFCBDataToFree();
		auto fcbIndex = fcb->getFCBIndex();
		FLICluster[fcbIndex.secondLvlIndex][fcbIndex.dataClusterIndex].setDirty();
		leftoverFreeFileSlots.push(FCBIndex(fcbIndex.secondLvlIndex, fcbIndex.dataClusterIndex, fcbIndex.rowInDataCluster));
		fcb->clearClusters();
		delete fcb;
	}
}

void RootDirMemoryHandler::saveToDrive() {
	FLICluster.saveToDrive();
	for (auto& elem : nameToFCBmap) {
		elem.second->saveToDrive();
	}
}

void RootDirMemoryHandler::format() {
	for (auto& elem : nameToFCBmap) {
		elem.second->clearClusters();
		delete elem.second; // fcb deallocation (first = string, second = fcb*)
	}
	nameToFCBmap.clear();
	FLICluster.format();
}

// void RootDirMemoryHandler::updateFile() TODO: WHEN SIZE CHANGES SO DOES THE ROOTDIRSLOT 






