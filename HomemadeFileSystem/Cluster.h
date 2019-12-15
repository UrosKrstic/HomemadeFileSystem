#ifndef _CLUSTER_H_
#define _CLUSTER_H_
#include "part.h"

class Cluster {
protected:
	ClusterNo clusterNumber;
	char data[ClusterSize];
	unsigned int ClusterSizeInt = ClusterSize / sizeof(unsigned int);
	bool dirty;
	Partition * part;
public:
	Cluster(ClusterNo clusterNumber, Partition * part);
	virtual ~Cluster();
	char* getData() { return data; }
	bool isDirty() const { return dirty; }
	void setDirty() { dirty = true; }
};

#endif //_CLUSTER_H_
