#ifndef _NO_FREE_CLUSTERS_H_
#define _NO_FREE_CLUSTERS_H_
#include <exception>

class NoFreeClustersException : public std::exception {
public:
	NoFreeClustersException() = default;
	const char * what() const override { return "Current cluster is full"; }
};

#endif //_NO_FREE_CLUSTERS_H_
