
#ifndef NODES_VNF_LOADBALANCING_BASE_BASELOADBALANCER_H_
#define NODES_VNF_LOADBALANCING_BASE_BASELOADBALANCER_H_

#include "./ILoadBalancer.h"
#include "./ISliceLoadBalanceExecutor.h"
#include <map>

class BaseLoadBalancer : public ILoadBalancer 
{
    protected:
        std::map<int, ISliceLoadBalanceExecutor*> sliceNumberExecutorMap;
        
    public:
        virtual int getVNFGateIndex(int sliceNumber) override;
        virtual void updateVNFGateIndexes(int sliceNumber, const std::vector<int> vnfGateIndexes) override;
        virtual void updateMaxResourceConfigs(int sliceNumber, int vnfIndex, double maxCpu, double maxRam, double maxStorage) override;
        virtual void requestUpdateVnfIndexSelectProb(int sliceNumber) override;
    protected:
        virtual ISliceLoadBalanceExecutor* createSliceExecutor();
};

#endif
