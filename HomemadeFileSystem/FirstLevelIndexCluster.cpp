#include "FirstLevelIndexCluster.h"
#include <iostream>
#include "ClusterFullException.h"


FirstLevelIndexCluster::FirstLevelIndexCluster(ClusterNo clusterNumber, Partition * part, bool loadAllSubClusters, bool loadClusterData) : IndexCluster(clusterNumber, part, loadClusterData) {
	auto * data_32 = reinterpret_cast<unsigned int *> (data);
	//std::cout << "Pravi se FLI Cluster, podaci mu se ucitavaju: " << loadClusterData << std::endl;
	if (loadAllSubClusters) {
		for (unsigned i = 0; i < currentSize; i++) {
			secondLvlClusters.push_back(new SecondLevelIndexCluster(data_32[i], part, loadAllSubClusters, loadClusterData));//loads SLI sub clusters
		}
	}
	//std::cout << "Napravljen FirstLvlIndex\n";
}

FirstLevelIndexCluster::~FirstLevelIndexCluster() {
	for (auto& sli: secondLvlClusters) {
		delete sli;
	}
}

SecondLevelIndexCluster & FirstLevelIndexCluster::addSecondLevelIndexCluster(unsigned int cNo, bool loadAllSubClusters, bool loadClusterData) {
	if (currentSize == ClusterSizeInt) throw ClusterFullException();
	dirty = true;
	auto * data_32 = reinterpret_cast<unsigned int *> (data);
	data_32[currentSize++] = cNo;
	secondLvlClusters.push_back(new SecondLevelIndexCluster(cNo, part, loadAllSubClusters, loadClusterData));
	secondLvlClusters[currentSize - 1]->initDataWithZeros();
	return *secondLvlClusters[currentSize - 1];
}

void FirstLevelIndexCluster::loadSLIClusters() {
	auto * data_32 = reinterpret_cast<unsigned int *> (data);
	for (unsigned i = 0; i < currentSize; i++) {
		auto* sli = new SecondLevelIndexCluster(data_32[i], part, false, true);
		sli->loadDataClusters();
		secondLvlClusters.push_back(sli);
	}
}

void FirstLevelIndexCluster::saveToDrive() {
	IndexCluster::saveToDrive();
	for (auto& sliCluster : secondLvlClusters) {
		sliCluster->saveToDrive();
	}
}

void FirstLevelIndexCluster::format() {
	//std::fill_n(data, ClusterSize, 0);
	memset(data, 0, ClusterSize);
	setDirty();
	secondLvlClusters.clear();
	currentSize = 0;
}

void FirstLevelIndexCluster::refreshIndexData() {
	dirty = true;
	currentSize = secondLvlClusters.size();
	memset(data + secondLvlClusters.size() * 4, 0, ClusterSize - secondLvlClusters.size() * 4);
}

