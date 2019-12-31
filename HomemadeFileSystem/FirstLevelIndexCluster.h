#pragma once
#include "IndexCluster.h"
#include <vector>
#include "SecondLevelIndexCluster.h"

class FirstLevelIndexCluster : public IndexCluster {
	
public:
	FirstLevelIndexCluster(ClusterNo clusterNumber, Partition * part, bool loadAllSubClusters = false, bool loadClusterData = true);

	FirstLevelIndexCluster(const FirstLevelIndexCluster&) = delete;
	FirstLevelIndexCluster(FirstLevelIndexCluster&&) = delete;
	FirstLevelIndexCluster& operator=(const FirstLevelIndexCluster&) = delete;
	FirstLevelIndexCluster& operator=(FirstLevelIndexCluster&&) = delete;

	~FirstLevelIndexCluster();
	std::vector<SecondLevelIndexCluster*>& getSecondLevelIndexClusters() { return secondLvlClusters; }
	SecondLevelIndexCluster& operator[](int i) { return *secondLvlClusters[i]; }
	SecondLevelIndexCluster& addSecondLevelIndexCluster(unsigned int cNo, bool loadAllSubClusters = false, bool loadClusterData = false);
	void loadSLIClusters();
	void saveToDrive();
	void format();
	static unsigned long getMaxFileSize() { return fileMaxSize; }
private:
	std::vector<SecondLevelIndexCluster*> secondLvlClusters;
	static constexpr unsigned long fileMaxSize = ClusterSize / 4 * ClusterSize / 4 * ClusterSize;
};

