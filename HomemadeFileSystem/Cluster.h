#ifndef _CLUSTER_H_
#define _CLUSTER_H_
#include "part.h"
#include <cstring>

class Cluster {
protected:
	ClusterNo clusterNumber;
	char * data = nullptr;
	unsigned int ClusterSizeInt = ClusterSize / sizeof(unsigned int);
	bool dirty = true;
	Partition * part;
public:
	Cluster(ClusterNo clusterNumber, Partition * part, bool loadClusterData = true);
	void initDataWithZeros();
	char* loadData();
	char* getData() { return data; }
	bool isDirty() const { return dirty; }
	void setDirty() { dirty = true; }
	void saveToDrive();
};

#endif //_CLUSTER_H_
