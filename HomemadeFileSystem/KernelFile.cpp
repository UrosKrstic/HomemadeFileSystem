#include "KernelFile.h"
#include "FirstLevelIndexCluster.h"



KernelFile::KernelFile(FCB * myFCB, char mode, FirstLevelIndexCluster * fli) {
	this->currentSize = myFCB->fcbData->fileSize;
	this->currentPos = mode != 'a' ? 0 : currentSize;
	this->mode = mode;
	this->myFCB = myFCB;
	fliCluster = fli;
	InitializeCriticalSection(&critSection);
	InitializeConditionVariable(&condVar);
}

KernelFile::~KernelFile() {
	if (mode == 'r') {
		myFCB->readCount--;
		if (myFCB->readCount == 0) {
			myFCB->currentMode = FCB::idle;
			WakeConditionVariable(&myFCB->writeCond);
			WakeAllConditionVariable(&myFCB->readCond);
		}
	}
	else {
		myFCB->currentMode = FCB::idle;
		WakeConditionVariable(&myFCB->writeCond);
		WakeAllConditionVariable(&myFCB->readCond);
	}
	DeleteCriticalSection(&critSection);
	//SAVE SHIT GOD DAAAAMN TODO: DO IT M8
}


char KernelFile::write(BytesCnt cnt, char * buffer) {
	if (mode == 'r' || cnt + currentPos > FirstLevelIndexCluster::getMaxFileSize()) return 0;



	return 0;
}

BytesCnt KernelFile::read(BytesCnt cnt, char * buffer)  {	
	if (eof() && mode != 'r') return 0;
	return 0;
}

char KernelFile::seek(BytesCnt newPos) {
	char returnVal = 0;
	if (newPos < currentSize) {
		currentPos = newPos;
		return 1;
	}
	else {
		return 0;
	}
}


char KernelFile::truncate()
{
	return 0;
}


