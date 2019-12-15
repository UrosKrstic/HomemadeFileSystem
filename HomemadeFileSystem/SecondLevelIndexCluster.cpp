#include "SecondLevelIndexCluster.h"
#include <iostream>


SecondLevelIndexCluster::SecondLevelIndexCluster(ClusterNo clusterNumber, Partition * part, bool loadAllData) : IndexCluster(clusterNumber, part) {
	auto * data_32 = reinterpret_cast<unsigned int *> (data);
	if (loadAllData) {
		for (unsigned i = 0; i < currentSize; i++) {
			dataClusters.emplace_back(data_32[i], part);
		}
	}
	//std::cout << "Napravljen FirstLvlIndex\n";
}
