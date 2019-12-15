#include "Cluster.h"

Cluster::Cluster(ClusterNo clusterNumber, Partition * part) {
	this->clusterNumber = clusterNumber;
	this->part = part;
	part->readCluster(clusterNumber, data);
	dirty = false;
}

Cluster::~Cluster() {
	if (dirty) part->writeCluster(clusterNumber, data);
}
