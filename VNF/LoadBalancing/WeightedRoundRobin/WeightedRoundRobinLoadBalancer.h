/*
 * WeightedRoundRobinLoadBalancer.h
 *
 *  Created on: Apr. 23, 2022
 *      Author: trantuan (Hugo)
 */

#ifndef NODES_VNF_LOADBALANCING_WEIGHTEDROUNDROBINLOADBALANCER_H_
#define NODES_VNF_LOADBALANCING_WEIGHTEDROUNDROBINLOADBALANCER_H_


#include "../Base/BaseLoadBalancer.h"
#include "../Base/BaseSliceLoadBalanceExecutor.h"
#include <map>
#include <algorithm>
#include <time.h>

struct MaxVnfResource
{
    double maxCpu = 0;
    double maxRam = 0;
    double maxStorage = 0;
};


class SliceWeightedRoundRobinExecuter : public BaseSliceLoadBalanceExecutor
{
private:
    int nextVNFIndex;
    std::map<int, MaxVnfResource> vnfMaxResourceConfigsMap; // Map vnfIndex with vnf resource configs
    std::map<int, double> vnfIndexSelectProbMap; // Store probability of an vnfIndex to be selected
    //void removeDeletedVNFGateIndexes(const std::vector<int> vnfGateIndexes);

//protected:


public:

    int executeLoadBalancing() override;
    //virtual void updateVNFGateIndexes(const std::vector<int> vnfGateIndexes) override;
    //void updateMaxResourceConfigsMap(int vnfIndex, double maxCpu, double maxRam, double maxStorage) override;
    void updateMaxResourceConfigsMap(int vnfIndex, double maxCpu, double maxRam, double maxStorage);
    void updateGateIndexSelectProb();
    void printMaxVnfResources(const MaxVnfResource* maxResource);
};

/*-------------------------------*/

class WeightedRoundRobinLoadBalancer : public BaseLoadBalancer
{
private:
    //std::map<int, SliceWeightedRoundRobinExecuter*> sliceNumberExecutorMap; // Already defined in Base class

protected:
    ISliceLoadBalanceExecutor* createSliceExecutor() override;

public:
    void updateMaxResourceConfigs(int sliceNumber, int vnfIndex, double maxCpu, double maxRam, double maxStorage) override;
    void requestUpdateVnfIndexSelectProb(int sliceNumber) override;

};

#endif /* NODES_VNF_LOADBALANCING_WEIGHTEDROUNDROBINLOADBALANCER_H_ */
