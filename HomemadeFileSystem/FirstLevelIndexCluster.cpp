#include "FirstLevelIndexCluster.h"
#include <iostream>


FirstLevelIndexCluster::FirstLevelIndexCluster(ClusterNo clusterNumber, Partition * part, bool loadAllData) : IndexCluster(clusterNumber, part) {
	auto * data_32 = reinterpret_cast<unsigned int *> (data);
	for (unsigned i = 0; i < currentSize; i++) {
		secondLvlClusters.emplace_back(data_32[i], part, loadAllData);
	}
	//std::cout << "Napravljen FirstLvlIndex\n";
}

