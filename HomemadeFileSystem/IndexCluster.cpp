#include "IndexCluster.h"

IndexCluster::IndexCluster(ClusterNo clusterNumber, Partition * part) : Cluster(clusterNumber, part) {
	auto* data_32b = reinterpret_cast<unsigned int *>(data);
	currentSize = 0;
	while (currentSize < ClusterSizeInt && data_32b[currentSize] != 0) currentSize++;
}

