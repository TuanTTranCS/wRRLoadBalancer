#ifndef NODES_VNF_LOADBALANCING_ISLICELOADBALANCEEXECUTOR_H_
#define NODES_VNF_LOADBALANCING_ISLICELOADBALANCEEXECUTOR_H_

#include <vector>

class ISliceLoadBalanceExecutor 
{
    public:
        virtual int executeLoadBalancing() = 0;
        virtual void updateVNFGateIndexes(const std::vector<int> vnfGateIndexes) = 0;
        //virtual void updateMaxResourceConfigsMap(int vnfIndex, double maxCpu, double maxRam, double maxStorage) = 0;
};

#endif
