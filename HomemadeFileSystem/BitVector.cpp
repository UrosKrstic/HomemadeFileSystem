#include "BitVector.h"
#include <iostream>


BitVector::BitVector(ClusterNo clusterNumber, Partition * part) : Cluster(clusterNumber, part) {
	totalClusterNo = part->getNumOfClusters();
	totalBytes = totalClusterNo / 8;
	remainder = totalClusterNo % 8;
	data[0] &= notAllowedBitMask;
}

ClusterNo BitVector::getFreeClusterNumberForUse() {
	unsigned bitMask = startingBitMask;
	for (unsigned i = 0; i < totalBytes; i++) {
		if (data[i] != 0) {
			for (unsigned j = 0; j < 8; j++) {
				if (data[i] & bitMask) {
					dirty = true;
					return indexToClusterNo(i, j);
				}
				bitMask >>= 1;
			}
		}
		bitMask = startingBitMask;
	}
	bitMask = startingBitMask;
	for (unsigned i = 0; i < remainder; i++) {
		if (data[totalBytes] * bitMask) {
			dirty = true;
			return indexToClusterNo(totalBytes, i);
		}
		bitMask >>= 1;
	}
	return 0;
}

void BitVector::freeUpClusters(std::vector<ClusterNo>& clusterVector) {
	dirty = true;
	for (unsigned long clusterNo : clusterVector) {
		data[clusterNo / 8] |= 1 << clusterNo % 8;
	}
}

void BitVector::format() {
	std::fill_n(data, ClusterSize, 1);
	data[0] &= notAllowedBitMask;
	dirty = true;
}
