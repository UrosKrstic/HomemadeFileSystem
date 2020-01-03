#ifndef _DATA_CLUSTER_H_
#define _DATA_CLUSTER_H_
#include "Cluster.h"
class DataCluster : public Cluster {
public:
	DataCluster(ClusterNo clusterNumber, Partition * part, bool loadClusterData = true);
	void unSetDirty() { EnterCriticalSection(&critSection); dirty = false; LeaveCriticalSection(&critSection); }
	void addReference() { EnterCriticalSection(&critSection); referenceCount++; LeaveCriticalSection(&critSection); }
	unsigned getReferenceCount() { return referenceCount; }
	void resetReferenceCount() { EnterCriticalSection(&critSection); referenceCount = 0; LeaveCriticalSection(&critSection); }
private:
	unsigned referenceCount = 0;
};

#endif // _DATA_CLUSTER_H_