#include "KernelFile.h"



KernelFile::KernelFile(unsigned int currentSize, unsigned int currentPos, FCB * myFCB) {
	this->currentSize = currentSize;
	this->currentPos = currentPos;
	this->myFCB = myFCB;
}

char KernelFile::write(BytesCnt cnt, char * buffer) {
	return myFCB->write(cnt, buffer, currentPos);
}

BytesCnt KernelFile::read(BytesCnt cnt, char * buffer) {
	unsigned int returnValue;
	if (eof()) return 0;
	else return myFCB->read(cnt, buffer, currentPos);
}

char KernelFile::seek(BytesCnt newPos) {
	currentPos = min(newPos, currentSize);
}


char KernelFile::truncate()
{
	return 0;
}

KernelFile::~KernelFile() {
	//TODO: CUVANJE SVEGA
}
