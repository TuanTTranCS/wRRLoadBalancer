/*
 * WeightedRoundRobinLoadBalancer.cc
 *
 *  Created on: Apr. 23, 2022
 *      Author: trantuan (Hugo)
 */

#include "WeightedRoundRobinLoadBalancer.h"

#include <iostream>

ISliceLoadBalanceExecutor* WeightedRoundRobinLoadBalancer::createSliceExecutor()
{
    return new SliceWeightedRoundRobinExecuter();
}

void WeightedRoundRobinLoadBalancer::updateMaxResourceConfigs(int sliceNumber, int vnfIndex, double maxCpu, double maxRam, double maxStorage)
{
    try {
        // If the sliceNumber not in the sliceExecutor map, add it into the map
        if (sliceNumberExecutorMap.find(sliceNumber) == sliceNumberExecutorMap.end())
        {
            /*
            std::cout << "Sending request to create sliceExecutor for slice " << sliceNumber
                    << " from WeightedRoundRobinLoadBalancer::updateMaxResourceConfigs" << std::endl;
                    */
            sliceNumberExecutorMap[sliceNumber] = createSliceExecutor();


        }

        // then call the function to pass max VNF resource configs to the executor
        /*
        std::cout << "Sending max resource config update from load balancer to executor of slice " << sliceNumber
                << " on vnfGateIndex#" << vnfIndex << std::endl;
        */
        ((SliceWeightedRoundRobinExecuter*)sliceNumberExecutorMap[sliceNumber])->updateMaxResourceConfigsMap(vnfIndex, maxCpu, maxRam, maxStorage);
    }

    catch(const std::runtime_error& re)
    {
        // speciffic handling for runtime_error
        std::cerr << "Runtime error during executing updateMaxResourceConfigs: " << re.what() << std::endl;
    }
    catch(const std::exception& ex)
    {
        // speciffic handling for all exceptions extending std::exception, except
        // std::runtime_error which is handled explicitly
        std::cerr << "Error occurred during executing updateMaxResourceConfigs: " << ex.what() << std::endl;
    }
    catch(...)
    {
        // catch any other errors (that we have no information about)
        std::cerr << "Unknown failure occurred. Possible memory corruption during executing updateMaxResourceConfigs." << std::endl;
    }
}

void WeightedRoundRobinLoadBalancer::requestUpdateVnfIndexSelectProb(int sliceNumber)
{
    if (sliceNumberExecutorMap.find(sliceNumber) == sliceNumberExecutorMap.end())
    {
        /*
        std::cout << "Sending request to create sliceExecutor for slice " << sliceNumber
                << " from WeightedRoundRobinLoadBalancer::requestUpdateVnfIndexSelectProb" << std::endl;
                */
        sliceNumberExecutorMap[sliceNumber] = createSliceExecutor();
    }

    /*
    std::cout << "Sending request to update vnfIndex select prob. from load balancer "
            << "to executor of slice " << sliceNumber << std::endl;
            */
    ((SliceWeightedRoundRobinExecuter*)sliceNumberExecutorMap[sliceNumber])->updateGateIndexSelectProb();
}
/*
void SliceWeightedRoundRobinExecuter::removeDeletedVNFGateIndexes(const std::vector<int> vnfGateIndexes)
{
    std::vector<int> indexesToRemove;
    std::map<int,MaxVnfResource>::iterator it_vnfResourceMap;
    for(std::vector<int>::iterator it = existingVNFIndex.begin(); it != existingVNFIndex.end(); ++it)
    {
        if (std::find(vnfGateIndexes.begin(), vnfGateIndexes.end(), *it) == vnfGateIndexes.end())
        {
            indexesToRemove.push_back(*it);
            // Remove the corresponding VNfIndex to max resouce config mapping as well:
            it_vnfResourceMap = vnfMaxResourceConfigsMap.find(*it);


            if (it_vnfResourceMap != vnfMaxResourceConfigsMap.end())
            {
                vnfMaxResourceConfigsMap.erase(it_vnfResourceMap);
            }

        }
    }

    auto newEnd = std::remove_if(existingVNFIndex.begin(), existingVNFIndex.end(), [indexesToRemove](int value)->bool
    {
        return std::find(indexesToRemove.begin(), indexesToRemove.end(), value) != indexesToRemove.end();
    });

    existingVNFIndex.erase(newEnd, existingVNFIndex.end());
}
*/

