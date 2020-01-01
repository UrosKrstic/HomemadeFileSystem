#include <windows.h>
#include <iostream>
#include "fs.h"
#include "part.h"
#include "file.h"

using namespace std;

char * str_creator(int size, char fill) {
	char * buffer = new char[size];
	for (int i = 0; i < size; i++) buffer[i] = fill;
	return buffer;
}

void func1() {
	char * fpath = const_cast<char*>("/fajl1.txt");

	File* file = FS::open(fpath, 'a');

	char * buffer = str_creator(4000, 'c');
	file->write(4000, buffer);

	file->seek(2100);

	delete[] buffer;
	buffer = str_creator(20, 'd');
	file->write(20, buffer);

	delete file;

	file = FS::open(fpath, 'r');

	delete[] buffer;
	buffer = str_creator(10, 'b');

	file->read(10, buffer);

	cout << buffer << endl;

	delete[] buffer;
}

void func2() {
	char * fpath = const_cast<char*>("/fajl1.txt");
	File* file = FS::open(fpath, 'a');
	int size = 510 * 2048;
	char* buffer = str_creator(size, 'f');
	file->write(size, buffer);
	delete file;
	delete[] buffer;
}

void func3() {
	char * fpath = const_cast<char*>("/fajl1.txt");
	File* file = FS::open(fpath, 'a');

	file->seek(file->getFileSize() - 5000);
	file->truncate();


	delete file;
}

void func4() {
	FS::deleteFile(const_cast<char*>("/fajl1.txt"));
	if (FS::doesExist(const_cast<char*>("/fajl1.txt"))) cout << "NO" << endl;
}


int main(int argc, char* argv[]) {
	auto * partition = new Partition(const_cast<char *>("p1.ini"));

	FS::mount(partition);

	func4();

	FS::unmount();

	delete partition;
}
