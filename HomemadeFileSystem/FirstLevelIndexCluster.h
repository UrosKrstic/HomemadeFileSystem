#pragma once
#include "IndexCluster.h"
#include <vector>
#include "SecondLevelIndexCluster.h"

class FirstLevelIndexCluster : public IndexCluster {
	
public:
	FirstLevelIndexCluster(ClusterNo clusterNumber, Partition * part, bool loadAllSubClusters = false, bool loadClusterData = true);
	std::vector<SecondLevelIndexCluster>& getSecondLevelIndexClusters() { return secondLvlClusters; }
	SecondLevelIndexCluster& operator[](int i) { return secondLvlClusters[i]; }
	SecondLevelIndexCluster& addSecondLevelIndexCluster(unsigned int cNo, bool loadAllSubClusters = false, bool loadClusterData = false);
	void saveToDrive();
	void format();
private:
	std::vector<SecondLevelIndexCluster> secondLvlClusters;
};

