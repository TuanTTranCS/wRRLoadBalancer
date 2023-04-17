/*
 * RoundRobinLoadBalancer.h
 *
 *  Created on: Aug. 27, 2021
 *      Author: vibisson
 */

#ifndef NODES_VNF_LOADBALANCING_ROUNDROBINLOADBALANCER_H_
#define NODES_VNF_LOADBALANCING_ROUNDROBINLOADBALANCER_H_


#include "../Base/BaseLoadBalancer.h"
#include "../Base/BaseSliceLoadBalanceExecutor.h"
#include <map>


class SliceRoundRobinExecuter : public BaseSliceLoadBalanceExecutor
{
private:
    int nextVNFIndex;
    
public:
    int executeLoadBalancing() override;
};

/*-------------------------------*/

class RoundRobinLoadBalancer : public BaseLoadBalancer
{
private:
    std::map<int, SliceRoundRobinExecuter*> sliceNumberExecuterMap;

protected:
    ISliceLoadBalanceExecutor* createSliceExecutor() override;
};

#endif /* NODES_VNF_LOADBALANCING_ROUNDROBINLOADBALANCER_H_ */
