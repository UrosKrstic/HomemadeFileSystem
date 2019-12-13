#ifndef _KERNEL_FS_H_
#define _KERNEL_FS_H_
#include "part.h"
#include "fs.h"
#include  <Windows.h>
#include <cstdint>

INIT_ONCE initOnceVariable = INIT_ONCE_STATIC_INIT;
CRITICAL_SECTION fsLock;
CONDITION_VARIABLE isMountedCond;
CONDITION_VARIABLE openFilesCond;

BOOL CALLBACK CondVarInitFunction(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *lpContext);


class KernelFS {
private:
	static bool partitionMounted;
	
	Partition * partition = nullptr;
	uint8_t bitVector[ClusterSize] = {0}; //1 - zauzet, 0 - slobodan
	bool areOpenFiles;
	bool formatingInProgress = false;

public:

	KernelFS()=default;
	char mount(Partition * partition);
	char unmount();
	char format();
	bool openFiles() { return areOpenFiles; }
	bool isFormatingInProgress() const { return formatingInProgress; }
	void setFormatingInProgress(bool value) { formatingInProgress = value; }
	

	static bool isPartitionMounted() { return partitionMounted; }


};

bool KernelFS::partitionMounted = false;

#endif //_KERNEL_FS_H_
