#ifndef  _PARTITION_ERROR_H_
#define _PARTITION_ERROR_H_
#include <exception>

class PartitionError : public std::exception {
public:
	PartitionError() = default;
	const char * what() const override { return "Error occured in Partition."; }
};

#endif //_PARTITION_ERROR_H_
