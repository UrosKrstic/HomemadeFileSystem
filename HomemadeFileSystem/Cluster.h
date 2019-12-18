#ifndef _CLUSTER_H_
#define _CLUSTER_H_
#include "part.h"

class Cluster {
protected:
	ClusterNo clusterNumber;
	char data[ClusterSize] = {0};
	unsigned int ClusterSizeInt = ClusterSize / sizeof(unsigned int);
	bool dirty = false;
	Partition * part;
public:
	Cluster(ClusterNo clusterNumber, Partition * part, bool loadClusterData = true);
	char* getData() { return data; }
	bool isDirty() const { return dirty; }
	void setDirty() { dirty = true; }
	void saveToDrive();
};

#endif //_CLUSTER_H_
