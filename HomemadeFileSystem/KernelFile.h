#ifndef _KERNELFILE_H_
#define _KERNELFILE_H_
#include "FCB.h"
#include "DataCluster.h"
#include <queue>

class KernelFile {
public:
	KernelFile(FCB * myFCB, char mode, FirstLevelIndexCluster * fli);
	~KernelFile();
	char write(BytesCnt cnt, char *buffer);
	BytesCnt read(BytesCnt cnt, char * buffer);
	char seek(BytesCnt newPos);
	BytesCnt filePos() { return currentPos; }
	char eof() {
		return currentPos < currentSize ? 0 : 2;
	}
	BytesCnt getFileSize() { return currentSize; }
	char truncate();
	
private:

	struct dataClusterWithReferenceBit {
		DataCluster *dataCluster;
		bool isReferenced;
	};

	FCB * myFCB;
	char mode;
	unsigned int currentSize;
	unsigned int currentPos;
	FirstLevelIndexCluster* fliCluster;
	std::map<BytesCnt, DataCluster*> byteCntToDataCluster;
	std::queue<dataClusterWithReferenceBit> SecondChanceCache;

};

#endif //_KERNELFILE_H_