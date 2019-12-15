#include "BitVector.h"
#include <iostream>

char BitVector::freeBitValue = 0;

BitVector::BitVector(ClusterNo clusterNumber, Partition * part) : Cluster(clusterNumber, part) {
	totalClusterNo = part->getNumOfClusters();
	//std::cout << "Napravljen bitVector\n";
}

ClusterNo BitVector::getFreeClusterNumberForUse() {
	//TODO: implement
	return 0;
}

void BitVector::freeUpClusters(std::vector<ClusterNo> clusterVector) {
	//TODO: implement
}

void BitVector::format() {
	std::fill_n(data, ClusterSize, freeBitValue);
	dirty = true;
}
