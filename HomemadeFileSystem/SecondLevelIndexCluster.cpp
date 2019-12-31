#include "SecondLevelIndexCluster.h"
#include <iostream>
#include "ClusterFullException.h"


SecondLevelIndexCluster::SecondLevelIndexCluster(ClusterNo clusterNumber, Partition * part, bool loadAllSubClusters, bool loadClusterData) : IndexCluster(clusterNumber, part, loadClusterData) {
	auto * data_32 = reinterpret_cast<unsigned int *> (data);
	if (loadAllSubClusters) {
		for (unsigned i = 0; i < currentSize; i++) {
			dataClusters.push_back(new DataCluster(data_32[i], part, loadClusterData));
		}
	}
	//std::cout << "Napravljen FirstLvlIndex\n";
}

SecondLevelIndexCluster::~SecondLevelIndexCluster() {
	for (auto& dc: dataClusters) {
		delete dc;
	}
}

DataCluster & SecondLevelIndexCluster::addDataCluster(ClusterNo cNo, bool loadClusterData, bool initClusterData) {
	if (currentSize == ClusterSizeInt) throw ClusterFullException();
	dirty = true;
	auto * data_32 = reinterpret_cast<unsigned int *> (data);
	data_32[currentSize++] = cNo;
	dataClusters.push_back(new DataCluster(cNo, part, loadClusterData));
	if (initClusterData) dataClusters[currentSize - 1]->initDataWithZeros();
	return *dataClusters[currentSize - 1];
}

DataCluster * SecondLevelIndexCluster::addCluster(ClusterNo cNo) {
	if (currentSize == ClusterSizeInt) throw ClusterFullException();
	dirty = true;
	auto * data_32 = reinterpret_cast<unsigned int *> (data);
	data_32[currentSize++] = cNo;
	dataClusters.push_back(new DataCluster(cNo, part, false));
	return dataClusters[currentSize - 1];
}

void SecondLevelIndexCluster::loadDataClusters(bool loadSubClusterData) {
	auto * data_32 = reinterpret_cast<unsigned int *> (data);
	for (unsigned i = 0; i < currentSize; i++) {
		dataClusters.push_back(new DataCluster(data_32[i], part, false));
	}
}

void SecondLevelIndexCluster::saveToDrive() {
	IndexCluster::saveToDrive();
	for (auto& dataCluster: dataClusters) {
		dataCluster->saveToDrive();
	}
}

void SecondLevelIndexCluster::unSetDirtyForAllDataClusters() {
	for (auto& dataCluster : dataClusters) {
		dataCluster->unSetDirty();
	}
}
