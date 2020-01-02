#include "Cluster.h"
#include "PartitionError.h"

Cluster::Cluster(ClusterNo clusterNumber, Partition * part, bool loadClusterData) {
	InitializeCriticalSection(&critSection);
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
	EnterCriticalSection(&critSection);
	dirty = true;
	if (data == nullptr) data = new char[ClusterSize];
	memset(data, 0, ClusterSize);
	LeaveCriticalSection(&critSection);
}

char * Cluster::loadData() {
	EnterCriticalSection(&critSection);
	if (data == nullptr) {
		data = new char[ClusterSize];
		auto ret = part->readCluster(clusterNumber, data);
		if (ret == 0) {
			LeaveCriticalSection(&critSection);
			throw PartitionError();
		}
		dirty = false;
	}
	LeaveCriticalSection(&critSection);
	return data;
}

void Cluster::saveToDrive() {
	EnterCriticalSection(&critSection);
	if (dirty && data != nullptr) {
		auto ret = part->writeCluster(clusterNumber, data);
		if (ret == 0) {
			LeaveCriticalSection(&critSection);
			throw PartitionError();
		}
		delete[] data;
		data = nullptr;
		dirty = false;
	}
	LeaveCriticalSection(&critSection);
}
