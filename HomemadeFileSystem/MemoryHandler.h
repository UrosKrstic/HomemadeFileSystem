#ifndef _MEMORYHANDLER_H_
#define _MEMORYHANDLER_H_
#include "part.h"
#include "FirstLevelIndexCluster.h"
#include "BitVector.h"

class MemoryHandler {
public:
	MemoryHandler(BitVector& bVector, ClusterNo no, Partition * part, bool loadAllData) : bitVector(bVector), FLICluster(no, part, loadAllData), part(part) {}
protected:
	BitVector& bitVector;
	FirstLevelIndexCluster FLICluster;
	Partition * part;
};

#endif //_MEMORYHANDLER_H_

