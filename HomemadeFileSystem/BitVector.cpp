#include "BitVector.h"
#include <iostream>
#include "NoFreeClustersException.h"
#include "PartitionError.h"


BitVector::BitVector(Partition * part) : Cluster(bitVectorStartingCluster, part, false) {
	totalClusterNo = part->getNumOfClusters();
	totalBytes = totalClusterNo / 8;
	remainder = totalClusterNo % 8;
	sizeOfBitVector = static_cast<unsigned>(ceil(totalClusterNo / (ClusterSize * 8.)));
	data = new char[sizeOfBitVector * ClusterSize];
	auto ret = part->readCluster(bitVectorStartingCluster, data);
	if (ret == 0) throw PartitionError();
	for (unsigned cNo = 2; cNo < sizeOfBitVector + 1; cNo++) {
		ret = part->readCluster(cNo, data + ClusterSize * (cNo - 1));
		if (ret == 0) throw PartitionError();
	}
	//data[0] &= notAllowedBitMask;
	setBitVector();
	dirty = false;
}


void BitVector::setBitVector() {
	int bytes = sizeOfBitVector / 8;
	int bits = sizeOfBitVector % 8;
	memset(data, 0, bytes);
	if (bytes == 0) bits++;
	for (int i = 0; i < bits; i++) {
		data[bytes] &= ~(1 << (7 - i));
	}
}

ClusterNo BitVector::getFreeClusterNumberForUse() {
	unsigned bitMask = startingBitMask;
	for (unsigned i = 0; i < totalBytes; i++) {
		if (data[i] != 0) {
			for (unsigned j = 0; j < 8; j++) {
				if (data[i] & bitMask) {
					dirty = true;
					data[i] &= ~bitMask;
					return indexToClusterNo(i, j);
				}
				bitMask >>= 1;
			}
		}
		bitMask = startingBitMask;
	}
	bitMask = startingBitMask;
	for (unsigned i = 0; i < remainder; i++) {
		if (data[totalBytes] & bitMask) {
			dirty = true;
			data[totalBytes] &= ~bitMask;
			return indexToClusterNo(totalBytes, i);
		}
		bitMask >>= 1;
	}
	throw NoFreeClustersException();
}

void BitVector::freeUpClusters(std::vector<ClusterNo>& clusterVector) {
	dirty = true;
	for (unsigned long clusterNo : clusterVector) {
		data[clusterNo / 8] |= 1 << (7 - clusterNo % 8);
	}
}



void BitVector::format() {
	memset(data, 0xff, ClusterSize * sizeOfBitVector);
	//data[0] &= notAllowedBitMask;
	setBitVector();
	dirty = true;
}

void BitVector::saveToDrive() {
	EnterCriticalSection(&critSection);
	if (dirty) {
		auto ret = part->writeCluster(clusterNumber, data);
		if (ret == 0) {
			LeaveCriticalSection(&critSection);
			throw PartitionError();
		}
		for (unsigned cNo = 2; cNo < sizeOfBitVector + 1; cNo++) {
			ret = part->writeCluster(cNo, data + ClusterSize * (cNo - 1));
			if (ret == 0) {
				LeaveCriticalSection(&critSection);
				throw PartitionError();
			}
		}
		delete[] data;
		data = nullptr;
		dirty = false;
	}
	LeaveCriticalSection(&critSection);
}

