#ifndef _FILE_H_
#define _FILE_H_
#include "fs.h"

class KernelFile;

class File {
public:
	//zatvaranje fajla
	~File();
	char write(BytesCnt cnt, char *buffer);
	BytesCnt read(BytesCnt cnt, char * buffer);
	char seek(BytesCnt cnt);
	BytesCnt filePos();
	char eof();
	BytesCnt getFileSize();
	char truncate();
private:
	friend class FS;
	friend class KernelFS;
	friend class FCB;
	File()=default; //objekat fajla se moze kreirati samo otvaranjem
	KernelFile *myImpl;

};


#endif //_FILE_H_
