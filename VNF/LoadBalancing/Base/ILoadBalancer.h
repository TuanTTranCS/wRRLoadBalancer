/*
 * LoadBalancer.h
 *
 *  Created on: Aug. 27, 2021
 *      Author: vibisson
 */

#ifndef NODES_VNF_LOADBALANCING_BASE_ILOADBALANCER_H_
#define NODES_VNF_LOADBALANCING_BASE_ILOADBALANCER_H_

#include <vector>

class ILoadBalancer
{
public:
    virtual int getVNFGateIndex(int sliceNumber) = 0;
    virtual void updateVNFGateIndexes(int sliceNumber, const std::vector<int> vnfGateIndexes) = 0;
    virtual void updateMaxResourceConfigs(int sliceNumber, int vnfIndex, double maxCpu, double maxRam, double maxStorage) = 0;
    virtual void requestUpdateVnfIndexSelectProb(int sliceNumber) = 0;
};


#endif /* NODES_VNF_LOADBALANCING_ILOADBALANCER_H_ */
