#pragma once
#include "Cluster.h"
class DataCluster : public Cluster {
public:
	DataCluster(ClusterNo clusterNumber, Partition * part);
};

