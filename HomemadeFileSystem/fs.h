#ifndef  _FS_H_
#define _FS_H_

typedef long FileCnt;
typedef unsigned long BytesCnt;

const unsigned int FNAMELEN = 8;
const unsigned int FEXTLEN = 3;

class KernelFS;
class Partition;
class File;

class FS {
public:
	~FS();

	//montira particiju
	//vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha
	static char mount(Partition *partition);

	//demontira particiju
	//vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha
	static char unmount();

	//formatira particiju
	//vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha
	static char format();

	//vraca -1 u slucaju neuspeha ili broj fajlova u slucaju uspeha 
	static FileCnt readRootDir();

	//argument je naziv fajla sa apsolutnom putanjom
	static char doesExist(char *fname);

	static File * open(char * fname, char mode);
	static char deleteFile(char * fname);
protected:
	FS();
	static KernelFS *myImpl;
};

#endif // _FS_H_

