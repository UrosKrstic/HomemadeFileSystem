#ifndef _BITVECTOR_H_
#define _BITVECTOR_H_
#include "Cluster.h"
#include <vector>

class BitVector : public Cluster {

public:
	BitVector(ClusterNo clusterNumber, Partition *part);
	ClusterNo  indexToClusterNo(int ind, int remain) { return ind * 8 + remain; }
	ClusterNo getFreeClusterNumberForUse(); //0 vraca ako ne uspe, pozitivan broj ako uspe
	void freeUpClusters(std::vector<ClusterNo>& clusterVector);
	void format();
private:
	unsigned int totalClusterNo;
	unsigned int totalBytes;
	unsigned int remainder;

	static constexpr unsigned int notAllowedBitMask = 0x3f; // 0011 1111 (nulti i prvi klaster se ne mogu koristiti nikad)
	static constexpr unsigned int startingBitMask = 0x80; // 1000 0000 (pocetna bit maska, za pretragu slobodnog klastera u bajtu, shiftovace se do 0x01)
};

#endif //_BITVECTOR_H_
