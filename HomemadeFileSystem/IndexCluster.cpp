#include "IndexCluster.h"

IndexCluster::IndexCluster(ClusterNo clusterNumber, Partition * part, bool loadClusterData) : Cluster(clusterNumber, part, loadClusterData) {
	auto* data_32b = reinterpret_cast<unsigned int *>(data);
	currentSize = 0;
	if (loadClusterData)
		while (currentSize < ClusterSizeInt && data_32b[currentSize] != 0) currentSize++;
}

