#ifndef _KERNELFILE_H_
#define _KERNELFILE_H_
#include "FCB.h"
#include "DataCluster.h"

class KernelFile {
public:
	KernelFile(FCB * myFCB, char mode, FirstLevelIndexCluster * fli);

	KernelFile(const KernelFile&) = delete;
	KernelFile(KernelFile&&) = delete;
	KernelFile& operator=(const KernelFile&) = delete;
	KernelFile& operator=(KernelFile&&) = delete;

	~KernelFile();
	char write(BytesCnt cnt, char *buffer);
	BytesCnt read(BytesCnt cnt, char * buffer);
	char seek(BytesCnt newPos);
	BytesCnt filePos() { return currentPos; }
	char eof();
	BytesCnt getFileSize() { return currentSize; }
	char truncate();
	
private:

	typedef struct DataClusterWithReferenceBitStruct {
		DataCluster *dataCluster;
		int referenceCount = 0;
		DataClusterWithReferenceBitStruct(DataCluster* dc) : dataCluster(dc) {}
	} DataClusterWithReferenceBit;

	char * getDataFromCacheAndUpdateCache(BytesCnt dataClusterStartByte);

	FCB * myFCB;
	char mode;
	unsigned int currentSize;
	unsigned int currentPos;
	FirstLevelIndexCluster* fliCluster;
	std::map<BytesCnt, DataClusterWithReferenceBit*> byteCntToDataCluster;
	std::map<BytesCnt, DataClusterWithReferenceBit*> cache;

	static constexpr int cacheSize = 512;

};

#endif //_KERNELFILE_H_