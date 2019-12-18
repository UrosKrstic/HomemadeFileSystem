#include "RootDirMemoryHandler.h"
#include "FCB.h"
#include <iostream>

std::map<std::string, FCB*>* RootDirMemoryHandler::getNameToFCBMap() {
	if (!isMapCreated) {
		unsigned long i = 0, j = 0, k = 0;
		for (; i < FLICluster.getCurrentSize_32b(); i++) {
			for (; j < FLICluster[i].getCurrentSize_32b(); j++) {
				auto *fcbData = reinterpret_cast<FCB::FCBData*>(FLICluster[i][j].getData());
				for (; k < rowCountInDataCluster; k++) {
					if (fcbData[k].allZeros()) break;
					else if (!fcbData[k].isEmpty()) {
						std::string fullName = std::string("/") + std::string(fcbData[k].name);
						fullName.erase(fullName.find_first_of(std::string(" ")));
						std::string ext(std::string(".") + std::string(fcbData[k].ext));
						ext.resize(4);
						fullName += ext;
						nameToFCBmap[fullName] = new FCB(FCB::FCBIndex(i, j, k), &fcbData[k], part);
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
	std::string name = fpath.substr(1, fpath.size() - FEXTLEN - 1);
	std::cout << name.c_str() << " " << ext.c_str() << std::endl;
	FCBIndex ind;
	if (!leftoverFreeFileSlots.empty()) {
		std::cout << "Ima leftover spot\n";
		auto fcbIndex = leftoverFreeFileSlots.front();
		ind = fcbIndex;
		leftoverFreeFileSlots.pop();
		std::cout << "Spot: " << fcbIndex.sli << " " << fcbIndex.dc << " " << fcbIndex.ridc << std::endl; 
		FLICluster[fcbIndex.sli][fcbIndex.dc].setDirty();
		/*auto* fcbData = reinterpret_cast<FCB::FCBData*>(FLICluster[fcbIndex.sli][fcbIndex.dc].getData());
		memcpy(fcbData[fcbIndex.ridc].name, name.c_str(), FNAMELEN);
		memcpy(fcbData[fcbIndex.ridc].ext, ext.c_str(), FEXTLEN);
		fcbData[fcbIndex.ridc].fileSize = 0;
		fcbData[fcbIndex.ridc].firstIndexClusterNo = 0;
		return new FCB(FCB::FCBIndex(fcbIndex.sli, fcbIndex.dc, fcbIndex.ridc), &fcbData[fcbIndex.ridc], part);*/
	}
	else {
		if (nextFreeSlot.sli == 0 && nextFreeSlot.dc == 0 && nextFreeSlot.ridc == 0) {
			ClusterNo cNo = bitVector.getFreeClusterNumberForUse();
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
			bool newDataCluster = false;
			if (nextFreeSlot.ridc >= rowCountInDataCluster) {
				newDataCluster = true;
				nextFreeSlot.ridc = 0;
				nextFreeSlot.dc++;
			}
			else nextFreeSlot.ridc++;

			bool newSecondLevelIndexCluster = false;
			if (nextFreeSlot.dc >= ClusterSize_32b) {
				newSecondLevelIndexCluster = true;
				nextFreeSlot.dc = 0;
				nextFreeSlot.sli++;
			}
			if (nextFreeSlot.sli == ClusterSize_32b) return nullptr;

			if (newSecondLevelIndexCluster) {
				ClusterNo cNo = bitVector.getFreeClusterNumberForUse();
				if (cNo == 0) return nullptr;
				FLICluster.addSecondLevelIndexCluster(cNo);
				FLICluster.setDirty();
			}
			auto& SLICluster = FLICluster[nextFreeSlot.sli];
			if (newDataCluster) {
				ClusterNo cNo = bitVector.getFreeClusterNumberForUse();
				if (cNo == 0) return nullptr;
				SLICluster.addDataCluster(cNo, false);
				SLICluster.setDirty();
			}
			auto& DataCluster = SLICluster[nextFreeSlot.dc];
			DataCluster.setDirty();
		}
		ind = nextFreeSlot;
	}
	auto * fcbData = reinterpret_cast<FCB::FCBData*>(FLICluster[ind.sli][ind.dc].getData());
	memcpy(fcbData[ind.ridc].name, name.c_str(), FNAMELEN);
	memcpy(fcbData[ind.ridc].ext, ext.c_str(), FEXTLEN);
	fcbData[ind.ridc].fileSize = 0;
	fcbData[ind.ridc].firstIndexClusterNo = 0;
	return new FCB(FCB::FCBIndex(ind.sli, ind.dc, ind.ridc), &fcbData[ind.ridc], part);
}

void RootDirMemoryHandler::deleteFile(std::string& fpath) {
	auto * fcb = nameToFCBmap[fpath];
	fcb->setFCBDataToFree();
	auto fcbIndex = fcb->getFCBIndex();
	FLICluster[fcbIndex.secondLvlIndex][fcbIndex.dataClusterIndex].setDirty();
	leftoverFreeFileSlots.push(FCBIndex(fcbIndex.secondLvlIndex, fcbIndex.dataClusterIndex, fcbIndex.rowInDataCluster));
}

void RootDirMemoryHandler::saveToDrive() {
	FLICluster.saveToDrive();
}

void RootDirMemoryHandler::format() {
	FLICluster.format();
}

// void RootDirMemoryHandler::updateFile() TODO: WHEN SIZE CHANGES SO DOES THE ROOTDIRSLOT 






