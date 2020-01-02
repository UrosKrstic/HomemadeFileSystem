#ifndef _DATA_CLUSTER_H_
#define _DATA_CLUSTER_H_
#include "Cluster.h"
class DataCluster : public Cluster {
public:
	DataCluster(ClusterNo clusterNumber, Partition * part, bool loadClusterData = true);
	void unSetDirty() {  dirty = false; }
};

#endif // _DATA_CLUSTER_H_