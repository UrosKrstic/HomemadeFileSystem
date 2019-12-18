#include "DataCluster.h"
#include <iostream>

DataCluster::DataCluster(ClusterNo clusterNumber, Partition * part, bool loadClusterData) : Cluster(clusterNumber, part, loadClusterData) {
	//std::cout << "Napravljen FirstLvlIndex\n";
}