int SliceWeightedRoundRobinExecuter::executeLoadBalancing()
{
    bool vnfIndexSelected = false;
    int vnfIndex;
    int currentVnfIndex = -1;
    double vnfIndexProb = 0;
    double accumProb = 0;
    double randProb = 0;
    double previousProb = 0;

    // Calculate traditional RoundRobinLoadBalancer method first
    if (nextVNFIndex >= existingVNFIndex.size())
    {
        nextVNFIndex = 0;
    }

    vnfIndex = existingVNFIndex[nextVNFIndex]; // Index to return with traditional RoundRobin
    nextVNFIndex++;
    //std::cout << "VNF index initially selected with traditional Round Robin: " << vnfIndex << std::endl;
    // Generate random number from 0 to 1
    //srand((unsigned)time(NULL));
    if (existingVNFIndex.size() > 1){
        randProb = (double)rand() / (double)RAND_MAX;

        for (std::vector<int>::iterator it = existingVNFIndex.begin(); it != existingVNFIndex.end(); ++it)
        {
            currentVnfIndex = *it;


            // Get probability to for the GateIndex to be selected
            vnfIndexProb = vnfIndexSelectProbMap[currentVnfIndex];
            accumProb += vnfIndexProb;

            if (randProb <= accumProb)
            {
                vnfIndexSelected = true;
                vnfIndex = currentVnfIndex;

            }

            /*
            std::string selected = vnfIndexSelected? "True" : "False";
            std::cout << "    VNF Gate Index in considering: " << currentVnfIndex << " - Selection: " << selected
                    << " (" << previousProb << " <= " << randProb << " <= " << accumProb << ")." << std::endl;
                    */
            if (vnfIndexSelected) break;
            if (it > existingVNFIndex.begin()) previousProb = accumProb;
        }
    }

    std::cout << "VNF " << vnfIndex << " is finally selected with probability "
            << randProb << " vs. " << vnfIndexProb << std::endl;
    return vnfIndex;
}

void SliceWeightedRoundRobinExecuter::updateMaxResourceConfigsMap(int vnfIndex, double maxCpu, double maxRam, double maxStorage)
{
    //try {

        bool vnfIndexExisted = false;
        /*
        std::cout << "Received max VNF resource config update from Load balancer: VNF"
                << vnfIndex << " - Max CPU: " << maxCpu << " - Max RAM: " << maxRam << " - Max storage: "
                << maxStorage << std::endl;
                */
        MaxVnfResource maxResources = {maxCpu, maxRam, maxStorage};
        //std::cout << "maxResources:" << std::endl;
        //printMaxVnfResources(&maxResources);


        //std::cout << "Current vnfMaxResourceConfigsMap:" << std::endl;

        std::map<int, MaxVnfResource>::iterator it;
        it = vnfMaxResourceConfigsMap.find(vnfIndex);
        if (it != vnfMaxResourceConfigsMap.end())
        {
            vnfIndexExisted = true;
        }
        /*for (it = vnfMaxResourceConfigsMap.begin(); it != vnfMaxResourceConfigsMap.end(); it++)
        {
            int vnfId = it->first;
            if (vnfId == vnfIndex) vnfIndexExisted = true;
            MaxVnfResource currentMaxResources = it->second;
            std::cout << "  VNF#" << vnfId << ":" << std::endl;
            printMaxVnfResources(&currentMaxResources);
        }*/

        if (vnfIndexExisted)
        {
            //std::cout << "VnfIndex#" << vnfIndex << " existed" << std::endl;
        }
        else
        {
            //std::cout << "VnfIndex#" << vnfIndex << " not existed" << std::endl;
        }

        // Save the new max config into the map

        vnfMaxResourceConfigsMap[vnfIndex] = maxResources;
        //std::cout << "Updated max resource config map for vnf#" << vnfIndex << std::endl;
        //printMaxVnfResources(maxResources);

        // Update gate select probabilities for all vnfGateIndexes of current slice.
        updateGateIndexSelectProb();
        //delete maxResources;
    /*}
    catch(const std::runtime_error& re)
    {
        // speciffic handling for runtime_error
        std::cerr << "Runtime error during executing SliceWeightedRoundRobinExecuter::updateMaxResourceConfigs: " << re.what() << std::endl;
    }
    catch(const std::exception& ex)
    {
        // speciffic handling for all exceptions extending std::exception, except
        // std::runtime_error which is handled explicitly
        std::cerr << "Error occurred during executing SliceWeightedRoundRobinExecuter::updateMaxResourceConfigs: " << ex.what() << std::endl;
    }
    catch(...)
    {
        // catch any other errors (that we have no information about)
        std::cerr << "Unknown failure occurred. Possible memory corruption during executing SliceWeightedRoundRobinExecuter::updateMaxResourceConfigs." << std::endl;
    }*/
}

