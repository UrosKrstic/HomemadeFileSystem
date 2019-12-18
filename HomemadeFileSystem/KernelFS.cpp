#include "KernelFS.h"
#include <iostream>
#include <regex>
#include "FCB.h"
#include "RootDirMemoryHandler.h"
#include "PartitionError.h"


INIT_ONCE kernel_fs::initOnceVariable = INIT_ONCE_STATIC_INIT;
CRITICAL_SECTION kernel_fs::fsLock;
CONDITION_VARIABLE kernel_fs::isMountedCond;
CONDITION_VARIABLE kernel_fs::openFilesCond;

bool KernelFS::partitionMounted = false;
ClusterNo KernelFS::bitVectorClusterNo = 0;
ClusterNo KernelFS::rootDirFirstLevelIndexClusterNo = 1;


BOOL CALLBACK kernel_fs::InitFunction(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *lpContext) {
	InitializeConditionVariable(&kernel_fs::isMountedCond);
	InitializeConditionVariable(&kernel_fs::openFilesCond);
	InitializeCriticalSection(&kernel_fs::fsLock);
	//std::cout << "Uspesno inicijalizovano\n";
	return true;
}

char KernelFS::mount(Partition* partition) {
	try {
		this->partition = partition;
		partitionMounted = true;
		bitVector = new BitVector(bitVectorClusterNo, partition);
		rootDirMemoryHandler = new RootDirMemoryHandler(*bitVector, rootDirFirstLevelIndexClusterNo, partition);
		fileNameToFCBmap = rootDirMemoryHandler->getNameToFCBMap();
		fileCount = fileNameToFCBmap->size();
		return 1;
	}
	catch(PartitionError& pe) {
		std::cout << "Mount failed: " << pe.what() << std::endl;
		return 0;
	}
}

char KernelFS::unmount() {
	try {
		partition = nullptr;
		partitionMounted = false;
		bitVector->saveToDrive();
		rootDirMemoryHandler->saveToDrive();
		delete bitVector;
		delete rootDirMemoryHandler;
		std::cout << "Budi niti na mount cond\n";
		WakeAllConditionVariable(&kernel_fs::isMountedCond);
		return 1;
	}
	catch (PartitionError& pe) {
		std::cout << "Unmount failed: " << pe.what() << std::endl;
		return 0;
	}
}

char KernelFS::format() {
	bitVector->format();
	rootDirMemoryHandler->format();
	return 1;
}

char KernelFS::doesExist(std::string& fname) {
	return (*fileNameToFCBmap)[fname] != nullptr;
}

File * KernelFS::open(char * fname, char mode) {
	if (!formatingInProgress) {
		try {
			std::regex reg("^/[^\\.]*\\..{3}&");
			std::string fpath(fname);
			std::cmatch m;
			if (!std::regex_match(fpath.c_str(), m, reg)) return nullptr;
			//rootDirMemoryHandler->createNewFile(fpath);
			//return nullptr;
			if ((*fileNameToFCBmap)[fpath] == nullptr) {
				if (mode == 'w') {
					rootDirMemoryHandler->createNewFile(fpath);
					return nullptr; // nece biti nullptr 
				}
				return nullptr;
			}
			else {
				return nullptr;
			}
		}
		catch(PartitionError& pe) {
			std::cout << "Opening of file failed: " << fname  << " " << pe.what() << std::endl;
			return nullptr;
		}
	}
	else return nullptr;
}

char KernelFS::deleteFile(char * fname) {
	std::regex reg("/[^\\.]*\\..{3}");
	std::string fpath(fname);
	std::cmatch m;
	if (!std::regex_match(fpath.c_str(), m, reg)) return 0;
	rootDirMemoryHandler->deleteFile(fpath);
	FCB * fcb = (*fileNameToFCBmap)[fpath];
	(*fileNameToFCBmap)[fpath] = nullptr;
	delete fcb;
	return 0;
}

