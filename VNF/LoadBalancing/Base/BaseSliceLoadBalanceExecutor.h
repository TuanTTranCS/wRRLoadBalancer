#ifndef NODES_VNF_LOADBALANCING_BASE_BASESLICELOADBALANCEEXECUTOR_H_
#define NODES_VNF_LOADBALANCING_BASE_BASESLICELOADBALANCEEXECUTOR_H_

#include "./ISliceLoadBalanceExecutor.h"

class BaseSliceLoadBalanceExecutor : public  ISliceLoadBalanceExecutor 
{
    protected:
        std::vector<int> existingVNFIndex;
    public:
        virtual int executeLoadBalancing() override = 0;
        virtual void updateVNFGateIndexes(const std::vector<int> vnfGateIndexes) override;
        //virtual void updateMaxResourceConfigsMap(int vnfIndex, double maxCpu, double maxRam, double maxStorage) override;
    private:
        void addNewVNFGateIndexes(const std::vector<int> vnfGateIndexes);
        void removeDeletedVNFGateIndexes(const std::vector<int> vnfGateIndexes);
};

#endif
