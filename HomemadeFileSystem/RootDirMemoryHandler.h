#ifndef _ROOTDIRMH_H_
#define _ROOTDIRMH_H_
#include "MemoryHandler.h"
#include <map>
#include "FCB.h"
#include <queue>

class FCB;


class RootDirMemoryHandler : public MemoryHandler {
public:
	RootDirMemoryHandler(BitVector& bitVector, ClusterNo no, Partition * part, KernelFS& kerFS) : MemoryHandler(bitVector, no, part, false), kernelFS(kerFS) { InitializeCriticalSection(&critSection); }

	RootDirMemoryHandler(const RootDirMemoryHandler&) = delete;
	RootDirMemoryHandler(RootDirMemoryHandler&&) = delete;
	RootDirMemoryHandler& operator=(const RootDirMemoryHandler&) = delete;
	RootDirMemoryHandler& operator=(RootDirMemoryHandler&&) = delete;
	
	~RootDirMemoryHandler();

	std::map<std::string, FCB*>* getNameToFCBMap();
	FCB * createNewFile(std::string& fpath);
	int deleteFile(std::string& fpath);
	void saveToDrive();
	void format();
	char * getDCFromCacheAndUpdateCache(unsigned sli, unsigned dci);

	typedef struct FCBIndexStruct {
		ClusterNo sli = 0;
		ClusterNo dc = 0;
		ClusterNo ridc = 0;
		FCBIndexStruct() = default;
		FCBIndexStruct(ClusterNo _sli, ClusterNo _dc, ClusterNo _ridc) : sli(_sli), dc(_dc), ridc(_ridc) {}
		void setIndices(ClusterNo _sli, ClusterNo _dc, ClusterNo _ridc) { sli = _sli; dc = _dc; ridc = _ridc; }
	} FCBIndex;
	
private:

	

	CRITICAL_SECTION critSection;
	KernelFS& kernelFS;
	std::map<std::string, FCB*> nameToFCBmap;
	std::queue<FCBIndex> leftoverFreeFileSlots;
	std::vector<DataCluster*> cache;
	FCBIndex nextFreeSlot;
	bool isMapCreated = false;

	static constexpr unsigned int rowCountInDataCluster = ClusterSize / 32;
	static constexpr unsigned int ClusterSize_32b = ClusterSize / 4;
	static constexpr unsigned int cacheSize = 64;

};

#endif //_ROOTDIRMH_H_