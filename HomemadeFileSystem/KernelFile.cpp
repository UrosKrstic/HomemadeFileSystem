#include "KernelFile.h"



KernelFile::KernelFile(unsigned int currentSize, unsigned int currentPos, FCB * myFCB) {
	this->currentSize = currentSize;
	this->currentPos = currentPos;
	this->myFCB = myFCB;
}

char KernelFile::write(BytesCnt cnt, char * buffer) {
	return myFCB->write(cnt, buffer, currentPos, currentSize);
}

BytesCnt KernelFile::read(BytesCnt cnt, char * buffer) {
	//unsigned int returnValue;
	if (eof()) return 0;
	else return myFCB->read(cnt, buffer, currentPos);
}

char KernelFile::seek(BytesCnt newPos) {
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
	return myFCB->truncate(currentPos, currentSize);
}

KernelFile::~KernelFile() {
	//TODO: CUVANJE SVEGA I BRISANJE FAJLA
}
