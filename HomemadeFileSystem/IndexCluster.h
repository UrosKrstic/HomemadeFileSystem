#pragma once
#include "Cluster.h"
class IndexCluster : public Cluster {
protected:
	unsigned int currentSize;
public:
	IndexCluster(ClusterNo clusterNumber, Partition * part, bool loadClusterData = true);
	unsigned int getCurrentSize_32b() { return currentSize; }
	unsigned int getCurrentSize_8b() { return currentSize * sizeof(unsigned int); }
	
};

