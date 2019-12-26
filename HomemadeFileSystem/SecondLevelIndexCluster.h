#pragma once
#include "IndexCluster.h"
#include <vector>
#include "DataCluster.h"

class SecondLevelIndexCluster : public IndexCluster {
	std::vector<DataCluster> dataClusters;
public:
	SecondLevelIndexCluster(ClusterNo clusterNumber, Partition * part, bool loadAllSubClusters = false, bool loadClusterData = true);
	std::vector<DataCluster>& getDataClusters() { return dataClusters; }
	DataCluster& operator[](int i) { return dataClusters[i]; }
	DataCluster& addDataCluster(ClusterNo cNo, bool loadClusterData = false, bool initClusterData = true);
	void loadDataClusters(bool loadSubClusterData = false);
	void saveToDrive();
	void unSetDirtyForAllDataClusters();
};

