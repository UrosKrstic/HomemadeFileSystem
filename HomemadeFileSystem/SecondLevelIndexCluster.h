#pragma once
#include "IndexCluster.h"
#include <vector>
#include "DataCluster.h"

class SecondLevelIndexCluster : public IndexCluster {
	std::vector<DataCluster*> dataClusters;
public:
	SecondLevelIndexCluster(ClusterNo clusterNumber, Partition * part, bool loadAllSubClusters = false, bool loadClusterData = true);

	SecondLevelIndexCluster(const SecondLevelIndexCluster&) = delete;
	SecondLevelIndexCluster(SecondLevelIndexCluster&&) = delete;
	SecondLevelIndexCluster& operator=(const SecondLevelIndexCluster&) = delete;
	SecondLevelIndexCluster& operator=(SecondLevelIndexCluster&&) = delete;

	~SecondLevelIndexCluster();
	std::vector<DataCluster*>& getDataClusters() { return dataClusters; }
	DataCluster& operator[](int i) { return *dataClusters[i]; }
	DataCluster& addDataCluster(ClusterNo cNo, bool loadClusterData = false, bool initClusterData = true);
	DataCluster * addCluster(ClusterNo cNo);
	void loadDataClusters(bool loadSubClusterData = false);
	void saveToDrive();
	void refreshIndexData();
	void unSetDirtyForAllDataClusters();
};

