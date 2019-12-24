#include "SecondLevelIndexCluster.h"
#include <iostream>
#include "ClusterFullException.h"


SecondLevelIndexCluster::SecondLevelIndexCluster(ClusterNo clusterNumber, Partition * part, bool loadAllSubClusters, bool loadClusterData) : IndexCluster(clusterNumber, part, loadClusterData) {
	auto * data_32 = reinterpret_cast<unsigned int *> (data);
	if (loadAllSubClusters) {
		for (unsigned i = 0; i < currentSize; i++) {
			dataClusters.emplace_back(data_32[i], part, loadClusterData);
		}
	}
	//std::cout << "Napravljen FirstLvlIndex\n";
}

DataCluster & SecondLevelIndexCluster::addDataCluster(ClusterNo cNo, bool loadClusterData, bool initClusterData) {
	if (currentSize == ClusterSizeInt) throw ClusterFullException();
	dirty = true;
	auto * data_32 = reinterpret_cast<unsigned int *> (data);
	data_32[currentSize++] = cNo;
	DataCluster dc(cNo, part, loadClusterData);
	if (initClusterData)
		dc.initDataWithZeros();
	dataClusters.emplace_back(dc);
	return dataClusters[currentSize - 1];
}

void SecondLevelIndexCluster::loadDataClusters(bool loadSubClusterData) {
	auto * data_32 = reinterpret_cast<unsigned int *> (data);
	for (unsigned i = 0; i < currentSize; i++) {
		dataClusters.emplace_back(data_32[i], part, loadSubClusterData);
	}
}

void SecondLevelIndexCluster::saveToDrive() {
	IndexCluster::saveToDrive();
	for (auto& dataCluster: dataClusters) {
		dataCluster.saveToDrive();
	}
}

void SecondLevelIndexCluster::unSetDirtyForAllDataClusters() {
	for (auto& dataCluster : dataClusters) {
		dataCluster.unSetDirty();
	}
}
