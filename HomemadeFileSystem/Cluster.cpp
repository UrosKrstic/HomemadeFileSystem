#include "Cluster.h"
#include "PartitionError.h"

Cluster::Cluster(ClusterNo clusterNumber, Partition * part, bool loadClusterData) {
	this->clusterNumber = clusterNumber;
	this->part = part;
	if (loadClusterData) {
		data = new char[ClusterSize];
		auto ret = part->readCluster(clusterNumber, data);
		if (ret == 0) throw PartitionError();
		dirty = false;
	}
}

void Cluster::initDataWithZeros() {
	dirty = true;
	if (data == nullptr) data = new char[ClusterSize];
	memset(data, 0, ClusterSize);
}

char * Cluster::loadData() {
	if (data == nullptr) {
		data = new char[ClusterSize];
		auto ret = part->readCluster(clusterNumber, data);
		if (ret == 0) throw PartitionError();
		dirty = false;
	}
	return data;
}

void Cluster::saveToDrive() {
	if (dirty && data != nullptr) {
		auto ret = part->writeCluster(clusterNumber, data);
		if (ret == 0) throw PartitionError();
		delete[] data;
		data = nullptr;
		dirty = false;
	}
}
