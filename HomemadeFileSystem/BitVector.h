#ifndef _BITVECTOR_H_
#define _BITVECTOR_H_
#include "Cluster.h"
#include <vector>

class BitVector : public Cluster {
private:
	unsigned int totalClusterNo;
	static char freeBitValue;
public:
	BitVector(ClusterNo clusterNumber, Partition *part);
	ClusterNo getFreeClusterNumberForUse(); //0 vraca ako ne uspe, pozitivan broj ako uspe
	void freeUpClusters(std::vector<ClusterNo> clusterVector);
	void format();
};

#endif //_BITVECTOR_H_
