#include "FirstLevelIndexCluster.h"
#include <iostream>


FirstLevelIndexCluster::FirstLevelIndexCluster(ClusterNo clusterNumber, Partition * part, bool loadAllSubClusters, bool loadClusterData) : IndexCluster(clusterNumber, part, loadClusterData) {
	auto * data_32 = reinterpret_cast<unsigned int *> (data);
	std::cout << "Pravi se FLI Cluster, podaci mu se ucitavaju: " << loadClusterData << std::endl;
	if (loadAllSubClusters) {
		for (unsigned i = 0; i < currentSize; i++) {
			secondLvlClusters.emplace_back(data_32[i], part, loadAllSubClusters, loadClusterData);//loads SLI sub clusters
		}
	}
	//std::cout << "Napravljen FirstLvlIndex\n";
}

SecondLevelIndexCluster & FirstLevelIndexCluster::addSecondLevelIndexCluster(ClusterNo cNo, bool loadAllSubClusters, bool loadClusterData) {
	dirty = true;
	auto * data_32 = reinterpret_cast<unsigned int *> (data);
	data_32[currentSize++] = cNo;
	secondLvlClusters.emplace_back(clusterNumber, part, loadAllSubClusters, loadClusterData);
	return secondLvlClusters[currentSize - 1];
}

void FirstLevelIndexCluster::saveToDrive() {
	IndexCluster::saveToDrive();
	for (auto& sliCluster : secondLvlClusters) {
		sliCluster.saveToDrive();
	}
}

void FirstLevelIndexCluster::format() {
	std::fill_n(data, ClusterSize, 0);
	setDirty();
	for (auto& sliCluster : secondLvlClusters) {
		std::fill_n(sliCluster.getData(), ClusterSize, 0);
		sliCluster.setDirty();
		sliCluster.unSetDirtyForAllDataClusters();
	}
}

