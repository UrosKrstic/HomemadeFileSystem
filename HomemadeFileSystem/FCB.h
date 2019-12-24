#ifndef _FCB_H_
#define _FCB_H_
#include "KernelFS.h"

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
		FCBIndexStruct(const FCBIndexStruct& fcbi) : secondLvlIndex(fcbi.secondLvlIndex), dataClusterIndex(fcbi.dataClusterIndex), rowInDataCluster(fcbi.rowInDataCluster) {}
		void setIndices(ClusterNo sli, ClusterNo dci, ClusterNo ridc, bool isInvalid = false) { secondLvlIndex = sli; dataClusterIndex = dci;  rowInDataCluster = ridc; }//goToNextIndex(); }
		/*void goToNextIndex() {
			rowInDataCluster++;
			if (rowInDataCluster >= rowsInCluster) {
				rowInDataCluster = 0;
				dataClusterIndex++;
			}
			if (dataClusterIndex >= ClusterSize / 4) {
				dataClusterIndex = 0;
				secondLvlIndex++;
			}
			if (secondLvlIndex == ClusterSize / 4) isInvalid = true;
		}*/
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
	FCB(FCBIndex& fcbInd, FCBData * data, Partition * p, BitVector& bitV, KernelFS& kerFS);
	FCB(FCBIndex&& fcbInd, FCBData * data, Partition * p, BitVector& bitV, KernelFS& kerFS);
	~FCB();
	File * createFileInstance(char mode);
	char write(BytesCnt cnt, char *buffer, unsigned int& currentPosOfFile, unsigned int & currentSizeOfFile);
	BytesCnt read(BytesCnt cnt, char *buffer, unsigned int& currentPosOfFile);
	char truncate(unsigned int& currentPosOfFile, unsigned int& currentSizeOfFile);
	void setFCBDataToFree() { fcbData->name[0] = 0; }
	FCBIndex getFCBIndex() { return fcbIndex; }
private:
	unsigned numberOfOpenFiles = 0;

	CRITICAL_SECTION criticalSection;
	CONDITION_VARIABLE readCond, writeCond;
	int readCount = 0;
	Mode currentMode = idle;
	
	FCBIndex fcbIndex;
	FCBData * fcbData;
	Partition * part;
	BitVector &bitVector;
	KernelFS &kernelFS;
	FirstLevelIndexCluster * fliCluster;
	
	static constexpr unsigned numOfFreeBytes = 12;
	static constexpr  unsigned rowsInCluster = ClusterSize / sizeof(FCBData);
};




#endif //_FCB_H_
