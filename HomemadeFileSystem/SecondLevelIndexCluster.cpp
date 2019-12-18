#include "SecondLevelIndexCluster.h"
#include <iostream>


SecondLevelIndexCluster::SecondLevelIndexCluster(ClusterNo clusterNumber, Partition * part, bool loadAllSubClusters, bool loadClusterData) : IndexCluster(clusterNumber, part, loadClusterData) {
	auto * data_32 = reinterpret_cast<unsigned int *> (data);
	if (loadAllSubClusters) {
		for (unsigned i = 0; i < currentSize; i++) {
			dataClusters.emplace_back(data_32[i], part, loadClusterData);
		}
	}
	//std::cout << "Napravljen FirstLvlIndex\n";
}

DataCluster & SecondLevelIndexCluster::addDataCluster(ClusterNo cNo, bool loadClusterData) {
	dirty = true;
	auto * data_32 = reinterpret_cast<unsigned int *> (data);
	data_32[currentSize++] = cNo;
	dataClusters.emplace_back(clusterNumber, part, loadClusterData);
	return dataClusters[currentSize - 1];
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
