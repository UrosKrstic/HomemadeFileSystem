#ifndef _FCB_H_
#define _FCB_H_
#include "KernelFS.h"

struct FCBData;
class KernelFile;

class FCB {
private:
	ClusterNo secondLvlIndex;
	ClusterNo dataClusterIndex;
	ClusterNo rowInDataCluster;
	char name[FNAMELEN];
	char ext[FEXTLEN];
	unsigned fileSize;
	unsigned numberOfOpenFiles = 0;
	Partition * part;
	FirstLevelIndexCluster * FLICluster = nullptr;
public:
	FCB(unsigned secondLvlIndex, unsigned dataClusterIndex, unsigned rowInDataCluster, KernelFS::FCBData& fcbData, Partition * part);
	KernelFile * createKernelFile(char mode);
};
#endif //_FCB_H_
