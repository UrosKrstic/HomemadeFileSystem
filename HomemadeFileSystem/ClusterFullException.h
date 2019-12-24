#ifndef _CLUSTER_FULL_EXCEPTION_H_
#define _CLUSTER_FULL_EXCEPTION_H_
#include <exception>

class ClusterFullException : public std::exception {
public:
	ClusterFullException() = default;
	const char * what() const override { return "Current cluster is full"; }
};

#endif //_CLUSTER_FULL_EXCEPTION_H_
