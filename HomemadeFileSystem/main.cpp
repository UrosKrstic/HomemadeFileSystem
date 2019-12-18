#include <windows.h>
#include <iostream>
#include "fs.h"
#include "part.h"


int main(int argc, char* argv[]) {
	
	auto*partition = new Partition(const_cast<char *>("p1.ini"));

	std::cout << "Poceo sa mount\n";
	FS::mount(partition);
	std::cout << "Pocinje pravljenje fajlova\n";
	char * fajl1 = const_cast<char*>("/mam1.txt");
	char * fajl2 = const_cast<char*>("/mam2.txt");
	char * fajl3 = const_cast<char*>("/mam3.txt");

	FS::open(fajl1, 'w');
	FS::open(fajl2, 'w');
	FS::open(fajl3, 'w');

	if (FS::doesExist(fajl2)) { std::cout << "Postoji fajl: " << fajl2 << std::endl; }
	FS::deleteFile(fajl2);
	if (!FS::doesExist(fajl2)) { std::cout << "Obrisan fajl: " << fajl2 << std::endl; }
	FS::open(fajl2, 'w');
	if (FS::doesExist(fajl2)) { std::cout << "Dodat opet fajl: " << fajl2 << std::endl; }

	FS::deleteFile(fajl1);
	FS::deleteFile(fajl3);

	std::cout << "Poceo sa unmount\n";
	FS::unmount();

	std::cout << "Poceo sa mount\n";
	FS::mount(partition);
	if (!FS::doesExist(fajl1)) { std::cout << "Ne postoji fajl: " << fajl1 << std::endl; }
	FS::open(fajl1, 'w');
	FS::open(fajl3, 'w');
	if (FS::doesExist(fajl1)) { std::cout << "Postoji fajl: " << fajl1 << std::endl; }

	FS::format();

	std::cout << "Poceo sa unmount\n";
	FS::unmount();


	
	delete partition;	
}
