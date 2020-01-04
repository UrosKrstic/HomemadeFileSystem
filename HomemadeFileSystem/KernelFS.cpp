#include "KernelFS.h"
#include <iostream>
#include <regex>
#include "FCB.h"
#include "RootDirMemoryHandler.h"
#include "PartitionError.h"
#include "NoFreeClustersException.h"


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
		bitVector = new BitVector(partition);
		rootDirMemoryHandler = new RootDirMemoryHandler(*bitVector, rootDirFirstLevelIndexClusterNo, partition, *this);
		fileNameToFCBmap = rootDirMemoryHandler->getNameToFCBMap();
		fileCount = fileNameToFCBmap->size();
		threadID = GetCurrentThreadId();
		return 1;
	}
	catch(PartitionError& pe) {
		delete bitVector;
		delete rootDirMemoryHandler;
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
		WakeAllConditionVariable(&kernel_fs::isMountedCond);
		return 1;
	}
	catch (PartitionError& pe) {
		std::cout << "Unmount failed: " << pe.what() << std::endl;
		return 0;
	}
}

char KernelFS::format() {
	rootDirMemoryHandler->format();
	bitVector->format();
	return 1;
}

char KernelFS::doesExist(std::string& fname) {
	return fileNameToFCBmap->find(fname) != fileNameToFCBmap->end();
}

File * KernelFS::open(char * fname, char mode) {
	if (!formatingInProgress) {
		try {
			std::regex reg("/[^\\.]*\\..{3}");
			std::string fpath(fname);
			std::cmatch m;
			if (!std::regex_match(fpath.c_str(), m, reg)) { std::cout << "Nije prosao regex\n"; return nullptr; }
			if (fileNameToFCBmap->find(fname) == fileNameToFCBmap->end()) {
				if (mode == 'w') {
					FCB* fcb = rootDirMemoryHandler->createNewFile(fpath);
					if (fcb == nullptr) return nullptr;
					else return fcb->createFileInstance(mode);
				}
			}
			else {
				FCB* fcb = (*fileNameToFCBmap)[fpath];
				if (mode == 'r' || mode == 'a') {
					return fcb->createFileInstance(mode);
				}
				else if (mode == 'w') {
					fcb->clearClusters();
					return fcb->createFileInstance(mode);
				}
			}
			return nullptr;
		}
		catch(NoFreeClustersException& e) {
			std::cout << "No free clusters for opening files: " << e.what() << std::endl;
			return nullptr;
		}
		catch(PartitionError& e) {
			std::cout << "Opening of file failed: " << fname  << " " << e.what() << std::endl;
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
	return rootDirMemoryHandler->deleteFile(fpath);
}

