#ifndef _KERNEL_FS_H_
#define _KERNEL_FS_H_
#include "part.h"
#include "fs.h"
#include "BitVector.h"
#include <map>
#include <Windows.h>

namespace kernel_fs {
	extern INIT_ONCE initOnceVariable;
	extern CRITICAL_SECTION fsLock;
	extern CONDITION_VARIABLE isMountedCond;
	extern CONDITION_VARIABLE openFilesCond;

	extern BOOL CALLBACK InitFunction(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *lpContext);
}


class FCB;
class RootDirMemoryHandler;

class KernelFS {
private:
	static bool partitionMounted;
	static ClusterNo bitVectorClusterNo;
	static ClusterNo rootDirFirstLevelIndexClusterNo;
	
	Partition * partition = nullptr;
	BitVector * bitVector = nullptr;
	RootDirMemoryHandler * rootDirMemoryHandler = nullptr;
	unsigned n = 100;
	std::map<std::string, FCB*>* fileNameToFCBmap = nullptr;
	bool areOpenFiles = false;
	bool formatingInProgress = false;
	int fileCount = 0;

public:
	KernelFS()=default;
	char mount(Partition * partition);
	char unmount();
	char format();
	char doesExist(std::string& fname);
	File * open(char * fname, char mode);
	char deleteFile(char * fname);
	bool openFiles() { return areOpenFiles; }
	bool isFormatingInProgress() const { return formatingInProgress; }
	void setFormatingInProgress(bool value) { formatingInProgress = value; }
	int getFileCount() const { return fileCount; }

	

	static bool isPartitionMounted() { return partitionMounted; }
};

#endif //_KERNEL_FS_H_