void SliceWeightedRoundRobinExecuter::updateGateIndexSelectProb()
{
    try {
        int currentVnfIndex = 0;
        MaxVnfResource* maxResources = new MaxVnfResource;
        double totalCpu = 0;
        double totalRam = 0;
        double totalStorage = 0;
        double percentageCpu = 0;
        double percentageRam = 0;
        double percentageStorage = 0;
        double maxPercentage = 0;
        double totalPercentage = 0;

        //std::cout << "Updating VNF gates selection probabilities for load balance executer..." << std::endl;
        //std::cout << "    Current VNF gate index(es):" << std::endl;

        if (existingVNFIndex.size() == 1)
        {
            for (std::vector<int>::iterator it = existingVNFIndex.begin(); it != existingVNFIndex.end(); ++it){

                   currentVnfIndex = *it;
                   //std::cout << "        " << currentVnfIndex << std::endl;
            }
            vnfIndexSelectProbMap[currentVnfIndex] = 1;
            /*
            std::cout << "   Gate probability: " << vnfIndexSelectProbMap[currentVnfIndex]
                    << " (only 1 vnfGateIndex)" << std::endl;
                    */
        }
        else
        {
            // Get the sum of all CPU, RAM, STORAGE limits across all VNFs used by the slice
            for (std::vector<int>::iterator it = existingVNFIndex.begin(); it != existingVNFIndex.end(); ++it)
            {

                currentVnfIndex = *it;
                //std::cout << "        " << currentVnfIndex << std::endl;
                *maxResources = vnfMaxResourceConfigsMap[currentVnfIndex];
                //printMaxVnfResources(maxResources);
                totalCpu += maxResources->maxCpu;
                totalRam += maxResources->maxRam;
                totalStorage += maxResources->maxStorage;

            }

            /*
            std::cout << "Total CPU: " << totalCpu << " - Total RAM: " << totalRam
                    << " - Total Storage: " << totalStorage << std::endl;
                    */

            for (std::vector<int>::iterator it = existingVNFIndex.begin(); it != existingVNFIndex.end(); ++it)
            {
                currentVnfIndex = *it;

                //std::cout << "        VNF#" << currentVnfIndex << ": " << std::endl;
                *maxResources = vnfMaxResourceConfigsMap[currentVnfIndex];
                percentageCpu = maxResources->maxCpu / totalCpu;
                percentageRam = maxResources->maxRam / totalRam;
                percentageStorage = maxResources->maxStorage / totalStorage;
                /*
                std::cout << "            %CPU: " << percentageCpu << " - %RAM: " << percentageRam
                            << " - %Storage: " << percentageStorage << std::endl;
                            */
                maxPercentage = std::max({percentageCpu, percentageRam, percentageStorage});
                //std::cout << "            Max percentage: " << maxPercentage << std::endl;
                vnfIndexSelectProbMap[currentVnfIndex] = maxPercentage;
                totalPercentage += maxPercentage;
            }
            //std::cout << "Total%: " << totalPercentage << std::endl;


            for (std::vector<int>::iterator it = existingVNFIndex.begin(); it != existingVNFIndex.end(); ++it)
            {
                currentVnfIndex = *it;
                vnfIndexSelectProbMap[currentVnfIndex] = vnfIndexSelectProbMap[currentVnfIndex] / totalPercentage;

                //std::cout << "   VNF" << currentVnfIndex << ": " << std::endl;
                *maxResources = vnfMaxResourceConfigsMap[currentVnfIndex];
                //printMaxVnfResources(maxResources);
                //std::cout << "       Gate probability: " << vnfIndexSelectProbMap[currentVnfIndex] << std::endl;
            }
        }


        delete maxResources;
    }

    catch(const std::runtime_error& re)
    {
        // speciffic handling for runtime_error
        std::cerr << "Runtime error during executing SliceWeightedRoundRobinExecuter::updateGateIndexSelectProb: " << re.what() << std::endl;
    }
    catch(const std::exception& ex)
    {
        // speciffic handling for all exceptions extending std::exception, except
        // std::runtime_error which is handled explicitly
        std::cerr << "Error occurred during executing SliceWeightedRoundRobinExecuter::updateGateIndexSelectProb: " << ex.what() << std::endl;
    }
    catch(...)
    {
        // catch any other errors (that we have no information about)
        std::cerr << "Unknown failure occurred. Possible memory corruption during executing SliceWeightedRoundRobinExecuter::updateGateIndexSelectProb." << std::endl;
    }
}

void SliceWeightedRoundRobinExecuter::printMaxVnfResources(const MaxVnfResource* maxResource)
{
    std::cout << "        Max CPU: " << maxResource->maxCpu << std::endl;
    std::cout << "        Max RAM: " << maxResource->maxRam << std::endl;
    std::cout << "        Max CPU: " << maxResource->maxStorage << std::endl;
}
