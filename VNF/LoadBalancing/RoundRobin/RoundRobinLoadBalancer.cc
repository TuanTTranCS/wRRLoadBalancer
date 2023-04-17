/*
 * RoundRobinLoadBalancer.cc
 *
 *  Created on: Aug. 27, 2021
 *      Author: vibisson
 */

#include "./RoundRobinLoadBalancer.h"
#include <iostream>

ISliceLoadBalanceExecutor* RoundRobinLoadBalancer::createSliceExecutor()
{
    return new SliceRoundRobinExecuter();
}

int SliceRoundRobinExecuter::executeLoadBalancing()
{
    if (nextVNFIndex >= existingVNFIndex.size())
    {
        nextVNFIndex = 0;
    }

    int vnfIndex = existingVNFIndex[nextVNFIndex];
    nextVNFIndex++;

    return vnfIndex;
}
