#ifndef _PART_H_
#define _PART_H_

typedef unsigned long ClusterNo;
const unsigned long ClusterSize = 2048;

class PartitionImpl;
namespace part {
	class Partition {
	public:
		Partition(char *);
		virtual ClusterNo getNumOfClusters() const;

		virtual int readCluster(ClusterNo, char *buffer);
		virtual int writeCluster(ClusterNo, const char *buffer);

		virtual ~Partition();
	private:
		PartitionImpl * myImpl;
	};
}

#endif // _PART_H_
