#ifndef _KERNELFILE_H_
#define _KERNELFILE_H_
#include "FCB.h"

class KernelFile {
private:
	FCB * myFCB;
	unsigned int currentSize;
	unsigned int currentPos;
public:
	KernelFile(unsigned int currentSize, unsigned int currentPos, FCB * myFCB);
	char write(BytesCnt cnt, char *buffer);
	BytesCnt read(BytesCnt cnt, char * buffer);
	char seek(BytesCnt newPos);
	BytesCnt filePos() { return currentPos; }
	char eof() {
		return currentPos < currentSize ? 0 : 2;
	}
	BytesCnt getFileSize() { return currentSize; }
	char truncate();
	~KernelFile();
};

#endif //_KERNELFILE_H_