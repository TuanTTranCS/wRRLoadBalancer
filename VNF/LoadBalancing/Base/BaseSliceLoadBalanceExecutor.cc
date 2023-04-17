#include "./BaseSliceLoadBalanceExecutor.h"
#include <algorithm>

void BaseSliceLoadBalanceExecutor::updateVNFGateIndexes(const std::vector<int> vnfGateIndexes)
{
    addNewVNFGateIndexes(vnfGateIndexes);
    removeDeletedVNFGateIndexes(vnfGateIndexes);
}

void BaseSliceLoadBalanceExecutor::addNewVNFGateIndexes(const std::vector<int> vnfGateIndexes)
{
    for(std::vector<int>::const_iterator it = vnfGateIndexes.begin(); it != vnfGateIndexes.end(); ++it)
    {
        if (std::find(existingVNFIndex.begin(), existingVNFIndex.end(), *it) == existingVNFIndex.end())
        {
            existingVNFIndex.push_back(*it);
        }
    }
}

void BaseSliceLoadBalanceExecutor::removeDeletedVNFGateIndexes(const std::vector<int> vnfGateIndexes)
{
    std::vector<int> indexesToRemove;
    for(std::vector<int>::iterator it = existingVNFIndex.begin(); it != existingVNFIndex.end(); ++it)
    {
        if (std::find(vnfGateIndexes.begin(), vnfGateIndexes.end(), *it) == vnfGateIndexes.end())
        {
            indexesToRemove.push_back(*it);
        }
    }

    auto newEnd = std::remove_if(existingVNFIndex.begin(), existingVNFIndex.end(), [indexesToRemove](int value)->bool 
    {
        return std::find(indexesToRemove.begin(), indexesToRemove.end(), value) != indexesToRemove.end();
    });

    existingVNFIndex.erase(newEnd, existingVNFIndex.end());
}

/*void BaseSliceLoadBalanceExecutor::updateMaxResourceConfigsMap(int vnfIndex, double maxCpu, double maxRam, double maxStorage)
{
    throw std::runtime_error("updateMaxResourceConfigsMap is not implemented");
}*/

