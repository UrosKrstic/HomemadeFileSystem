#ifndef _FCB_H_
#define _FCB_H_
#include "KernelFS.h"
#include "DataCluster.h"

struct FCBData;
class KernelFile;
class FirstLevelIndexCluster;


class FCB {
public:

	enum Mode {idle, reading, writing, append};
	
	typedef struct FCBIndexStruct {
		ClusterNo secondLvlIndex = 0;
		ClusterNo dataClusterIndex = 0;
		ClusterNo rowInDataCluster = 0;
		bool isInvalid = false;
		FCBIndexStruct() = default;
		FCBIndexStruct(ClusterNo sli, ClusterNo dci, ClusterNo ridc) : secondLvlIndex(sli), dataClusterIndex(dci), rowInDataCluster(ridc) {}
		void setIndices(ClusterNo sli, ClusterNo dci, ClusterNo ridc, bool isInvalid = false) {
			secondLvlIndex = sli; dataClusterIndex = dci;  rowInDataCluster = ridc;
		}
	} FCBIndex;

	typedef struct FCBDataStruct {
		char name[FNAMELEN];
		char ext[FEXTLEN];
		char freeByte;
		unsigned int firstIndexClusterNo;
		unsigned int fileSize;
		char freeBytes[12];
		bool allZeros() {
			char * data = reinterpret_cast<char*>(this);
			for (int i = 0; i < 20; i++)
				if (data[i] != static_cast<char>(0)) return false;
			return true;
		}
		bool isEmpty() {
			return name[0] == 0;
		}
	} FCBData;

	friend class KernelFile;
	FCB(FCBIndex& fcbInd, FCBData * data, Partition * p, BitVector& bitV, KernelFS& kerFS, DataCluster& dc);
	FCB(FCBIndex&& fcbInd, FCBData * data, Partition * p, BitVector& bitV, KernelFS& kerFS, DataCluster& dc);

	FCB(const FCB&) = delete;
	FCB(FCB&&) = delete;
	FCB& operator=(const FCB&) = delete;
	FCB& operator=(FCB&&) = delete;

	~FCB();
	File * createFileInstance(char mode);
	void clearClusters();
	void saveToDrive();
	void setFCBDataToFree() { fcbData->name[0] = 0; }
	FCBIndex getFCBIndex() { return fcbIndex; }
private:
	unsigned numberOfOpenFiles = 0;

	CRITICAL_SECTION criticalSection;
	CONDITION_VARIABLE readCond, writeCond;
	int readCount = 0;
	bool loadFLI = true;
	Mode currentMode = idle;
	
	FCBIndex fcbIndex;
	FCBData * fcbData;
	Partition * part;
	BitVector &bitVector;
	KernelFS &kernelFS;
	DataCluster& myDC;
	FirstLevelIndexCluster * fliCluster = nullptr;
	
	static constexpr unsigned numOfFreeBytes = 12;
	static constexpr  unsigned rowsInCluster = ClusterSize / sizeof(FCBData);
};




#endif //_FCB_H_
