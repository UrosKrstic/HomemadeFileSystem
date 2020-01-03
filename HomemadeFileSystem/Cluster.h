#ifndef _CLUSTER_H_
#define _CLUSTER_H_
#include "part.h"
#include <Windows.h>

class KernelFile;

class Cluster {
protected:
	ClusterNo clusterNumber;
	char * data = nullptr;
	unsigned int ClusterSizeInt = ClusterSize / sizeof(unsigned int);
	bool dirty = true;
	Partition * part;
	CRITICAL_SECTION critSection;
public:
	Cluster(ClusterNo clusterNumber, Partition * part, bool loadClusterData = true);

	Cluster(const Cluster&) = delete;
	Cluster(Cluster&&) = delete;
	Cluster& operator=(const Cluster&) = delete;
	Cluster& operator=(Cluster&&) = delete;

	~Cluster() { delete[] data; DeleteCriticalSection(&critSection); }
	ClusterNo getClusterNumber() { return clusterNumber; }
	void initDataWithZeros();
	char* loadData();
	char* getData() { return data; }
	bool isDirty() const { return dirty; }
	void setDirty() { EnterCriticalSection(&critSection); dirty = true; LeaveCriticalSection(&critSection); }
	void saveToDrive();
};

#endif //_CLUSTER_H_
