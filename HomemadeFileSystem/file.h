#ifndef _FILE_H_
#define _FILE_H_
#include "fs.h"

class KernelFile;
namespace fs {
	class File {
	public:
		//zatvaranje fajla
		~File();
		char write(BytesCnt, char *buffer);
		BytesCnt read(BytesCnt, char * buffer);
		char seek(BytesCnt);
		BytesCnt filePos();
		char eof();
		BytesCnt getFileSize();
		char truncate();
	private:
		friend class FS;
		friend class KernelFS;
		File(); //objekat fajla se moze kreirati samo otvaranjem
		KernelFile *myImpl;

	};
}

#endif //_FILE_H_
