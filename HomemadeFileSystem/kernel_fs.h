#ifndef _KERNEL_FS_H_
#define _KERNEL_FS_H_
#include "part.h"
#include  <Windows.h>

namespace kernel_fs {
	extern INIT_ONCE initOnceVariable;
	extern CRITICAL_SECTION fsLock;
	extern CONDITION_VARIABLE isMountedCond;
	extern CONDITION_VARIABLE openFilesCond;

	extern BOOL CALLBACK InitFunction(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *lpContext);
}



class KernelFS {
private:
	static bool partitionMounted;
	static ClusterNo bitVectorClusterNo;
	static ClusterNo rootDirFirstLevelIndexClusterNo;
	
	Partition * partition = nullptr;
	char bitVector[ClusterSize]; //1 - zauzet, 0 - slobodan
	char firstLvlIndex[ClusterSize];
	bool areOpenFiles = false;
	bool formatingInProgress = false;
	int fileCount = 0;

public:

	KernelFS()=default;
	char mount(Partition * partition);
	char unmount();
	char format();
	bool openFiles() { return areOpenFiles; }
	bool isFormatingInProgress() const { return formatingInProgress; }
	void setFormatingInProgress(bool value) { formatingInProgress = value; }
	int getFileCount() const { return fileCount; }
	

	static bool isPartitionMounted() { return partitionMounted; }
};

#endif //_KERNEL_FS_H_
