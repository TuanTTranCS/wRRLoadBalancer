#include "./BaseLoadBalancer.h"
#include <stdexcept>

int BaseLoadBalancer::getVNFGateIndex(int sliceNumber) 
{
    if (sliceNumberExecutorMap.find(sliceNumber) == sliceNumberExecutorMap.end())
    {
        throw std::runtime_error("No executor for the slice");
    }

    return sliceNumberExecutorMap[sliceNumber]->executeLoadBalancing();
}

void BaseLoadBalancer::updateVNFGateIndexes(int sliceNumber, const std::vector<int> vnfGateIndexes)
{
    if (sliceNumberExecutorMap.find(sliceNumber) == sliceNumberExecutorMap.end()) 
    {
        sliceNumberExecutorMap[sliceNumber] = createSliceExecutor();
    }

    sliceNumberExecutorMap[sliceNumber]->updateVNFGateIndexes(vnfGateIndexes);
}


void BaseLoadBalancer::updateMaxResourceConfigs(int sliceNumber, int vnfIndex, double maxCpu, double maxRam, double maxStorage)
{
    throw std::runtime_error("updateMaxResourceConfigs is not implemented");
}

void BaseLoadBalancer::requestUpdateVnfIndexSelectProb(int sliceNumber)
{
    throw std::runtime_error("requestUpdateVnfIndexSelectProb is not implemented");
}

ISliceLoadBalanceExecutor* BaseLoadBalancer::createSliceExecutor()
{
    throw std::runtime_error("createSliceExecutor is not implemented");
}
