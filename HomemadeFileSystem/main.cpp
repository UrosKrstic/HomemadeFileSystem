#include <windows.h>
#include <iostream>
#include "fs.h"
#include "part.h"


int main(int argc, char* argv[]) {
	
	auto*partition = new Partition(const_cast<char *>("p1.ini"));

	std::cout << "Poceo sa mount\n";
	FS::mount(partition);
	std::cout << "Poceo sa pravljenjem fajlova." << std::endl;
	FS::open(const_cast<char*>("/mam1.txt"), 'w');
	FS::open(const_cast<char*>("/mam2.txt"), 'w');
	FS::open(const_cast<char*>("/mam3.exe"), 'w');
	FS::open(const_cast<char*>("/mam4.txt"), 'w');
	FS::open(const_cast<char*>("/ma4.txt"), 'w');
	FS::open(const_cast<char*>("/mama.cpp"), 'w');
	FS::open(const_cast<char*>("/mala.txt"), 'w');


	std::cout << "Poceo sa unmount\n";
	FS::unmount();

	std::cout << "Poceo sa mount\n";
	FS::mount(partition);

	FS::unmount();
	
	delete partition;	
}
