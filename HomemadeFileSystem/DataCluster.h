#pragma once
#include "Cluster.h"
class DataCluster : public Cluster {
public:
	DataCluster(ClusterNo clusterNumber, Partition * part, bool loadClusterData = true);
	void unSetDirty() {  dirty = false; }
};

