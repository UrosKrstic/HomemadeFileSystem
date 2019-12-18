#include "Cluster.h"
#include "PartitionError.h"

Cluster::Cluster(ClusterNo clusterNumber, Partition * part, bool loadClusterData) {
	this->clusterNumber = clusterNumber;
	this->part = part;
	if (loadClusterData) {
		auto ret = part->readCluster(clusterNumber, data);
		if (ret == 0) throw PartitionError();
		dirty = false;
	}
}

void Cluster::saveToDrive() {
	if (dirty) {
		auto ret = part->writeCluster(clusterNumber, data);
		if (ret == 0) throw PartitionError();
	}
}
