#pragma once
#include "IndexCluster.h"
#include <vector>
#include "SecondLevelIndexCluster.h"

class FirstLevelIndexCluster : public IndexCluster {
private:
	std::vector<SecondLevelIndexCluster> secondLvlClusters;
public:
	FirstLevelIndexCluster(ClusterNo clusterNumber, Partition * part, bool loadAllData = false);
	std::vector<SecondLevelIndexCluster>& getSecondLevelIndexClusters() { return secondLvlClusters; }
	SecondLevelIndexCluster& operator[](int i) { return secondLvlClusters[i]; }
};

