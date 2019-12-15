#ifndef _KERNEL_FS_H_
#define _KERNEL_FS_H_
#include "part.h"
#include "fs.h"
#include "BitVector.h"
#include "FirstLevelIndexCluster.h"
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

class KernelFS {
private:
	static bool partitionMounted;
	static ClusterNo bitVectorClusterNo;
	static ClusterNo rootDirFirstLevelIndexClusterNo;
	static constexpr unsigned numOfFreeBytes = 12;


	Partition * partition = nullptr;
	BitVector * bitVector;
	FirstLevelIndexCluster * rootDirFirstLvlIndex;
	std::map<std::string, FCB*> fileNameToFCBmap;
	bool areOpenFiles = false;
	bool formatingInProgress = false;
	int fileCount = 0;

public:

	typedef struct FCBDataStruct {
		char name[FNAMELEN];
		char ext[FEXTLEN];
		char freeByte;
		unsigned firstIndexClusterNo;
		unsigned fileSize;
		char freeBytes[numOfFreeBytes];
		bool allZeros() {
			char * data = reinterpret_cast<char*>(this);
			for (int i = 0; i < 20; i++)
				if (data[i] != static_cast<char>(0)) return false;
			return true;
		}
		bool isEmpty() {
			for (int i = 0; i < FNAMELEN; i++)
				if (name[i] != static_cast<char>(0)) return false;
			return true;
		}
	} FCBData;

	KernelFS()=default;
	char mount(Partition * partition);
	char unmount();
	char format();
	char doesExist(std::string fname);
	File * open(char * fname, char mode);
	char deleteFile(char * fname);
	bool openFiles() { return areOpenFiles; }
	bool isFormatingInProgress() const { return formatingInProgress; }
	void setFormatingInProgress(bool value) { formatingInProgress = value; }
	int getFileCount() const { return fileCount; }

	

	static bool isPartitionMounted() { return partitionMounted; }
};

#endif //_KERNEL_FS_H_
